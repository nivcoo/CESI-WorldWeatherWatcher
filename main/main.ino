#include "src/imported_libs/RTClib/RTClib.h"
#include "src/project_libs/Config/Config.h"
#include "src/project_libs/Led/Led.h"
#include "src/project_libs/SDCard/SDCard.h"
#include "src/imported_libs/BME280/src/BME280I2C.h"
#include "src/imported_libs/TinyGPS/TinyGPS.h"
#include <SoftwareSerial.h>

#define LIGHT_PIN 0
#define BUTTON_GREEN 2
#define BUTTON_RED 3
#define GPS_PIN_1 4
#define GPS_PIN_2 5

#define LED_PIN_1 8
#define LED_PIN_2 9

#define MODE_NORMAL 0
#define MODE_ECO 1
#define MODE_MAINTENANCE 2
#define MODE_CONFIG 3
#define MAX_VALUE 3
#define VERSION 1

#define SD_CHIP 4

String batchNumber = "C3W20_7038";

SoftwareSerial gps(GPS_PIN_1, GPS_PIN_2);
TinyGPS GPS;
BME280I2C bme;

RTC_DS1307 rtc;
Config config(VERSION, batchNumber);
Led leds(LED_PIN_1, LED_PIN_2, 1);
SDCard card(SD_CHIP, config.getValue("FILE_MAX_SIZE"));
byte previousMode = MODE_NORMAL;
byte mode = MODE_NORMAL;
unsigned long buttonPressedMs = millis();
bool buttonPressed = false;
bool checkStartPressedButton = true;
float gpsLon(0), gpsLat(0), gpsAlt(0);
bool SDWriteError = false;

struct Sensor {
  char id;
  String name;
  bool error;
  float average;
  float values[MAX_VALUE];
};
Sensor sensors[] {
  {'L', "Light (Lumen)" , 0, 0, {}},
  {'T', "Temperature (°C)", 0, 0, {}},
  {'H', "Hygrometry (%)", 0, 0, {}},
  {'P', "Pressure (HPa)", 0, 0, {}},
  {'W', "Water Temp (°C)", 1, 0, {}},
  {'C', "Water Current (Nœuds)", 1, 0, {}},
  {'F', "Wind Force (Nœuds)", 1, 0, {}},
  {'F', "Fine Particles (%)", 1, 0, {}}
};

void setup()
{
  Serial.begin(9600);
  rtc.begin();
  while (!card.begin()) {
    Serial.println(F("SD Card loading Failed"));
    delay(5000);
  }
  gps.begin(9600);
  config.showValues();
  pinMode(BUTTON_RED, INPUT_PULLUP);
  pinMode(BUTTON_GREEN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(BUTTON_RED), clickButtonRedEvent, FALLING);
  attachInterrupt(digitalPinToInterrupt(BUTTON_GREEN), clickButtonGreenEvent, FALLING);
  
}

void clickButtonGreenEvent() {
  buttonPressedMs = millis();
  buttonPressed = true;
}

void clickButtonRedEvent() {
  buttonPressedMs = millis();
  buttonPressed = true;
}

void pressedButtonGreen() {
  if (mode == MODE_NORMAL)
    changeMode(MODE_ECO);
  else if (mode == MODE_ECO)
    changeMode(MODE_NORMAL);
}

void pressedButtonRed() {
  if (mode == MODE_MAINTENANCE || mode == MODE_CONFIG) {
    changeMode(previousMode);
    while (!card.begin()) {
      Serial.println(F("SD Card loading Failed"));
      delay(5000);
    }
  }
  else {
    previousMode = mode;
    changeMode(MODE_MAINTENANCE);
    card.end();
  }
}

void changeMode(int _mode) {
  mode = _mode;

  String name = F("");
  switch (_mode) {
    case MODE_NORMAL:
      name = F("normal");
      break;
    case MODE_ECO:
      name = F("eco");
      break;
    case MODE_MAINTENANCE:
      name = F("maintenance");
      break;
    case MODE_CONFIG:
      name = F("configuration");
      break;
  }
  Serial.print(F("The new mode is "));
  Serial.println(name);
}

void checkPressedButton() {
  if (checkStartPressedButton && digitalRead(BUTTON_RED) == 0) {
    // go into config mode
    changeMode(MODE_CONFIG);
  }
  checkStartPressedButton = false;
  if ((millis() - buttonPressedMs) > (5 * 1000) && buttonPressed) {

    if (digitalRead(BUTTON_GREEN) == 0) {
      pressedButtonGreen();
    } else if (digitalRead(BUTTON_RED) == 0) {
      pressedButtonRed();
    }
    buttonPressed = false;
  }
}

void addSensorValue(float *values, float value) {
  for (int i = 0; i < MAX_VALUE - 1; i++) {
    values[i] = values[i + 1];
  }
  values[MAX_VALUE - 1] = value;
}

float getSensorAverage(float *values) {
  float avr = 0;
  for (int i = 0; i < MAX_VALUE; i++) {
    avr += values[i];
  }
  return (avr / MAX_VALUE);
}

unsigned long lastSuccess(0);

bool gpsEco = false;
bool gpsError = false;

byte getSensorValues() {
  int code = 0;
  float sensorTempValue(0), sensorHumValue(0), sensorPresValue(0);
  int sensorLightValue = analogRead(LIGHT_PIN);
  bme.read(sensorPresValue, sensorTempValue, sensorHumValue, BME280::TempUnit_Celsius, BME280::PresUnit_hPa);
  gpsEco = !gpsEco || mode != MODE_ECO;

  if (gpsEco) {
    bool updateGPS = false;
    for (unsigned long start = millis(); millis() - start < 1000;)
    {
      while (gps.available())
      {
        char c = gps.read();
        if (GPS.encode(c))
          updateGPS = true;
      }
    }
    if (updateGPS)
    {
      gpsError = false;
      unsigned long age;
      GPS.f_get_position(&gpsLat, &gpsLon, &age);
    } else
      gpsError = true;
  }

  bool sensorLightError = (sensorLightValue < config.getValue(F("LUMIN_LOW")) || sensorLightValue > config.getValue(F("LUMIN_HIGH"))) && config.getValue(F("LUMINO"));
  bool sensorTempError = (sensorTempValue < config.getValue(F("MIN_TEMP_AIR")) || sensorTempValue > config.getValue(F("MAX_TEMP_AIR"))) && config.getValue(F("TEMP_AIR"));
  bool sensorPresError = (sensorPresValue < config.getValue(F("PRESSURE_MIN")) || sensorPresValue > config.getValue(F("PRESSURE_MAX"))) && config.getValue(F("PRESSURE"));
  bool sensorHumError = (sensorTempValue < config.getValue(F("HYGR_MINT")) || sensorTempValue > config.getValue(F("HYGR_MAXT"))) && config.getValue(F("HYGR"));
  if (!rtc.begin()) {
    code = 1;
  }
  else if (sensorLightError || sensorTempError || sensorPresError || sensorHumError) {
    code = 2;
  }
  else if (!bme.begin())
  {
    code = 3;
  }
  else if (gpsError)
  {
    code = 4;
  }
  else if (SDWriteError) {
    code = 5;
  }
  /**else if (SDFull) {
    code = 6;
    }**/
  float value = 0;
  for (int i = 0; i < sizeof(sensors) / sizeof(Sensor); i++) {
    switch (sensors[i].id) {
      case 'L':
        if (sensorLightError)
          sensors[i].error = true;
        else {
          sensors[i].error = false;
          value = sensorLightValue;
          addSensorValue(sensors[i].values, value);
        }
        break;
      case 'T':
        if (sensorTempError || code == 3)
          sensors[i].error = true;
        else {
          sensors[i].error = false;
          value = sensorTempValue;
          addSensorValue(sensors[i].values, value);
        }
        break;
      case 'H':
        if (sensorHumError || code == 3)
          sensors[i].error = true;
        else {
          sensors[i].error = false;
          value = sensorHumValue;
          addSensorValue(sensors[i].values, value);
        }
        break;
      case 'P':
        if (sensorPresError || code == 3)
          sensors[i].error = true;
        else {
          sensors[i].error = false;
          value = sensorPresValue;
          addSensorValue(sensors[i].values, value);
        }
        break;
    }
    sensors[i].average = getSensorAverage(sensors[i].values);
  }
  return code;
}
unsigned long lastWrite(0);

void writeValues(bool sd) {
  if ((millis() - lastWrite) / 1000 > (60 * config.getValue(F("LOG_INTERVAL")) * ((mode == MODE_ECO) ? 2 : 1 )) ) {
    lastWrite = millis();
    if (rtc.begin()) {
      DateTime now = rtc.now();
      String text = F("[");
      text += now.day();
      text += F("/");
      text += now.month();
      text += F("/");
      text += now.year();
      text += F(" ");
      text += now.hour();
      text += F(":");
      text += now.minute();
      text += F(":");
      text += now.second();
      text += F("]  ");
      for (int i = 0; i < sizeof(sensors) / sizeof(Sensor); i++) {
        text += sensors[i].name + " : ";
        float averageValue = sensors[i].average;
        if (sensors[i].error || isnan((averageValue)))
          text += F("NA");
        else
          text += averageValue;
        text += F("  ");
      }
      text += F("|");
      text += F("  ");
      text += F("Latitude : ");
      if (gpsLat == TinyGPS::GPS_INVALID_F_ANGLE || gpsLat == 0)
        text += F("NA");
      else
        text += gpsLat;
      text += F("  ");
      text += F("Longitude : ");
      if (gpsLon == TinyGPS::GPS_INVALID_F_ANGLE || gpsLon == 0)
        text += F("NA");
      else
        text += gpsLon;
      text += F("  ");
      text += F("Altitude (m) : ");
      if (GPS.altitude() == TinyGPS::GPS_INVALID_ALTITUDE)
        text += F("NA");
      else {
        float altitude = GPS.altitude() / 100;
        text += altitude;
      }
      text += F("  ");
      text += F("Satelites : ");
      if (GPS.satellites() == TinyGPS::GPS_INVALID_SATELLITES)
        text += F("NA");
      else
        text += GPS.satellites();

      if (sd) {
        //write in SD card
        SDWriteError = card.writeTextInSD(text);

      }
      Serial.println(text);
    }
  }
}

unsigned long lastSensorCheck(0);
byte errorCode(0);
bool showError = false;
void loop()
{
  checkPressedButton();
  if (mode != MODE_CONFIG) {
    int timeCheck = (60 * config.getValue(F("LOG_INTERVAL")) / (MAX_VALUE + 2) * ((mode == MODE_ECO) ? 2 : 1 ));
    if ((millis() - lastSensorCheck) / 1000 > timeCheck ) {
      lastSensorCheck = millis();
      errorCode = getSensorValues();
      if (!errorCode) {
        lastSuccess == millis();
        showError = false;
      }
    }
    if ((millis() - lastSuccess) / 1000 > config.getValue(F("TIMEOUT")) + timeCheck) {
      showError = true;
      switch (errorCode) {
        case 1:
          //rtc error
          leds.color(F("RED"), 1, F("BLUE"), 1);
          break;
        case 2:
          //data error
          leds.color(F("RED"), 1, F("GREEN"), 2);
          break;
        case 3:
          //sensor error
          leds.color(F("RED"), 1, F("GREEN"), 1);
          break;
        case 4:
          //gps error
          leds.color(F("RED"), 1, F("YELLOW"), 1);
          break;
        case 5:
          //SD card write error
          leds.color(F("RED"), 1, F("WHITE"), 2);
          break;
        case 6:
          //SD card full
          leds.color(F("RED"), 1, F("WHITE"), 1);
          break;
      }
    }
    if (mode == MODE_NORMAL) {
      if (!showError)
        leds.color(F("GREEN"));
      //true = write in SD card
      writeValues(true);
    }
    else if (mode == MODE_ECO) {
      if (!showError)
        leds.color(F("BLUE"));
      //true = write in SD card
      writeValues(true);
    }
    else if (mode == MODE_MAINTENANCE) {
      if (!showError)
        leds.color(F("ORANGE"));
      //true = write in SD card
      writeValues(false);
    }
  }
  else {
    leds.color(F("YELLOW"));
    unsigned long lastActivity = config.getLastActivity();
    //go to normal if inactivity > 30m
    if ((millis() - lastActivity) / 1000 > (30 * 60)) {
      changeMode(MODE_NORMAL);
    }
    config.waitValues();
  }
}

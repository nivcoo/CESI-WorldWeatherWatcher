#include "src/project_libs/Config/Config.h"
#include "src/project_libs/Led/Led.h"
#include "src/imported_libs/DS1307RTC/DS1307RTC.h"
#include "src/imported_libs/BME280/src/BME280I2C.h"
#include "src/imported_libs/TinyGPS/TinyGPS.h"
#include <SoftwareSerial.h>
//#include <SD.h>


#define LIGHT_PIN 0
#define BUTTON_GREEN 2
#define BUTTON_RED 3
#define GPS_PIN_1 4
#define GPS_PIN_2 2

#define MODE_NORMAL 0
#define MODE_ECO 1
#define MODE_MAINTENANCE 2
#define MODE_CONFIG 3
#define MAX_VALUE 3

SoftwareSerial gps(GPS_PIN_1, GPS_PIN_2);
TinyGPS GPS;
BME280I2C bme;
tmElements_t tm;
DS1307RTC clock;
Config config(1, "09A");
Led leds(8, 9, 1);
//0 : Normal, 1 : Eco, 2 : Maintenance, 3 : Config
byte previousMode = 0;
byte mode = 0;
unsigned long buttonPressedMs = millis();
bool buttonPressed = false;
bool checkStartPressedButton = true;
float gpsLon(0), gpsLat(0), gpsAlt(0);

typedef struct {
  char name;
  float avr;
  float values[MAX_VALUE];
} Sensor;
Sensor sensors[] {
  {'L', 0, {}},
  {'T', 0, {}},
  {'H', 0, {}},
  {'P', 0, {}}
};

void setup()
{
  Serial.begin(9600);
  gps.begin(9600);
  config.showValues();
  pinMode(BUTTON_RED, INPUT_PULLUP);
  pinMode(BUTTON_GREEN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(BUTTON_RED), clickButtonRedEvent, RISING);
  attachInterrupt(digitalPinToInterrupt(BUTTON_GREEN), clickButtonGreenEvent, RISING);
  //setDate("05 10 2020 12:28:35");
  //showDate();
}

bool setDate(const char *str)
{
  int Day, Month, Year, Hour, Min, Sec;

  if (sscanf(str, "%d %d %d %d:%d:%d", &Day, &Month, &Year, &Hour, &Min, &Sec) != 6) return false;
  tm.Day = Day;
  tm.Month = Month;
  tm.Year = CalendarYrToTm(Year);
  tm.Hour = Hour;
  tm.Minute = Min;
  tm.Second = Sec;
  RTC.write(tm);
}

void showDate()
{
  if (RTC.read(tm)) {
    Serial.print(tm.Day, DEC);
    Serial.print(F("/"));
    Serial.print(tm.Month, DEC);
    Serial.print(F("/"));
    Serial.print(tmYearToCalendar(tm.Year), DEC);
    Serial.print(F(" "));
    Serial.print(tm.Hour, DEC);
    Serial.print(F(":"));
    Serial.print(tm.Minute, DEC);
    Serial.print(F(":"));
    Serial.print(tm.Second, DEC);
  } else {
    if (RTC.chipPresent()) {
      Serial.println(F("The RTC is stopped. Please run the setDate"));
    } else {
      Serial.println(F("RTC read error! Please check."));
    }
  }
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
  if (mode == 0)
    changeMode(MODE_ECO);
  else if (mode == 1)
    changeMode(MODE_NORMAL);
}

void pressedButtonRed() {
  if (mode == 2 || mode == 3)
    changeMode(previousMode);
  else {
    previousMode = mode;
    changeMode(MODE_MAINTENANCE);
  }
}

void changeMode(int _mode) {
  mode = _mode;

  String name = F("");
  //0 : Normal, 1 : Eco, 2 : Maintenance, 3 : Config
  switch (_mode) {
    case 0:
      name = F("normal");
      break;
    case 1:
      name = F("eco");
      break;
    case 2:
      name = F("maintenance");
      break;
    case 3:
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

void addValue(float *values, float value) {
  for (int i = 0; i < MAX_VALUE - 1; i++) {
    values[i] = values[i + 1];
  }
  values[MAX_VALUE - 1] = value;
}

float getAvr(float *values) {
  float avr = 0;

  for (int i = 0; i < MAX_VALUE; i++) {
    avr += values[i];
  }
  return (avr / MAX_VALUE);
}

unsigned long lastSuccess(0);

BME280::TempUnit sensorTempUnit(BME280::TempUnit_Celsius);
BME280::PresUnit sensorPresUnit(BME280::PresUnit_hPa);


bool getSensorValues() {
  bool error = false;
  int code = 0;
  float sensorTempValue(0), sensorHumValue(0), sensorPresValue(0);
  int sensorLightValue = analogRead(LIGHT_PIN);
  bme.read(sensorPresValue, sensorTempValue, sensorHumValue, sensorTempUnit, sensorPresUnit);
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
    unsigned long age;
    GPS.f_get_position(&gpsLat, &gpsLon, &age);
  }


  bool sensorLightError = (sensorLightValue < config.getValue(F("LUMIN_LOW")) || sensorLightValue > config.getValue(F("LUMIN_HIGH"))) && config.getValue(F("LUMINO"));
  bool sensorTempError = (sensorTempValue < config.getValue(F("MIN_TEMP_AIR")) || sensorTempValue > config.getValue(F("MAX_TEMP_AIR"))) && config.getValue(F("TEMP_AIR"));
  bool sensorPresError = (sensorPresValue < config.getValue(F("PRESSURE_MIN")) || sensorPresValue > config.getValue(F("PRESSURE_MAX"))) && config.getValue(F("PRESSURE"));
  bool sensorHumError = (sensorTempValue < config.getValue(F("HYGR_MINT")) || sensorTempValue > config.getValue(F("HYGR_MAXT"))) && config.getValue(F("HYGR"));
  if (!RTC.read(tm)) {
    code = 1;
  }
  else if (sensorLightError || sensorTempError || sensorPresError || sensorHumError) {
    code = 2;
  }
  else if (!bme.begin())
  {
    code = 3;
  }
  else if (!updateGPS)
  {
    code = 4;
  } else {
    lastSuccess = millis();
  }

  float value = 0;

  for (int i = 0; i < sizeof(sensors) / sizeof(Sensor); i++) {

    switch (sensors[i].name) {
      case 'L':
        value = sensorLightValue;
        addValue(sensors[i].values, value);
        break;
      case 'T':
        value = sensorTempValue;
        addValue(sensors[i].values, value);
        break;
      case 'H':
        value = sensorHumValue;
        addValue(sensors[i].values, value);
        break;
      case 'P':
        value = sensorPresValue;
        addValue(sensors[i].values, value);
        break;
    }
    sensors[i].avr = getAvr(sensors[i].values);
  }

  if ((millis() - lastSuccess) / 1000 > config.getValue(F("TIMEOUT"))) {
    error = true;
    switch (code) {
      case 1:
        //rtc error
        leds.color(F("RED"), 1, F("BLUE"), 1);
        break;
      case 2:
        //data error
        leds.color(F("RED"), 1, F("GREEN"), 3);
        break;
      case 3:
        //sensor error
        leds.color(F("RED"), 1, F("GREEN"), 1);
        break;
      case 4:
        //gps error
        leds.color("RED", 1, "YELLOW", 1);
        break;
    }
  }
  return error;
}
unsigned long lastWrite(0);
void writeValues(bool sd) {
  if ((millis() - lastWrite) / 1000 > (60 * config.getValue(F("LOG_INTERVAL")))) {
    lastWrite = millis();
    if (sd) {
      //write in SD card
    } else {
      Serial.print(F("["));
      showDate();
      Serial.print(F("]  "));
      for (int i = 0; i < sizeof(sensors) / sizeof(Sensor); i++) {
        switch (sensors[i].name) {
          case 'L':
            //rtc error
            Serial.print(F("Light : "));
            break;
          case 'T':
            //data error
            Serial.print(F("Temperature (°C) : "));
            break;
          case 'H':
            //sensor error
            Serial.print(F("Hygrometry (%) : "));
            break;
          case 'P':
            //gps error
            Serial.print(F("Pressure (HPa) : "));
            break;
        }
        Serial.print(sensors[i].avr);
        Serial.print(F("   "));
      }
      Serial.print(F("|"));
      Serial.print(F("   "));
      Serial.print(F("Latitude : "));
      Serial.print(gpsLat == TinyGPS::GPS_INVALID_F_ANGLE ? 0.0 : gpsLat, 6);
      Serial.print(F("   "));
      Serial.print(F("Longitude : "));
      Serial.print(gpsLon == TinyGPS::GPS_INVALID_F_ANGLE ? 0.0 : gpsLon, 6);
      Serial.print(F("   "));
      Serial.print(F("Altitude (m) : "));
      Serial.print(GPS.altitude() == TinyGPS::GPS_INVALID_ALTITUDE ? 0 : GPS.altitude() / 100);
      Serial.print(F("   "));
      Serial.print(F("Satelites : "));
      Serial.println(GPS.satellites() == TinyGPS::GPS_INVALID_SATELLITES ? 0 : GPS.satellites());
     
      
    }
  }
}

void loop()
{
  checkPressedButton();
  if (mode != 3) {
    if (!getSensorValues()) {
      if (mode == 0) {
        leds.color(F("GREEN"));
        //true = write in SD card so if SD CARD works put true
        writeValues(false);
      }
      else if (mode == 1) {
        leds.color(F("BLUE"));
        //true = write in SD card so if SD CARD works put true
        writeValues(false);
      }
      else if (mode == 2) {
        leds.color(F("ORANGE"));
        //true = write in SD card
        writeValues(false);
      }
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

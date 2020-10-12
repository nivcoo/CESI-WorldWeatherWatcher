#include "src/imported_libs/RTClib/RTClib.h"
#include "src/project_libs/Config/Config.h"
#include "src/project_libs/Led/Led.h"
#include "src/imported_libs/BME280/src/BME280I2C.h"
#include "src/imported_libs/TinyGPS/TinyGPS.h"
#include <SoftwareSerial.h>
//#define USE_SD //remove comment to use SD card
#ifdef USE_SD

#include <SD.h>
#define CHIP 4

#endif

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

SoftwareSerial gps(GPS_PIN_1, GPS_PIN_2);
TinyGPS GPS;
BME280I2C bme;



RTC_DS1307 rtc;
Config config(1, "09A");
Led leds(LED_PIN_1, LED_PIN_2, 1);
byte previousMode = MODE_NORMAL;
byte mode = MODE_NORMAL;
unsigned long buttonPressedMs = millis();
bool buttonPressed = false;
bool checkStartPressedButton = true;
float gpsLon(0), gpsLat(0), gpsAlt(0);
#ifdef USE_SD
bool SDWriteError = false;
#endif


typedef struct {
  char name;
  bool error;
  float avr;
  float values[MAX_VALUE];
} Sensor;
Sensor sensors[] {
  {'L', 0, 0, {}},
  {'T', 0, 0, {}},
  {'H', 0, 0, {}},
  {'P', 0, 0, {}}
};

void setup()
{
  Serial.begin(9600);
  rtc.begin();
#ifdef USE_SD
  if (!SD.begin(CHIP)) {
    Serial.println(F("SD Card loading Failed"));
    while (true);
  }
#endif
  gps.begin(9600);
  config.showValues();
  pinMode(BUTTON_RED, INPUT_PULLUP);
  pinMode(BUTTON_GREEN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(BUTTON_RED), clickButtonRedEvent, RISING);
  attachInterrupt(digitalPinToInterrupt(BUTTON_GREEN), clickButtonGreenEvent, RISING);
  //showDate();
}


void showDate()
{
  if (rtc.begin()) {
    DateTime now = rtc.now();
    Serial.print(now.day(), DEC);
    Serial.print(F("/"));
    Serial.print(now.month(), DEC);
    Serial.print(F("/"));
    Serial.print(now.year(), DEC);
    Serial.print(F(" "));
    Serial.print(now.hour(), DEC);
    Serial.print(F(":"));
    Serial.print(now.minute(), DEC);
    Serial.print(F(":"));
    Serial.print(now.second(), DEC);
  } else {
    if (rtc.isrunning()) {
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
  if (mode == MODE_NORMAL)
    changeMode(MODE_ECO);
  else if (mode == MODE_ECO)
    changeMode(MODE_NORMAL);
}

void pressedButtonRed() {
  if (mode == MODE_MAINTENANCE || mode == MODE_CONFIG) {
    changeMode(previousMode);
#ifdef USE_SD
    while (!SD.begin(CHIP)) {
      Serial.println(F("SD Card loading Failed"));
    }
#endif
  }
  else {
    previousMode = mode;
    changeMode(MODE_MAINTENANCE);
#ifdef USE_SD
    SD.end();
#endif
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

bool gpsEco = false;
bool errorGPS = false;
byte getSensorValues() {
  int code = 0;
  float sensorTempValue(0), sensorHumValue(0), sensorPresValue(0);
  int sensorLightValue = analogRead(LIGHT_PIN);
  bme.read(sensorPresValue, sensorTempValue, sensorHumValue, sensorTempUnit, sensorPresUnit);
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
      errorGPS = false;
      unsigned long age;
      GPS.f_get_position(&gpsLat, &gpsLon, &age);
    } else {
      errorGPS = true;
    }
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
  else if (errorGPS)
  {
    code = 4;

  }
#ifdef USE_SD
  else if (SDWriteError) {
    code = 5;
  }
  else if (SDWriteError) {
    code = 5;
  }
#endif

  float value = 0;

  for (int i = 0; i < sizeof(sensors) / sizeof(Sensor); i++) {

    switch (sensors[i].name) {
      case 'L':
        if (sensorLightError)
          sensors[i].error = true;
        else {
          sensors[i].error = false;
          value = sensorLightValue;
          addValue(sensors[i].values, value);
        }
        break;
      case 'T':
        if (sensorTempError || code == 3)
          sensors[i].error = true;
        else {
          sensors[i].error = false;
          value = sensorTempValue;
          addValue(sensors[i].values, value);
        }

        break;
      case 'H':
        if (sensorHumError || code == 3)
          sensors[i].error = true;
        else {
          sensors[i].error = false;
          value = sensorHumValue;
          addValue(sensors[i].values, value);
        }
        break;
      case 'P':
        if (sensorPresError || code == 3)
          sensors[i].error = true;
        else {
          sensors[i].error = false;
          value = sensorPresValue;
          addValue(sensors[i].values, value);
        }
        break;
    }
    sensors[i].avr = getAvr(sensors[i].values);
  }
  return code;
}
#ifdef USE_SD
void dateTime(uint16_t* date, uint16_t* time) {
  DateTime now = rtc.now();

  // return date using FAT_DATE macro to format fields
  date = FAT_DATE(now.year(), now.month(), now.day());

  // return time using FAT_TIME macro to format fields
  time = FAT_TIME(now.hour(), now.minute(), now.second());
}

void checkSizeFiles(String startFile, int startNumber) {

  String extension = ".log";
  String fileName = startFile + startNumber + extension;
  File file = SD.open(fileName);
  int fileSize = file.size();
  if (fileSize > config.getValue(F("FILE_MAX_SIZE"))) {
    String newFileName = getLogFileName(startFile, 1);
    File newFile = SD.open(newFileName, FILE_WRITE);
    size_t n;
    uint8_t buf[64];
    while ((n = file.read(buf, sizeof(buf))) > 0) {
      newFile.write(buf, n);
    }
    newFile.close();
    file.close();
    SD.remove(fileName);
  } else
    file.close();

}


String getLogFileName(String startFile, int startNumber) {
  String extension = ".log";
  String fileName = startFile + startNumber + extension;
  File file = SD.open(fileName, FILE_WRITE);
  int fileSize = file.size();
  file.close();
  int i = 0;
  while (fileSize > config.getValue(F("FILE_MAX_SIZE"))) {
    fileName = startFile + (startNumber + i) + extension;
    file = SD.open(fileName, FILE_WRITE);
    fileSize = file.size();
    file.close();
    i++;
  }
  return fileName;
}

#endif
unsigned long lastWrite(0);

void writeValues(bool sd) {
  if ((millis() - lastWrite) / 1000 > (60 * config.getValue(F("LOG_INTERVAL")) * ((mode == MODE_ECO) ? 2 : 1 )) ) {
    lastWrite = millis();
    if (rtc.begin()) {
      if (sd) {
        //write in SD card
#ifdef USE_SD
        DateTime now = rtc.now();
        SdFile::dateTimeCallback(dateTime);
        String year = String(now.year() - 2000);
        String month = String(now.month());
        String day = String(now.day());
        String startFiles = year + month + day + "_";
        checkSizeFiles(startFiles, 0);
        String fileName = startFiles + 0 + ".log";
        File logFile = SD.open(fileName, FILE_WRITE);
        if (logFile) {
          SDWriteError = false;
          logFile.print(F("["));
          logFile.print(now.day(), DEC);
          logFile.print(F("/"));
          logFile.print(now.month(), DEC);
          logFile.print(F("/"));
          logFile.print(now.year());
          logFile.print(F(" "));
          logFile.print(now.hour(), DEC);
          logFile.print(F(":"));
          logFile.print(now.minute(), DEC);
          logFile.print(F(":"));
          logFile.print(now.second(), DEC);
          logFile.print(F("]  "));
          for (int i = 0; i < sizeof(sensors) / sizeof(Sensor); i++) {
            switch (sensors[i].name) {
              case 'L':
                //rtc error
                logFile.print(F("Light : "));
                break;
              case 'T':
                //data error
                logFile.print(F("Temperature (°C) : "));
                break;
              case 'H':
                //sensor error
                logFile.print(F("Hygrometry (%) : "));
                break;
              case 'P':
                //gps error
                logFile.print(F("Pressure (HPa) : "));
                break;
            }
            if (sensors[i].error || isnan((sensors[i].avr)))
              logFile.print(F("NA"));
            else
              logFile.print(sensors[i].avr);
            logFile.print(F("   "));
          }
          logFile.print(F("|"));
          logFile.print(F("   "));
          logFile.print(F("Latitude : "));
          if (gpsLat == TinyGPS::GPS_INVALID_F_ANGLE || gpsLat == 0)
            logFile.print(F("NA"));
          else
            logFile.print(gpsLat, 6);
          logFile.print(F("   "));
          logFile.print(F("Longitude : "));
          if (gpsLon == TinyGPS::GPS_INVALID_F_ANGLE || gpsLon == 0)
            logFile.print(F("NA"));
          else
            logFile.print(gpsLon, 6);
          logFile.print(F("   "));
          logFile.print(F("Altitude (m) : "));
          if (GPS.altitude() == TinyGPS::GPS_INVALID_ALTITUDE)
            logFile.print("NA");
          else
            logFile.print(GPS.altitude() / 100, 3);
          logFile.print(F("   "));
          logFile.print(F("Satelites : "));
          if (GPS.satellites() == TinyGPS::GPS_INVALID_SATELLITES)
            logFile.println(F("NA"));
          else
            logFile.println(GPS.satellites());
          logFile.close();
        } else {
          SDWriteError = true;
        }
#endif



      }
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
        if (sensors[i].error || isnan((sensors[i].avr)))
          Serial.print(F("NA"));
        else
          Serial.print(sensors[i].avr);

        Serial.print(F("   "));
      }
      Serial.print(F("|"));
      Serial.print(F("   "));
      Serial.print(F("Latitude : "));
      if (gpsLat == TinyGPS::GPS_INVALID_F_ANGLE || gpsLat == 0)
        Serial.print(F("NA"));
      else
        Serial.print(gpsLat, 6);
      Serial.print(F("   "));
      Serial.print(F("Longitude : "));
      if (gpsLon == TinyGPS::GPS_INVALID_F_ANGLE || gpsLon == 0)
        Serial.print(F("NA"));
      else
        Serial.print(gpsLon, 6);
      Serial.print(F("   "));
      Serial.print(F("Altitude (m) : "));
      if (GPS.altitude() == TinyGPS::GPS_INVALID_ALTITUDE)
        Serial.print(F("NA"));
      else
        Serial.print(GPS.altitude() / 100, 3);
      Serial.print(F("   "));
      Serial.print(F("Satelites : "));
      if (GPS.satellites() == TinyGPS::GPS_INVALID_SATELLITES)
        Serial.println(F("NA"));
      else
        Serial.println(GPS.satellites());
    }



  }
}

unsigned long lastSensorCheck(0);
byte errorCode(0);
void loop()
{
  checkPressedButton();
  if (mode != MODE_CONFIG) {

    int timeCheck = (60 * config.getValue(F("LOG_INTERVAL")) / (MAX_VALUE + 2) * ((mode == MODE_ECO) ? 2 : 1 ));
    
    if ((millis() - lastSensorCheck) / 1000 > timeCheck ) {
      lastSensorCheck = millis();
      errorCode = getSensorValues();
    }
    if (!errorCode) {
      lastSuccess == millis();
    }
    if ((millis() - lastSuccess) / 1000 > config.getValue(F("TIMEOUT")) + timeCheck) {
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
      if (!errorCode)
        leds.color(F("GREEN"));
      //true = write in SD card
#ifdef USE_SD
      writeValues(true);
#else
      writeValues(false);
#endif
    }
    else if (mode == MODE_ECO) {
      if (!errorCode)
        leds.color(F("BLUE"));
      //true = write in SD card
#ifdef USE_SD
      writeValues(true);
#else
      writeValues(false);
#endif
    }
    else if (mode == MODE_MAINTENANCE) {
      if (!errorCode)
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

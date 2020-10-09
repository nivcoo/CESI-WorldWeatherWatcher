#include "src/project_libs/Config/Config.h"
#include "src/project_libs/Led/Led.h"
#include "src/imported_libs/RTClib/RTClib.h"
#include "src/imported_libs/BME280/src/BME280I2C.h"
#include "src/imported_libs/TinyGPS/TinyGPS.h"
#include <SoftwareSerial.h>

/**
  #include <SD.h> //remove comment to use SD card
  #define CHIP 4  //remove comment to use SD card**/
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
//0 : Normal, 1 : Eco, 2 : Maintenance, 3 : Config
byte previousMode = 0;
byte mode = 0;
unsigned long buttonPressedMs = millis();
bool buttonPressed = false;
bool checkStartPressedButton = true;
float gpsLon(0), gpsLat(0), gpsAlt(0);
//bool SDWriteError = false; //remove comment to use SD card


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
  /**if (!SD.begin(CHIP)) { //remove comment to use SD card
    Serial.println(F("SD Card loading Failed")); //remove comment to use SD card
    while (true); //remove comment to use SD card
    } //remove comment to use SD card**/
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
  if (mode == 0)
    changeMode(MODE_ECO);
  else if (mode == 1)
    changeMode(MODE_NORMAL);
}

void pressedButtonRed() {
  if (mode == 2 || mode == 3) {
    changeMode(previousMode);
    /**while (!SD.begin(CHIP)) { //remove comment to use SD card
      Serial.println(F("SD Card loading Failed")); //remove comment to use SD card
      } //remove comment to use SD card**/
  }
  else {
    previousMode = mode;
    changeMode(MODE_MAINTENANCE);
    //SD.end(); //remove comment to use SD card
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

bool gpsEco = false;
byte getSensorValues() {
  int code = 0;
  float sensorTempValue(0), sensorHumValue(0), sensorPresValue(0);
  int sensorLightValue = analogRead(LIGHT_PIN);
  bme.read(sensorPresValue, sensorTempValue, sensorHumValue, sensorTempUnit, sensorPresUnit);
  bool updateGPS = false;
  gpsEco = !gpsEco || mode != MODE_ECO;
  if (gpsEco) {
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
  else if (!updateGPS)
  {
    code = 4;
  } /**else if (SDWriteError) { //remove comment to use SD card
    code = 5; //remove comment to use SD card
  } //remove comment to use SD card**/
  /**else if (SDWriteError) { //remove comment to use SD card
      code = 5; //remove comment to use SD card
    } //remove comment to use SD card**/

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

/**void dateTime(uint16_t* date, uint16_t* time) { //remove comment to use SD card
  DateTime now = rtc.now();

  // return date using FAT_DATE macro to format fields
   date = FAT_DATE(now.year(), now.month(), now.day());

  // return time using FAT_TIME macro to format fields
   time = FAT_TIME(now.hour(), now.minute(), now.second());
  }//remove comment to use SD card **/

/**void checkSizeFiles(String startFile, int startNumber) {

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

  } //remove comment to use SD card**/

/**
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
  } //remove comment to use SD card **/
unsigned long lastWrite(0);

void writeValues(bool sd) {
  if ((millis() - lastWrite) / 1000 > (60 * config.getValue(F("LOG_INTERVAL")) * ((mode == MODE_ECO) ? 2 : 1 )) ) {
    lastWrite = millis();
    if (rtc.begin()) {
      if (sd) {
        //write in SD card

        /** //remove comment to use SD card
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
                logFile.print("NA");
              else
                logFile.print(sensors[i].avr);
              logFile.print(F("   "));
            }
            logFile.print(F("|"));
            logFile.print(F("   "));
            logFile.print(F("Latitude : "));
            logFile.print(gpsLat == TinyGPS::GPS_INVALID_F_ANGLE ? 0.0 : gpsLat, 6);
            logFile.print(F("   "));
            logFile.print(F("Longitude : "));
            logFile.print(gpsLon == TinyGPS::GPS_INVALID_F_ANGLE ? 0.0 : gpsLon, 6);
            logFile.print(F("   "));
            logFile.print(F("Altitude (m) : "));
            logFile.print(GPS.altitude() == TinyGPS::GPS_INVALID_ALTITUDE ? 0 : GPS.altitude() / 100);
            logFile.print(F("   "));
            logFile.print(F("Satelites : "));
            logFile.println(GPS.satellites() == TinyGPS::GPS_INVALID_SATELLITES ? 0 : GPS.satellites());
            logFile.close();
          } else {
            SDWriteError = true;
          }//remove comment to use SD card **/





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
          Serial.print("NA");
        else
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

unsigned long lastSensorCheck(0);
byte errorCode(0);
void loop()
{
  checkPressedButton();
  if (mode != 3) {
    if ((millis() - lastSensorCheck) / 1000 > (60 * config.getValue(F("LOG_INTERVAL")) / (MAX_VALUE + 2) * ((mode == MODE_ECO) ? 2 : 1 )) ) {
      lastSensorCheck = millis();
      errorCode = getSensorValues();
    }
    if (!errorCode == 0) {
      lastSuccess == millis();
    }
    if ((millis() - lastSuccess) / 1000 > config.getValue(F("TIMEOUT")) / 30) {
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
          leds.color("RED", 1, "YELLOW", 1);
          break;
        case 5:
          //SD card write error
          leds.color("RED", 1, "WHITE", 2);
          break;
        case 6:
          //SD card full
          leds.color("RED", 1, "WHITE", 1);
          break;
      }
    }
    if (mode == 0) {
      if (!errorCode)
        leds.color(F("GREEN"));
      //true = write in SD card so if SD CARD works put true
      writeValues(false);
    }
    else if (mode == 1) {
      if (!errorCode)
        leds.color(F("BLUE"));
      //true = write in SD card so if SD CARD works put true
      writeValues(false);
    }
    else if (mode == 2) {
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

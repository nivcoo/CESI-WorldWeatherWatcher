#include "src/project_libs/Config/Config.h"
#include "src/project_libs/Led/Led.h"
#include "src/imported_libs/DS1307RTC/DS1307RTC.h"
#include "src/imported_libs/BME280/src/BME280I2C.h"
#include <SoftwareSerial.h>
//#include <SD.h>

/**
BME280I2C bme;
SoftwareSerial SoftSerial(4, 5);
**/
SoftwareSerial SoftSerial(4, 5);
BME280I2C bme;
/**Sd2Card card;
SdVolume volume;
SdFile root;**/

tmElements_t tm;
DS1307RTC clock;
Config config(2, "0930A");
Led leds(8, 9, 1);


//0 : Normal, 1 : Eco, 2 : Maintenance, 3 : Config
byte previousMode = 0;
byte mode = 0;
long buttonPressedMs = millis();
bool buttonPressed = false;
bool checkStartPressedButton = true;


const byte buttonPinGreen PROGMEM = 2;
const byte buttonPinRed PROGMEM = 3;

int sensorLightValue(0);

float sensorTempValue(0), sensorHumValue(0), sensorPresValue(0);

void setup()
{
  Serial.begin(9600);
  config.showValues();
  pinMode(buttonPinRed, INPUT_PULLUP);
  pinMode(buttonPinGreen, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(buttonPinRed), clickButtonRedEvent, CHANGE);
  attachInterrupt(digitalPinToInterrupt(buttonPinGreen), clickButtonGreenEvent, CHANGE);
  showDate();


  //setDate("02 10 2020 10:25:20");
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
    Serial.print(tm.Hour, DEC);
    Serial.print(F(":"));
    Serial.print(tm.Minute, DEC);
    Serial.print(F(":"));
    Serial.print(tm.Second, DEC);
    Serial.print(F("  "));
    Serial.print(tm.Day, DEC);
    Serial.print(F("/"));
    Serial.print(tm.Month, DEC);
    Serial.print(F("/"));
    Serial.print(tmYearToCalendar(tm.Year), DEC);
    Serial.print(F(" "));
    Serial.println(F(" "));
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
    changeMode(1);
  else if (mode == 1)
    changeMode(0);
}

void pressedButtonRed() {
  if (mode == 2 || mode == 3)
    changeMode(previousMode);
  else {
    previousMode = mode;
    changeMode(2);
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
  if (checkStartPressedButton && digitalRead(buttonPinRed) == 0) {
    // go into config mode
    changeMode(3);
  }
  checkStartPressedButton = false;
  if ((millis() - buttonPressedMs) > (5 * 1000) && buttonPressed) {

    if (digitalRead(buttonPinGreen) == 0) {
      pressedButtonGreen();
    } else if (digitalRead(buttonPinRed) == 0) {
      pressedButtonRed();
    }
    buttonPressed = false;
  }
}

long lastError(0);

bool checkError() {
  bool error = false;
  int code = 0;
  if (!RTC.read(tm)) {
    code = 1;
  }


  bool sensorLightError = (sensorLightValue < config.getValue(F("LUMIN_LOW")) || sensorLightValue > config.getValue(F("LUMIN_HIGH"))) && config.getValue(F("LUMINO"));
  bool sensorTempError = (sensorTempValue < config.getValue(F("MIN_TEMP_AIR")) || sensorTempValue > config.getValue(F("MAX_TEMP_AIR"))) && config.getValue(F("TEMP_AIR"));
  bool sensorPresError = (sensorPresValue < config.getValue(F("PRESSURE_MIN")) || sensorPresValue > config.getValue(F("PRESSURE_MAX"))) && config.getValue(F("PRESSURE"));
  bool sensorHumError = (sensorTempValue < config.getValue(F("HYGR_MINT")) || sensorTempValue > config.getValue(F("HYGR_MAXT"))) && config.getValue(F("HYGR"));


  if (sensorLightError || sensorTempError || sensorPresError || sensorHumError) {
    code = 2;
  }

  if (!bme.begin())
  {
    code = 3;
  }

  if (!code)
    lastError = millis();

  if (millis() - lastError > config.getValue(F("TIMEOUT")) * 1000) {

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


    }
  }





  //gps error
  //leds.color("RED", 1, "YELLOW", 1);
  //sensor error
  //leds.color("RED", 1, "GREEN", 1);
  //wrong data
  //leds.color("RED", 1, "GREEN", 3);
  //sd full
  //leds.color("RED", 1, "WHITE", 1);
  //write error in sd
  //leds.color("RED", 1, "WHITE", 3);
  return error;
}


void getSensorValues() {
  sensorLightValue = analogRead(0);
  BME280::TempUnit sensorTempUnit(BME280::TempUnit_Celsius);
  BME280::PresUnit sensorPresUnit(BME280::PresUnit_hPa);
  bme.read(sensorPresValue, sensorTempValue, sensorHumValue, sensorTempUnit, sensorPresUnit);
}

void loop()
{
  checkPressedButton();
  if (mode != 3) {
    getSensorValues();
    if (!checkError()) {
      if (mode == 0) {
        leds.color(F("GREEN"));
      }
      else if (mode == 1) {
        leds.color(F("BLUE"));
      }
      else if (mode == 2) {
        leds.color(F("ORANGE"));
      }
    }
  }
  else {
    leds.color(F("YELLOW"));
    long lastActivity = config.getLastActivity();
    //go to normal if inactivity > 30m
    if ((millis() - lastActivity) / 100 > (30 * 60 * 10)) {
      changeMode(0);
    }
    config.waitValues();
  }



  //leds.color("BLUE", 2);
  //leds.color("RED");

  //leds.color("RED", 10, "YELLOW", 3);
  //delay(300);
}

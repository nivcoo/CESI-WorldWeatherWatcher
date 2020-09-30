#include "src/project_libs/Config/Config.h"
#include "src/project_libs/Led/Led.h"
#include "src/imported_libs/DS1307RTC/DS1307RTC.h"
#include "src/imported_libs/BME280/src/BME280I2C.h"

tmElements_t tm;
BME280I2C bme;

byte version = 2;
String batchNumber = "20200930A";

//0 : Normal, 1 : Eco, 2 : Maintenance, 3 : Config
int previousMode = 0;
int mode = 0;
Config config(version, batchNumber);
Led leds(8, 9, 1);
DS1307RTC clock;
BME280::TempUnit sensorTempUnit(BME280::TempUnit_Celsius);
BME280::PresUnit sensorPresUnit(BME280::PresUnit_hPa);
long buttonPressedMs = millis();
bool buttonPressed = false;
bool checkStartPressedButton = true;

const byte buttonPinGreen = 2;
const byte buttonPinRed = 3;

float sensorTempValue(0), sensorHumValue(0), sensorPresValue(0), sensorLightValue(0);

void setup()
{

  Serial.begin(9600);
  config.showValues();
  pinMode(buttonPinRed, INPUT_PULLUP);
  pinMode(buttonPinGreen, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(buttonPinRed), clickButtonRedEvent, CHANGE);
  attachInterrupt(digitalPinToInterrupt(buttonPinGreen), clickButtonGreenEvent, CHANGE);
  //setDate("30 09 2020 13:27:40");
  showDate();
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
    Serial.print(":");
    Serial.print(tm.Minute, DEC);
    Serial.print(":");
    Serial.print(tm.Second, DEC);
    Serial.print("  ");
    Serial.print(tm.Day, DEC);
    Serial.print("/");
    Serial.print(tm.Month, DEC);
    Serial.print("/");
    Serial.print(tmYearToCalendar(tm.Year), DEC);
    Serial.print(" ");
    Serial.println(" ");
  } else {
    if (RTC.chipPresent()) {
      Serial.println("The RTC is stopped.  Please run the SetTime");
    } else {
      Serial.println("RTC read error!  Please check the circuitry.");
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

  String name = "";
  //0 : Normal, 1 : Eco, 2 : Maintenance, 3 : Config
  switch (_mode) {
    case 0:
      name = "normal";
      break;
    case 1:
      name = "eco";
      break;
    case 2:
      name = "maintenance";
      break;
    case 3:
      name = "configuration";
      break;
  }
  Serial.print("The new mode is ");
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

bool checkError() {
  bool error = false;
  if (!RTC.read(tm)) {
    //rtc error
    leds.color("RED", 1, "BLUE", 1);
    error = true;
  }


  bool sensorLightError = sensorLightValue < config.getValue("LUMIN_LOW") || sensorLightValue > config.getValue("LUMIN_HIGH");
  bool sensorTempError = sensorTempValue < config.getValue("MIN_TEMP_AIR") || sensorTempValue > config.getValue("MAX_TEMP_AIR");
  bool sensorPresError = sensorPresValue < config.getValue("PRESSURE_MIN") || sensorPresValue > config.getValue("PRESSURE_MAX");
  bool sensorHumError = sensorHumValue < config.getValue("HYGR_MINT") || sensorHumValue > config.getValue("HYGR_MAXT");
  if (sensorLightError || sensorTempError || sensorPresError || sensorHumError) {
    //data error
    leds.color("RED", 1, "GREEN", 3);
    error = true;
  }

  if (!bme.begin())
  {
    //sensor error
    leds.color("RED", 1, "GREEN", 1);
    error = true;
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
  bme.read(sensorPresValue, sensorTempValue, sensorHumValue, sensorTempUnit, sensorPresUnit);
}

void loop()
{
  checkPressedButton();
  if (mode != 3) {
    getSensorValues();
    if (!checkError()) {
      if (mode == 0) {
        leds.color("GREEN");
      }
      else if (mode == 1) {
        leds.color("BLUE");
      }
      else if (mode == 2) {
        leds.color("ORANGE");
      }
    }
  }
  else {
    leds.color("YELLOW");
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

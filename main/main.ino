#include "src/project_libs/Config/Config.h"
#include "src/project_libs/Led/Led.h"
#include <Wire.h>
//#include "/usr/share/arduino/libraries/Wire/Wire.h"
#include "src/imported_libs/DS1307RTC/DS1307RTC.h"
tmElements_t tm;

byte version = 2;
String batchNumber = "20200930A";

//0 : Normal, 1 : Eco, 2 : Maintenance, 3 : Config
int previousMode = 0;
int mode = 0;
Config config(version, batchNumber);
Led leds(8, 9, 1);
DS1307RTC clock;
long buttonPressedMs = millis();
bool buttonPressed = false;
bool checkStartPressedButton = true;

const byte buttonPinGreen = 2;
const byte buttonPinRed = 3;

void setup()
{

  Serial.begin(9600);
  //clock.begin();
  //clock.fillByYMD(2020,9,29);
  //clock.fillByHMS(16,12,20);
  //clock.fillDayOfWeek(TUE);
  //clock.setTime();
  config.showValues();
  //config.setValue("LUMINO", 0);
  pinMode(buttonPinRed, INPUT_PULLUP);
  pinMode(buttonPinGreen, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(buttonPinRed), clickButtonRedEvent, CHANGE);
  attachInterrupt(digitalPinToInterrupt(buttonPinGreen), clickButtonGreenEvent, CHANGE);
  
  showDate();


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
      Serial.println("The DS1307 is stopped.  Please run the SetTime");
      Serial.println("example to initialize the time and begin running.");
      Serial.println();
    } else {
      Serial.println("DS1307 read error!  Please check the circuitry.");
      Serial.println();
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
  else if (mode == 3)
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
  Serial.print("The new mode is ");
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

void loop()
{
  checkPressedButton();
  if (mode == 0) {
    leds.color("GREEN");
  }
  else if (mode == 1) {
    leds.color("BLUE");
  }
  else if (mode == 2) {
    leds.color("ORANGE");
  }
  else if (mode == 3) {
    leds.color("YELLOW");
    long lastActivity = config.getLastActivity();
    //go to normal if inactivity > 30m
    if ((millis() - lastActivity)/100 > (30 * 60*10)) {
      changeMode(0);
    }
    config.waitValues();
  }

  //leds.color("BLUE", 2);
  //leds.color("RED");

  //leds.color("RED", 10, "YELLOW", 3);
  //delay(300);
}

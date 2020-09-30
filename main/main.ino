#include "src/project_libs/Config/Config.h"
#include "src/project_libs/Led/Led.h"
#include "/usr/share/arduino/libraries/Wire/Wire.h"
#include <DS1307.h>

byte version = 2;
String batchNumber = "20200930A";

//0 : Normal, 1 : Eco, 2 : Maintenance, 3 : Config
int previousMode = 0;
int mode = 0;
Config config(version, batchNumber);
Led leds(8, 9, 1);
DS1307 clock;

long buttonPressedMs = millis();
bool buttonPressed = false;
bool checkStartPressedButton = true;

const byte buttonPinGreen = 2;
const byte buttonPinRed = 3;

void setup()
{

  Serial.begin(9600);
  clock.begin();
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
  clock.getTime();
  Serial.print(clock.hour, DEC);
  Serial.print(":");
  Serial.print(clock.minute, DEC);
  Serial.print(":");
  Serial.print(clock.second, DEC);
  Serial.print("  ");
  Serial.print(clock.dayOfMonth, DEC);
  Serial.print("/");
  Serial.print(clock.month, DEC);
  Serial.print("/");
  Serial.print(clock.year + 2000, DEC);
  Serial.print(" ");
  Serial.println(" ");
}

void clickButtonGreenEvent() {
  Serial.println("click green");
  buttonPressedMs = millis();
  buttonPressed = true;
}

void clickButtonRedEvent() {
  Serial.println("click red");
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
  }
  else if (mode == 2) {
  }
  else if (mode == 3) {
    long lastActivity = config.getLastActivity();
    //go to normal if inactivity > 30m
    if (millis() - lastActivity > (30*60 * 1000)) {
      changeMode(0);
    }
    config.waitValues();
  }

  //leds.color("BLUE", 2);
  //leds.color("RED");

  //leds.color("RED", 10, "YELLOW", 3);
  //delay(300);
}

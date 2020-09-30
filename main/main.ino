
#include "src/project_libs/Config/Config.h"
#include "src/project_libs/Led/Led.h"
#include <Wire.h>
#include <DS1307.h>

byte version = 1;

//0 : Normal, 1 : Eco, 2 : Maintenance, 3 : Config
int mode = 0;
Config config(version);
Led leds(8, 9, 1);
DS1307 clock;

long buttonPressedMs = millis();
bool buttonPressed = false;

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
  Serial.println("good 5s green");
}

void pressedButtonRed() {
  Serial.println("good 5s red");
}

void checkPressedButton() {
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

  if (mode == 0)
    leds.color("GREEN");

  //leds.color("BLUE", 2);
  //leds.color("RED");

  //leds.color("RED", 10, "YELLOW", 3);
  //delay(300);

  if (mode == 3)
    config.waitValues();
}

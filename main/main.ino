
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


const byte buttonPinGreen = 2;
const byte buttonPinRed = 3;

void clickButtonGreenEvent() {
  Serial.println("click green");
  long ms = millis();
  for (;;) {
    if ((millis() - ms) > (5 * 1000)) {
      if (digitalRead(buttonPinGreen) == 0)
        Serial.println("good 5s");
      break;
    }
  }
}

void clickButtonRedEvent() {
  Serial.println("click red");
}

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

void loop()
{

  if (mode == 0)
    leds.color("GREEN");

  //leds.color("BLUE", 2);
  //leds.color("RED");

  //leds.color("RED", 10, "YELLOW", 3);
  //delay(300);

  if (mode == 3)
    config.waitValues();

  /**Serial.print(hue);
    Serial.print(" ");**/
}

/**void write_String(char index, String data)
  {
  int _size = data.length();
  int i;
  for (i = 0; i < _size; i++)
  {
    EEPROM.update(index + i, data[i]);
  }
  EEPROM.update(index + _size, '\0');
  //EEPROM.commit();
  }

  String read_String(char add)
  {
  int i;
  char data[100];
  int len = 0;
  unsigned char k;
  k = EEPROM.read(add);
  while (k != '\0' && len < 500)
  {
    k = EEPROM.read(add + len);
    data[len] = k;
    len++;
  }
  data[len] = '\0';
  return String(data);
  }

  void showEEPROM()
  {
  pinMode(13, OUTPUT);
  for (int i = 0 ; i < EEPROM.length() ; i++) {
    Serial.println(EEPROM.read(i));
  }
  digitalWrite(13, HIGH);
  }
  void clearEEPROM()
  {
  pinMode(13, OUTPUT);
  for (int i = 0 ; i < EEPROM.length() ; i++) {
    EEPROM.update(i, 0);
  }
  digitalWrite(13, HIGH);
  }**/

















/**byte tes = 0;

  ChainableLED leds(7, 8, 1);
  void setup()
  {
  Serial.begin(9600);
  }

  float hue = 0.0;
  boolean up = true;

  void loop()
  {

    leds.setColorHSB(0, hue, 1, 0);

  delay(50);

  if (up)
    hue+= 0.025;
  else
    hue-= 0.025;

  if (hue>=1.0 && up)
    up = false;
  else if (hue<=0.0 && !up)
    up = true;

  hue = analogRead(A0)/100;
  hue = hue/10;
  Serial.print(hue);
  Serial.print(" ");
  }**/

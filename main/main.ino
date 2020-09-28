#include "src/imported_libs/ChainableLED/ChainableLED.h"
#include "src/project_libs/Config/Config.h"
Configuration configArray[] {
  {0, "VERSION", 0, 20, 1},
  {1, "LUMINO", 0, 1, 1},
  {255, "LUMIN_LOW", 0, 1023, 2},
  {768, "LUMIN_HIGH", 0, 1023, 2},
  {1, "TEMP_AIR", 0, 1, 1},
  { -10, "MIN_TEMP_AIR", -40, 85, 2},
  {60, "MAX_TEMP_AIR", -40, 85, 2},
  {1, "HYGR", 0, 1, 1},
  {0, "HYGR_MINT", -40, 85, 2},
  {50, "HYGR_MAXT", -40, 85, 2},
  {1, "PRESSURE", 0, 1, 1},
  {850, "PRESSURE_MIN", 300, 1100, 2},
  {1080, "PRESSURE_MAX", 300, 1080, 2}

};

byte version = 1;

//0 : Normal, 1 : Eco, 2 : Maintenance, 3 : Config
int mode = 3;

Config config(version, configArray);

ChainableLED leds(7, 8, 1);



void setup()
{
   
  Serial.begin(9600);
  config.showValues();
  //config.setValue("LUMINO", 0);
}

void loop()
{

  if(mode == 3)
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

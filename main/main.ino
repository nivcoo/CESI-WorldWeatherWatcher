#include "src/imported_libs/ChainableLED/ChainableLED.h"
#include <EEPROM.h>
ChainableLED leds(7, 8, 1);
byte tes = 0;
typedef struct {
  String name;
  int value;
  int size;
} config;
const config Config[] {
  {"LUMINO", 1, 1},
  {"LUMIN_LOW", 255, 2},
  {"LUMIN_HIGH", 768, 2},
  {"TEMP_AIR", 1, 1},
  {"MIN_TEMP_AIR", -10, 1},
  {"MAX_TEMP_AIR", 60, 1},
  {"HYGR", 1, 1},
  {"HYGR_MINT", 0, 1},
  {"HYGR_MAXT", 50, 1},
  {"PRESSURE", 1, 1},
  {"PRESSURE_MIN", 850, 2},
  {"PRESSURE_MAX", 1080, 2}

};

void write_int(int index, int value, int _size) {
  for (int i = 0; i < _size; i++)
  {
  
    EEPROM.update(index + i, ((byte) (value >> 8 * i)));
  }

}

int read_int(int index, int _size) {

  int value = 0;
  for (int i = 0; i < _size; i++)
  {
    value |= EEPROM.read(index + i)<< (8*i);
  }
  return value;
  

}

void setup()
{
  Serial.begin(9600);
  int index = 0;

  for (int i = 0; i < sizeof(Config) / sizeof(config); ++i) {
    //if(Config[i].value == 0)
    write_int(index, Config[i].value, Config[i].size);
    Serial.println(read_int(index, Config[i].size));
    index += Config[i].size;
  }

  /**for (int i = 0; i < EEPROM.length(); i++)
  {
    Serial.println(EEPROM.read(i));
  }**/
  /**std::map<std::string, int> configuration;
    configuration['LUMIN'] = 1;**/

  //write_config(0, "455");






  //EEPROM.update(0, 455);
  //write_String(0, "455");
  //write_String(4, "7895");
  //Serial.println(EEPROM.read(0));
  //Serial.println(read_String(0));
  //Serial.println(read_String(4));
  //write_String(0, "salut ca va toi ?");
  //Serial.println(read_String(0));
  //showEEPROM();
  //clearEEPROM();
}
float hue = 0.0;
void loop()
{
  leds.setColorHSB(0, hue, 1, 0.5);

  delay(50);



  hue = analogRead(A0) / 100;
  hue = hue / 10;
  /**Serial.print(hue);
    Serial.print(" ");**/
}

void write_String(char index, String data)
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
}

















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

#include "Config.h"

Configuration conf[] {
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

Config::Config(byte version)
{
	byte versionIndex = get("VERSION", 1);
	conf[versionIndex].value = version;

	if(version != readInt(get("VERSION", 0), conf[versionIndex].size)) 
		resetValues();
}

void Config::resetValues()
{
	int index = 0;
	for (int i = 0; i < sizeof(conf) / sizeof(conf); ++i) {
		writeInt(index, conf[i].value, conf[i].size);
		index += conf[i].size;
	}
}


String splitString(String data, char separator, int index)
{
  int found = 0;
  int strIndex[] = {0, -1};
  int maxIndex = data.length()-1;

  for(int i=0; i<=maxIndex && found<=index; i++){
    if(data.charAt(i)==separator || i==maxIndex){
        found++;
        strIndex[0] = strIndex[1]+1;
        strIndex[1] = (i == maxIndex) ? i+1 : i;
    }
  }
  return found>index ? data.substring(strIndex[0], strIndex[1]) : "";
}


void Config::waitValues() {
	String data;
	String name;
	int newValue;
	if (Serial.available() > 0) {
	
		data = Serial.readString();
		name = splitString(data, '=', 0);
		newValue = splitString(data, '=', 1).toInt();
		name.trim();
		name.toUpperCase();
		if(name == "SHOW" || name == "RESET") {
			if(name == "RESET") {
				resetValues();
			}
			showValues();
			return;
		} 
		int configIndex = get(name, 2);
		Configuration c = conf[configIndex];
		if(!get(name, 2)) {
			Serial.println("This parameter doesn't exist !");
			return;
		} else if (c.min > newValue || c.max < newValue) {
			Serial.print("Place set a number between ");
			Serial.print(c.min);
			Serial.print(" and ");
			Serial.println(c.max);
			return;
		} 
		setValue(name, newValue);
		Serial.print("New value for ");
		Serial.print(name);
		Serial.print(" = ");
		Serial.println(newValue); 
	}
}


int Config::getValue(String name) {
	int index = get(name, 0);
	return readInt(index, conf[get(name, 2)].size);
}


void Config::setValue(String name, int newValue){
	int index = get(name, 0);
	int indexConfig = get(name, 2);
	writeInt(index, newValue, conf[indexConfig].size);
}




int Config::get(String name, int type) {
	
	int result = 0;
	for (int i = 0; i < sizeof(conf) / sizeof(Configuration); ++i) {
		if(conf[i].name == name)
			break;
		if(type == 0) //get index in eeprom
			result += conf[i].size;
		if(type == 1) // get index in config
			result = i;
		if(type == 2) // check if exist
			result = 1;
		
	}
	return result;
}

void Config::writeInt(int index, int value, int _size) {
	for (int i = 0; i < _size; i++)
	{
		
		if(_size == 1)
			value = (int8_t) value;
		else if(_size == 2)
			value = (int16_t) value;
		EEPROM.update(index + i, ((byte) (value >> 8 * i)));
	}
}

int Config::readInt(int index, int _size) {
	
	int value = 0;	
	
	for (int i = 0; i < _size; i++)
	{
		value |= EEPROM.read(index + i) << (8 * i);
	}
	
	return value;
	
}

void Config::showValues() {
	Serial.println("------------------");
	for (int i = 0; i < sizeof(conf) / sizeof(Configuration); ++i) {
		//Serial.println(get(conf[i].name) readInt(index, conf[i].size));
		String name = conf[i].name;
		Serial.print(name);
		Serial.print(" : ");
		Serial.println(getValue(name));
	}
	Serial.println("------------------");
}


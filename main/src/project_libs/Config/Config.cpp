#include "Config.h"

typedef struct {
  int value;
  String name;
  int min;
  int max;
  int size;
} configuration;
configuration Configuration[] {
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
	byte versionIndex = getConfigIndex("VERSION");
	Configuration[versionIndex].value = version;

	if(version != readInt(getIndex("VERSION"), Configuration[versionIndex].size)) 
		resetValues();
}

void Config::resetValues()
{
	int index = 0;
	for (int i = 0; i < sizeof(Configuration) / sizeof(configuration); ++i) {
		writeInt(index, Configuration[i].value, Configuration[i].size);
		index += Configuration[i].size;
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
		int configIndex = getConfigIndex(name);
		configuration config = Configuration[configIndex];
		if(!exist(name)) {
			Serial.println("This parameter doesn't exist !");
			return;
		} else if (config.min > newValue || config.max < newValue) {
			Serial.print("Place set a number between ");
			Serial.print(config.min);
			Serial.print(" and ");
			Serial.println(config.max);
			return;
		} 
		setValue(name, newValue);
		Serial.print("New value for ");
		Serial.print(name);
		Serial.print(" = ");
		Serial.println(newValue);
		
		 
		
	}
}

bool Config::exist(String name) {
	
	bool exist = false;
	for (int i = 0; i < sizeof(Configuration) / sizeof(configuration); ++i) {
		if(Configuration[i].name != name)
			continue;
		exist = true;
		break;
	}
	return exist;
	
}

int Config::getValue(String name) {
	int index = getIndex(name);
	return readInt(index, Configuration[getConfigIndex(name)].size);
}


void Config::setValue(String name, int newValue){
	int index = getIndex(name);
	int indexConfig = getConfigIndex(name);
	writeInt(index, newValue, Configuration[indexConfig].size);
}

int Config::getConfigIndex(String name) {
	
	int index = 0;
	for (int i = 0; i < sizeof(Configuration) / sizeof(configuration); ++i) {
		if(Configuration[i].name != name)
			continue;
		index = i;
		break;
	}
	return index;
	
}


int Config::getIndex(String name) {
	
	int index = 0;
	for (int i = 0; i < sizeof(Configuration) / sizeof(configuration); ++i) {
		if(Configuration[i].name == name)
			break;
		index += Configuration[i].size;
		
	}
	return index;
	
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
	for (int i = 0; i < sizeof(Configuration) / sizeof(configuration); ++i) {
		//Serial.println(get(Configuration[i].name) readInt(index, Configuration[i].size));
		Serial.print(Configuration[i].name);
		Serial.print(" : ");
		Serial.println(getValue(Configuration[i].name));
	}
	Serial.println("------------------");
	
}


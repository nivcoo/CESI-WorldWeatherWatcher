#include "Config.h"




Config::Config(byte version, Configuration *c) :
    conf(c)
{
	Serial.println(sizeof(Configuration));
	Serial.println(sizeof(conf));
	Serial.println("------------------");
	for (int i = 0; i < sizeof(conf) / sizeof(Configuration); ++i) {
		//Serial.println(get(conf[i].name) readInt(index, conf[i].size));
		Serial.print(conf[i].name);
		Serial.print(" : ");
		Serial.println(getValue(conf[i].name));
	}
	Serial.println("------------------");
	Serial.println(sizeof(Configuration));
	Serial.println(sizeof(conf));
	
	byte versionIndex = getConfigIndex("VERSION");
	conf[versionIndex].value = version;

	if(version != readInt(getIndex("VERSION"), conf[versionIndex].size)) 
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
		int configIndex = getConfigIndex(name);
		Configuration c = conf[configIndex];
		if(!exist(name)) {
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

bool Config::exist(String name) {
	
	bool exist = false;
	for (int i = 0; i < sizeof(conf) / sizeof(conf); ++i) {
		if(conf[i].name != name)
			continue;
		exist = true;
		break;
	}
	return exist;
	
}

int Config::getValue(String name) {
	int index = getIndex(name);
	return readInt(index, conf[getConfigIndex(name)].size);
}


void Config::setValue(String name, int newValue){
	int index = getIndex(name);
	int indexConfig = getConfigIndex(name);
	writeInt(index, newValue, conf[indexConfig].size);
}

int Config::getConfigIndex(String name) {
	
	int index = 0;
	for (int i = 0; i < sizeof(conf) / sizeof(conf); ++i) {
		if(conf[i].name != name)
			continue;
		index = i;
		break;
	}
	return index;
	
}


int Config::getIndex(String name) {
	
	int index = 0;
	for (int i = 0; i < sizeof(conf) / sizeof(conf); ++i) {
		if(conf[i].name == name)
			break;
		index += conf[i].size;
		
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
	Serial.println(sizeof(Configuration));
	Serial.println(sizeof(conf));
	Serial.println("------------------");
	for (int i = 0; i < sizeof(conf) / sizeof(Configuration); ++i) {
		//Serial.println(get(conf[i].name) readInt(index, conf[i].size));
		Serial.print(conf[i].name);
		Serial.print(" : ");
		Serial.println(getValue(conf[i].name));
	}
	Serial.println("------------------");
	
}


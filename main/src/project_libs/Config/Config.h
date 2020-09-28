#ifndef __Config_h__
#define __Config_h__

#include "Arduino.h"
#include <EEPROM.h>
struct Configuration {
	int value;
	String name;
	int min;
	int max;
	int size;
};
class Config
{

public:
    Config(byte version, Configuration *c);
    
    
    void waitValues();
    
    int getValue(String name);
    void setValue(String name, int newValue);
    void showValues();

private:
	
	Configuration *conf;

    void writeInt(int index, int value, int _size);
    int readInt(int index, int _size);
    int getConfigIndex(String name);
    int getIndex(String name);
    bool exist(String name);
    
    void resetValues();
};

#endif


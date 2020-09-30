#ifndef __Config_h__
#define __Config_h__

#include "Arduino.h"

#include "../../imported_libs/EEPROM/EEPROM.h"
//#include "/usr/share/arduino/libraries/EEPROM/EEPROM.h"
typedef struct {
	int value;
	String name;
	int min;
	int max;
	int size;
} Configuration;
class Config
{

public:
    Config(byte version, String batchNumber);
    
    
    void waitValues();
    
    int getValue(String name);
    void setValue(String name, int newValue);
    void showValues();
    
    void getVersion();
    long getLastActivity();

private:
	
	String _batchNumber;

    void writeInt(int index, int value, int _size);
    int readInt(int index, int _size);
    int get(String name, int type);
    
    void resetValues();
};

#endif


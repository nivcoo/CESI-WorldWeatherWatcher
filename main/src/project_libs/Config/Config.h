#ifndef __Config_h__
#define __Config_h__

#include "Arduino.h"
#include <EEPROM.h>
class Config
{
public:
    Config(byte version);
    
    void waitValues();
    
    int getValue(String name);
    void setValue(String name, int newValue);
    void showValues();

private:

    void writeInt(int index, int value, int _size);
    int readInt(int index, int _size);
    int getConfigIndex(String name);
    int getIndex(String name);
    bool exist(String name);
    
    void resetValues();
};

#endif


#ifndef __Led_h__
#define __Led_h__

#include "Arduino.h"
#include "../../imported_libs/ChainableLED/ChainableLED.h"

typedef struct {
	String name;
	byte r;
	byte g;
	byte b;
} Color;
class Led
{

public:
    Led(byte pin, byte pin_data, byte led_number);
    
    void color(String color);
    void color(String firstColor, byte firstTime, String secondColor, byte secondTime);
    

private:

	byte _pin;
    byte _pin_data;
    byte _led_number;
    
    byte getIndex(String name);
    
    void setColor(String color);
};

#endif


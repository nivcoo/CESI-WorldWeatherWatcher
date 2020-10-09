#include "Led.h"

Color colors[] {
   {"BLUE", 0, 0, 255},
   {"RED", 255, 0, 0},
   {"GREEN", 0, 255, 0},
   {"YELLOW", 255, 255, 0},
   {"ORANGE", 255, 120, 0},
   {"WHITE", 255, 255, 255},
   {"OFF", 0, 0, 0}
   
};


Led::Led(byte pin, byte pinData, byte ledNumber) :
	_pin(pin), _pinData(pinData), _ledNumber(ledNumber)
{
	
}

byte Led::getIndex(String name) {
	
	byte result = 0;
	for (int i = 0; i < sizeof(colors) / sizeof(Color); ++i) {
		result = i;
		if(colors[i].name == name)
			break;
		
	}
	return result;
}


void Led::setColor(String color)
{
	byte index = getIndex(color);
	ChainableLED leds(_pin, _pinData, _ledNumber);
	leds.setColorRGB(0, colors[index].r, colors[index].g, colors[index].b);
	
}



String oldColor;
void Led::color(String color)
{
    if(oldColor != color) {
	    setColor(color);
	}
	oldColor = color;
	
}

bool color_change = true;
unsigned long temp = 0;
void Led::color(String firstColor, byte firstTime, String secondColor, byte secondTime)
{
	long actual = millis();
	int time = (color_change) ? firstTime : secondTime;

    if((actual - temp) / 1000 > time) {
        temp = actual;
        color_change = !color_change;

        if(color_change)
        	setColor(firstColor);
        else
        	setColor(secondColor);
    }


	

}



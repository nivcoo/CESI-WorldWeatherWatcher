#ifndef __SDCard_h__
#define __SDCard_h__

#include "Arduino.h"
#include "../../imported_libs/RTClib/RTClib.h"
#include <SD.h>

class SDCard
{

public:
    SDCard(byte chip, int fileSizeMax);
    bool writeTextInSD(String text);
    
    bool begin();

private: 

  	byte _chip;
  	int _fileSizeMax;
  	RTC_DS1307 SDTime;
    void checkSizeFiles(String startFile, int startNumber, int textSize);
};

#endif


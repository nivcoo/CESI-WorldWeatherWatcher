#include "SDCard.h"

RTC_DS1307 SDTime;

SDCard::SDCard(byte chip, int fileSizeMax) :
	_chip(chip), _fileSizeMax(fileSizeMax)
{
	
}

bool SDCard::begin() {
	return SD.begin(_chip);
}

void dateTime(uint16_t* date, uint16_t* time) {
  DateTime now = SDTime.now();
  *date = FAT_DATE(now.year(), now.month(), now.day());
  *time = FAT_TIME(now.hour(), now.minute(), now.second());
}

bool SDCard::writeTextInSD(String text) {
	
	DateTime now = SDTime.now();
    SdFile::dateTimeCallback(dateTime);
    String year = String(now.year() - 2000);
    String month = String(now.month());
    String day = String(now.day());
    String startFiles = year + month + day + "_";
    checkSizeFiles(startFiles, 0, text.length());
    String fileName = startFiles + 0 + ".log";
    File logFile = SD.open(fileName, FILE_WRITE);
    if (logFile) {
      logFile.println(text);
      logFile.close();
    }
    
    return (bool) logFile;
	
}


void SDCard::checkSizeFiles(String startFile, int startNumber, int textSize) {
	
	String extension = ".log";
  	String fileName = startFile + startNumber + extension;
 	File file = SD.open(fileName);
  	int fileSize = file.size() + textSize;
  	int sizeMax = _fileSizeMax;

 	if (fileSize > sizeMax) {
  	  	String newFileName = fileName;
  	  	File newFile = file;
  	  	int i = 0;
   	 	while (fileSize > sizeMax) {
  	    	newFileName = startFile + (startNumber + i) + extension;
  	    	newFile = SD.open(newFileName, FILE_WRITE);
  	    	fileSize = newFile.size() + textSize;
   		   	if(fileSize > sizeMax)
     	  		 newFile.close();
      		i++;
    	}
    	size_t n;
   		 uint8_t buf[64];
	    while ((n = file.read(buf, sizeof(buf))) > 0) {
	      newFile.write(buf, n);
	    }
	    newFile.close();
	    file.close();
	    SD.remove(fileName);
	} else
	    file.close();
	
}


void SDCard::end() {
	SD.end();
}




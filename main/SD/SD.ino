#define CHIP 4  //remove comment to use SD card
#include <SD.h> //remove comment to use SD card
#include "src/imported_libs/RTClib/RTClib.h"


RTC_DS1307 rtc;





void setup() {

  Serial.begin(9600);
  rtc.begin();
  if (!SD.begin(CHIP)) { //remove comment to use SD card
    Serial.println(F("SD Card loading Failed")); //remove comment to use SD card
    while (true); //remove comment to use SD card
  } //remove comment to use SD card
}
unsigned long lastWrite(0);
void dateTime(uint16_t* date, uint16_t* time) {
  DateTime now = rtc.now();

  // return date using FAT_DATE macro to format fields
  *date = FAT_DATE(now.year(), now.month(), now.day());

  // return time using FAT_TIME macro to format fields
  *time = FAT_TIME(now.hour(), now.minute(), now.second());
}
void checkSizeFiles(String startFile, int startNumber) {

  String extension = ".log";
  String fileName = startFile + startNumber + extension;
  File file = SD.open(fileName);
  int fileSize = file.size();
  if (fileSize > 1024) {
    String newFileName = getLogFileName(startFile, 1);
    File newFile = SD.open(newFileName, FILE_WRITE);
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
String getLogFileName(String startFile, int startNumber) {
  String extension = ".log";
  String fileName = startFile + startNumber + extension;
  File file = SD.open(fileName, FILE_WRITE);
  int fileSize = file.size();
  file.close();
  int i = 0;
  while (fileSize > 1024) {
    fileName = startFile + (startNumber + i) + extension;
    file = SD.open(fileName, FILE_WRITE);
    fileSize = file.size();
    file.close();
    i++;
  }
  return fileName;
}



bool SDWriteError = false; //remove comment to use SD card

void writeValues(bool sd) {

  if ((millis() - lastWrite) / 1000 > (2)) {
    lastWrite = millis();
    if (sd) {
      if (rtc.begin()) {
        DateTime now = rtc.now();
        SdFile::dateTimeCallback(dateTime);
        String year = String(now.year() - 2000);
        String month = String(now.month());
        String day = String(now.day());
        String startFiles = year + month + day + "_";
        checkSizeFiles(startFiles, 0);
        String fileName = startFiles + 0 + ".log";
        File logFile = SD.open(fileName, FILE_WRITE);
        Serial.println(fileName);
        Serial.println(logFile.size());
        if (logFile) {
          SDWriteError = false;
          logFile.print(F("["));
          logFile.print(now.day(), DEC);
          logFile.print(F("/"));
          logFile.print(now.month(), DEC);
          logFile.print(F("/"));
          logFile.print(now.year());
          logFile.print(F(" "));
          logFile.print(now.hour(), DEC);
          logFile.print(F(":"));
          logFile.print(now.minute(), DEC);
          logFile.print(F(":"));
          logFile.print(now.second(), DEC);
          logFile.print(F("]  "));
          for (int i = 0; i < 2; i++) {
            switch ('L') {
              case 'L':
                //rtc error
                logFile.print(F("Light : "));
                break;
              case 'T':
                //data error
                logFile.print(F("Temperature (°C) : "));
                break;
              case 'H':
                //sensor error
                logFile.print(F("Hygrometry (%) : "));
                break;
              case 'P':
                //gps error
                logFile.print(F("Pressure (HPa) : "));
                break;
            }
            logFile.print(10);
            logFile.print(F("   "));
          }
          logFile.print(F("|"));
          logFile.print(F("   "));
          logFile.print(F("Latitude : "));
          logFile.print(10);
          logFile.print(F("   "));
          logFile.print(F("Longitude : "));
          logFile.print(11);
          logFile.print(F("   "));
          logFile.print(F("Altitude (m) : "));
          logFile.print(12);
          logFile.print(F("   "));
          logFile.print(F("Satelites : "));
          logFile.println(13);
          logFile.close();
        } else {
          SDWriteError = true;
        }
      }





      //write in SD card
    }

  }
}


void loop() {
  writeValues(true);
}

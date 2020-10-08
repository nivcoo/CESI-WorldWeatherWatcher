#define CHIP 4  //remove comment to use SD card
#include <SPI.h> //remove comment to use SD card
#include <SD.h> //remove comment to use SD card
#include "src/imported_libs/DS1307RTC/DS1307RTC.h"

Sd2Card card; //remove comment to use SD card
SdVolume volume; //remove comment to use SD card
SdFile root; //remove comment to use SD card

DS1307RTC clock;
tmElements_t tm;





void setup() {

  Serial.begin(9600);
  if (!SD.begin(CHIP)) { //remove comment to use SD card
    Serial.println(F("SD Card loading Failed")); //remove comment to use SD card
    while (true); //remove comment to use SD card
  } //remove comment to use SD card
}
unsigned long lastWrite(0);

String getLogFileName(String year, String month, String day, int number) {
  String extension = ".txt";
  String fileName = year +  month + day + "_" + number + extension;
  File file = SD.open(fileName);
  int fileSize = file.size();
  file.close();
  int i = 0;
  while (fileSize > 4096) {
    fileName = year +  month + day + "_" + (number + i) + extension;
    file = SD.open(fileName);
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

      if (RTC.read(tm)) {
        String year = String(tmYearToCalendar(tm.Year) - 2000);
        String month = String(tm.Month);
        String day = String(tm.Day);
        String fileName = getLogFileName(year, month, day, 0);
        File logFile = SD.open(fileName, FILE_WRITE);
        if (logFile) {
          SDWriteError = false;
          logFile.print(F("["));
          logFile.print(tm.Day, DEC);
          logFile.print(F("/"));
          logFile.print(tm.Month, DEC);
          logFile.print(F("/"));
          logFile.print(tmYearToCalendar(tm.Year), DEC);
          logFile.print(F(" "));
          logFile.print(tm.Hour, DEC);
          logFile.print(F(":"));
          logFile.print(tm.Minute, DEC);
          logFile.print(F(":"));
          logFile.print(tm.Second, DEC);
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
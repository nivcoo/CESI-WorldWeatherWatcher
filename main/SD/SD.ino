#define CHIP 4  //remove comment to use SD card
#include <SPI.h> //remove comment to use SD card
#include <SD.h> //remove comment to use SD card
#include "src/imported_libs/DS1307RTC/DS1307RTC.h"

Sd2Card card; //remove comment to use SD card
SdVolume volume; //remove comment to use SD card
SdFile root; //remove comment to use SD card

DS1307RTC clock;
tmElements_t tm;



File  logFile; //remove comment to use SD card
String logFileName;


void setup() {

  Serial.begin(9600);
  if (!SD.begin(CHIP)) { //remove comment to use SD card
    Serial.println(F("SD Card loading Failed")); //remove comment to use SD card
    while (true); //remove comment to use SD card
  } //remove comment to use SD card
}
unsigned long lastWrite(0);

String getLogFileName(String year, String month, String day, int number, int stringSize) {
  String extension = ".txt";
  String fileName = year +  month + day + "_" + number + extension;
  File file = SD.open(fileName);
  int fileSize = file.size() + stringSize;
  file.close();
  int i = 0;
  while (fileSize > 4096) {
    fileName = year +  month + day + "_" + (number + i) + extension;
    file = SD.open(fileName);
    fileSize = file.size() + stringSize;
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
        String text = F("[" + day + "/" + month + "/" + tmYearToCalendar(tm.Year) + " " + tm.Hour + ":" + tm.Minute +  ":" + tm.Second + "  Temperature (°C) : " + 1 + "  Hygrometry (%) : " + 1 + "  Pressure (HPa) : " + 1 + "  Latitude : " + 1 + "  Longitude : " + 1 + "  Altitude (m) : " + 1 + "  Saltelites : " + 1);

        String fileName = getLogFileName(year, month, day, 0, text.length());
        File logFile = SD.open(fileName, FILE_WRITE);
        if (logFile) {
          SDWriteError = false;
          Serial.print("Writing to ");
          Serial.print(fileName);
          Serial.print(logFile.size());
          Serial.println("   ");
          logFile.println(text);
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

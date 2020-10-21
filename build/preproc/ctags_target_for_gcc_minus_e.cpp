# 1 "/home/nicolas/github/CESI-WorldWeatherWatcher/main/main.ino"
# 2 "/home/nicolas/github/CESI-WorldWeatherWatcher/main/main.ino" 2
# 3 "/home/nicolas/github/CESI-WorldWeatherWatcher/main/main.ino" 2
# 4 "/home/nicolas/github/CESI-WorldWeatherWatcher/main/main.ino" 2
# 5 "/home/nicolas/github/CESI-WorldWeatherWatcher/main/main.ino" 2
# 6 "/home/nicolas/github/CESI-WorldWeatherWatcher/main/main.ino" 2
# 7 "/home/nicolas/github/CESI-WorldWeatherWatcher/main/main.ino" 2
# 8 "/home/nicolas/github/CESI-WorldWeatherWatcher/main/main.ino" 2
# 27 "/home/nicolas/github/CESI-WorldWeatherWatcher/main/main.ino"
String batchNumber = "C3W20_19915";

SoftwareSerial gps(4, 5);
TinyGPS GPS;
BME280I2C bme;

RTC_DS1307 rtc;
Config config(1, batchNumber);
Led leds(8, 9, 1);
SDCard card(4, config.getValue("FILE_MAX_SIZE"));
byte previousMode = 0;
byte mode = 0;
unsigned long buttonPressedMs = millis();
bool buttonPressed = false;
bool checkStartPressedButton = true;
float gpsLon(0), gpsLat(0), gpsAlt(0);
bool SDWriteError = false;

struct Sensor {
  char id;
  String name;
  bool error;
  float average;
  float values[3];
};
Sensor sensors[] {
  {'L', "Light (Lumen)" , 0, 0, {}},
  {'T', "Temperature (°C)", 0, 0, {}},
  {'H',"Hygrometry (%)", 0, 0, {}},
  {'P',"Pressure (HPa)", 0, 0, {}},
  {'W',"Water Temp (°C)", 1, 0, {}},
  {'C',"Water Current (Nœuds)", 1, 0, {}},
  {'F',"Wind Force (Nœuds)", 1, 0, {}},
  {'F',"Fine Particles (%)", 1, 0, {}}
};

void setup()
{
  Serial.begin(9600);
  rtc.begin();
  while (!card.begin()) {
    Serial.println((reinterpret_cast<const __FlashStringHelper *>(
# 68 "/home/nicolas/github/CESI-WorldWeatherWatcher/main/main.ino" 3
                  (__extension__({static const char __c[] __attribute__((__progmem__)) = (
# 68 "/home/nicolas/github/CESI-WorldWeatherWatcher/main/main.ino"
                  "SD Card loading Failed"
# 68 "/home/nicolas/github/CESI-WorldWeatherWatcher/main/main.ino" 3
                  ); &__c[0];}))
# 68 "/home/nicolas/github/CESI-WorldWeatherWatcher/main/main.ino"
                  )));
    delay(5000);
  }
  gps.begin(9600);
  config.showValues();
  pinMode(3, 0x2);
  pinMode(2, 0x2);
  attachInterrupt(((3) == 2 ? 0 : ((3) == 3 ? 1 : ((3) >= 18 && (3) <= 21 ? 23 - (3) : -1))), clickButtonRedEvent, 2);
  attachInterrupt(((2) == 2 ? 0 : ((2) == 3 ? 1 : ((2) >= 18 && (2) <= 21 ? 23 - (2) : -1))), clickButtonGreenEvent, 2);
}

void clickButtonGreenEvent() {
  buttonPressedMs = millis();
  buttonPressed = true;
}

void clickButtonRedEvent() {
  buttonPressedMs = millis();
  buttonPressed = true;
}

void pressedButtonGreen() {
  if (mode == 0)
    changeMode(1);
  else if (mode == 1)
    changeMode(0);
}

void pressedButtonRed() {
  if (mode == 2 || mode == 3) {
    changeMode(previousMode);
    while (!card.begin()) {
      Serial.println((reinterpret_cast<const __FlashStringHelper *>(
# 100 "/home/nicolas/github/CESI-WorldWeatherWatcher/main/main.ino" 3
                    (__extension__({static const char __c[] __attribute__((__progmem__)) = (
# 100 "/home/nicolas/github/CESI-WorldWeatherWatcher/main/main.ino"
                    "SD Card loading Failed"
# 100 "/home/nicolas/github/CESI-WorldWeatherWatcher/main/main.ino" 3
                    ); &__c[0];}))
# 100 "/home/nicolas/github/CESI-WorldWeatherWatcher/main/main.ino"
                    )));
      delay(5000);
    }
  }
  else {
    previousMode = mode;
    changeMode(2);
    card.end();
  }
}

void changeMode(int _mode) {
  mode = _mode;

  String name = (reinterpret_cast<const __FlashStringHelper *>(
# 114 "/home/nicolas/github/CESI-WorldWeatherWatcher/main/main.ino" 3
               (__extension__({static const char __c[] __attribute__((__progmem__)) = (
# 114 "/home/nicolas/github/CESI-WorldWeatherWatcher/main/main.ino"
               ""
# 114 "/home/nicolas/github/CESI-WorldWeatherWatcher/main/main.ino" 3
               ); &__c[0];}))
# 114 "/home/nicolas/github/CESI-WorldWeatherWatcher/main/main.ino"
               ));
  switch (_mode) {
    case 0:
      name = (reinterpret_cast<const __FlashStringHelper *>(
# 117 "/home/nicolas/github/CESI-WorldWeatherWatcher/main/main.ino" 3
            (__extension__({static const char __c[] __attribute__((__progmem__)) = (
# 117 "/home/nicolas/github/CESI-WorldWeatherWatcher/main/main.ino"
            "normal"
# 117 "/home/nicolas/github/CESI-WorldWeatherWatcher/main/main.ino" 3
            ); &__c[0];}))
# 117 "/home/nicolas/github/CESI-WorldWeatherWatcher/main/main.ino"
            ));
      break;
    case 1:
      name = (reinterpret_cast<const __FlashStringHelper *>(
# 120 "/home/nicolas/github/CESI-WorldWeatherWatcher/main/main.ino" 3
            (__extension__({static const char __c[] __attribute__((__progmem__)) = (
# 120 "/home/nicolas/github/CESI-WorldWeatherWatcher/main/main.ino"
            "eco"
# 120 "/home/nicolas/github/CESI-WorldWeatherWatcher/main/main.ino" 3
            ); &__c[0];}))
# 120 "/home/nicolas/github/CESI-WorldWeatherWatcher/main/main.ino"
            ));
      break;
    case 2:
      name = (reinterpret_cast<const __FlashStringHelper *>(
# 123 "/home/nicolas/github/CESI-WorldWeatherWatcher/main/main.ino" 3
            (__extension__({static const char __c[] __attribute__((__progmem__)) = (
# 123 "/home/nicolas/github/CESI-WorldWeatherWatcher/main/main.ino"
            "maintenance"
# 123 "/home/nicolas/github/CESI-WorldWeatherWatcher/main/main.ino" 3
            ); &__c[0];}))
# 123 "/home/nicolas/github/CESI-WorldWeatherWatcher/main/main.ino"
            ));
      break;
    case 3:
      name = (reinterpret_cast<const __FlashStringHelper *>(
# 126 "/home/nicolas/github/CESI-WorldWeatherWatcher/main/main.ino" 3
            (__extension__({static const char __c[] __attribute__((__progmem__)) = (
# 126 "/home/nicolas/github/CESI-WorldWeatherWatcher/main/main.ino"
            "configuration"
# 126 "/home/nicolas/github/CESI-WorldWeatherWatcher/main/main.ino" 3
            ); &__c[0];}))
# 126 "/home/nicolas/github/CESI-WorldWeatherWatcher/main/main.ino"
            ));
      break;
  }
  Serial.print((reinterpret_cast<const __FlashStringHelper *>(
# 129 "/home/nicolas/github/CESI-WorldWeatherWatcher/main/main.ino" 3
              (__extension__({static const char __c[] __attribute__((__progmem__)) = (
# 129 "/home/nicolas/github/CESI-WorldWeatherWatcher/main/main.ino"
              "The new mode is "
# 129 "/home/nicolas/github/CESI-WorldWeatherWatcher/main/main.ino" 3
              ); &__c[0];}))
# 129 "/home/nicolas/github/CESI-WorldWeatherWatcher/main/main.ino"
              )));
  Serial.println(name);
}

void checkPressedButton() {
  if (checkStartPressedButton && digitalRead(3) == 0) {
    // go into config mode
    changeMode(3);
  }
  checkStartPressedButton = false;
  if ((millis() - buttonPressedMs) > (5 * 1000) && buttonPressed) {

    if (digitalRead(2) == 0) {
      pressedButtonGreen();
    } else if (digitalRead(3) == 0) {
      pressedButtonRed();
    }
    buttonPressed = false;
  }
}

void addSensorValue(float *values, float value) {
  for (int i = 0; i < 3 - 1; i++) {
    values[i] = values[i + 1];
  }
  values[3 - 1] = value;
}

float getSensorAverage(float *values) {
  float avr = 0;
  for (int i = 0; i < 3; i++) {
    avr += values[i];
  }
  return (avr / 3);
}

unsigned long lastSuccess(0);

bool gpsEco = false;
bool gpsError = false;

byte getSensorValues() {
  int code = 0;
  float sensorTempValue(0), sensorHumValue(0), sensorPresValue(0);
  int sensorLightValue = analogRead(0);
  bme.read(sensorPresValue, sensorTempValue, sensorHumValue, BME280::TempUnit_Celsius, BME280::PresUnit_hPa);
  gpsEco = !gpsEco || mode != 1;

  if (gpsEco) {
    bool updateGPS = false;
    for (unsigned long start = millis(); millis() - start < 1000;)
    {
      while (gps.available())
      {
        char c = gps.read();
        if (GPS.encode(c))
          updateGPS = true;
      }
    }
    if (updateGPS)
    {
      gpsError = false;
      unsigned long age;
      GPS.f_get_position(&gpsLat, &gpsLon, &age);
    } else
      gpsError = true;
  }

  bool sensorLightError = (sensorLightValue < config.getValue((reinterpret_cast<const __FlashStringHelper *>(
# 197 "/home/nicolas/github/CESI-WorldWeatherWatcher/main/main.ino" 3
                                                             (__extension__({static const char __c[] __attribute__((__progmem__)) = (
# 197 "/home/nicolas/github/CESI-WorldWeatherWatcher/main/main.ino"
                                                             "LUMIN_LOW"
# 197 "/home/nicolas/github/CESI-WorldWeatherWatcher/main/main.ino" 3
                                                             ); &__c[0];}))
# 197 "/home/nicolas/github/CESI-WorldWeatherWatcher/main/main.ino"
                                                             ))) || sensorLightValue > config.getValue((reinterpret_cast<const __FlashStringHelper *>(
# 197 "/home/nicolas/github/CESI-WorldWeatherWatcher/main/main.ino" 3
                                                                                                                   (__extension__({static const char __c[] __attribute__((__progmem__)) = (
# 197 "/home/nicolas/github/CESI-WorldWeatherWatcher/main/main.ino"
                                                                                                                   "LUMIN_HIGH"
# 197 "/home/nicolas/github/CESI-WorldWeatherWatcher/main/main.ino" 3
                                                                                                                   ); &__c[0];}))
# 197 "/home/nicolas/github/CESI-WorldWeatherWatcher/main/main.ino"
                                                                                                                   )))) && config.getValue((reinterpret_cast<const __FlashStringHelper *>(
# 197 "/home/nicolas/github/CESI-WorldWeatherWatcher/main/main.ino" 3
                                                                                                                                                        (__extension__({static const char __c[] __attribute__((__progmem__)) = (
# 197 "/home/nicolas/github/CESI-WorldWeatherWatcher/main/main.ino"
                                                                                                                                                        "LUMINO"
# 197 "/home/nicolas/github/CESI-WorldWeatherWatcher/main/main.ino" 3
                                                                                                                                                        ); &__c[0];}))
# 197 "/home/nicolas/github/CESI-WorldWeatherWatcher/main/main.ino"
                                                                                                                                                        )));
  bool sensorTempError = (sensorTempValue < config.getValue((reinterpret_cast<const __FlashStringHelper *>(
# 198 "/home/nicolas/github/CESI-WorldWeatherWatcher/main/main.ino" 3
                                                           (__extension__({static const char __c[] __attribute__((__progmem__)) = (
# 198 "/home/nicolas/github/CESI-WorldWeatherWatcher/main/main.ino"
                                                           "MIN_TEMP_AIR"
# 198 "/home/nicolas/github/CESI-WorldWeatherWatcher/main/main.ino" 3
                                                           ); &__c[0];}))
# 198 "/home/nicolas/github/CESI-WorldWeatherWatcher/main/main.ino"
                                                           ))) || sensorTempValue > config.getValue((reinterpret_cast<const __FlashStringHelper *>(
# 198 "/home/nicolas/github/CESI-WorldWeatherWatcher/main/main.ino" 3
                                                                                                                   (__extension__({static const char __c[] __attribute__((__progmem__)) = (
# 198 "/home/nicolas/github/CESI-WorldWeatherWatcher/main/main.ino"
                                                                                                                   "MAX_TEMP_AIR"
# 198 "/home/nicolas/github/CESI-WorldWeatherWatcher/main/main.ino" 3
                                                                                                                   ); &__c[0];}))
# 198 "/home/nicolas/github/CESI-WorldWeatherWatcher/main/main.ino"
                                                                                                                   )))) && config.getValue((reinterpret_cast<const __FlashStringHelper *>(
# 198 "/home/nicolas/github/CESI-WorldWeatherWatcher/main/main.ino" 3
                                                                                                                                                          (__extension__({static const char __c[] __attribute__((__progmem__)) = (
# 198 "/home/nicolas/github/CESI-WorldWeatherWatcher/main/main.ino"
                                                                                                                                                          "TEMP_AIR"
# 198 "/home/nicolas/github/CESI-WorldWeatherWatcher/main/main.ino" 3
                                                                                                                                                          ); &__c[0];}))
# 198 "/home/nicolas/github/CESI-WorldWeatherWatcher/main/main.ino"
                                                                                                                                                          )));
  bool sensorPresError = (sensorPresValue < config.getValue((reinterpret_cast<const __FlashStringHelper *>(
# 199 "/home/nicolas/github/CESI-WorldWeatherWatcher/main/main.ino" 3
                                                           (__extension__({static const char __c[] __attribute__((__progmem__)) = (
# 199 "/home/nicolas/github/CESI-WorldWeatherWatcher/main/main.ino"
                                                           "PRESSURE_MIN"
# 199 "/home/nicolas/github/CESI-WorldWeatherWatcher/main/main.ino" 3
                                                           ); &__c[0];}))
# 199 "/home/nicolas/github/CESI-WorldWeatherWatcher/main/main.ino"
                                                           ))) || sensorPresValue > config.getValue((reinterpret_cast<const __FlashStringHelper *>(
# 199 "/home/nicolas/github/CESI-WorldWeatherWatcher/main/main.ino" 3
                                                                                                                   (__extension__({static const char __c[] __attribute__((__progmem__)) = (
# 199 "/home/nicolas/github/CESI-WorldWeatherWatcher/main/main.ino"
                                                                                                                   "PRESSURE_MAX"
# 199 "/home/nicolas/github/CESI-WorldWeatherWatcher/main/main.ino" 3
                                                                                                                   ); &__c[0];}))
# 199 "/home/nicolas/github/CESI-WorldWeatherWatcher/main/main.ino"
                                                                                                                   )))) && config.getValue((reinterpret_cast<const __FlashStringHelper *>(
# 199 "/home/nicolas/github/CESI-WorldWeatherWatcher/main/main.ino" 3
                                                                                                                                                          (__extension__({static const char __c[] __attribute__((__progmem__)) = (
# 199 "/home/nicolas/github/CESI-WorldWeatherWatcher/main/main.ino"
                                                                                                                                                          "PRESSURE"
# 199 "/home/nicolas/github/CESI-WorldWeatherWatcher/main/main.ino" 3
                                                                                                                                                          ); &__c[0];}))
# 199 "/home/nicolas/github/CESI-WorldWeatherWatcher/main/main.ino"
                                                                                                                                                          )));
  bool sensorHumError = (sensorTempValue < config.getValue((reinterpret_cast<const __FlashStringHelper *>(
# 200 "/home/nicolas/github/CESI-WorldWeatherWatcher/main/main.ino" 3
                                                          (__extension__({static const char __c[] __attribute__((__progmem__)) = (
# 200 "/home/nicolas/github/CESI-WorldWeatherWatcher/main/main.ino"
                                                          "HYGR_MINT"
# 200 "/home/nicolas/github/CESI-WorldWeatherWatcher/main/main.ino" 3
                                                          ); &__c[0];}))
# 200 "/home/nicolas/github/CESI-WorldWeatherWatcher/main/main.ino"
                                                          ))) || sensorTempValue > config.getValue((reinterpret_cast<const __FlashStringHelper *>(
# 200 "/home/nicolas/github/CESI-WorldWeatherWatcher/main/main.ino" 3
                                                                                                               (__extension__({static const char __c[] __attribute__((__progmem__)) = (
# 200 "/home/nicolas/github/CESI-WorldWeatherWatcher/main/main.ino"
                                                                                                               "HYGR_MAXT"
# 200 "/home/nicolas/github/CESI-WorldWeatherWatcher/main/main.ino" 3
                                                                                                               ); &__c[0];}))
# 200 "/home/nicolas/github/CESI-WorldWeatherWatcher/main/main.ino"
                                                                                                               )))) && config.getValue((reinterpret_cast<const __FlashStringHelper *>(
# 200 "/home/nicolas/github/CESI-WorldWeatherWatcher/main/main.ino" 3
                                                                                                                                                   (__extension__({static const char __c[] __attribute__((__progmem__)) = (
# 200 "/home/nicolas/github/CESI-WorldWeatherWatcher/main/main.ino"
                                                                                                                                                   "HYGR"
# 200 "/home/nicolas/github/CESI-WorldWeatherWatcher/main/main.ino" 3
                                                                                                                                                   ); &__c[0];}))
# 200 "/home/nicolas/github/CESI-WorldWeatherWatcher/main/main.ino"
                                                                                                                                                   )));
  if (!rtc.begin()) {
    code = 1;
  }
  else if (sensorLightError || sensorTempError || sensorPresError || sensorHumError) {
    code = 2;
  }
  else if (!bme.begin())
  {
    code = 3;
  }
  else if (gpsError)
  {
    code = 4;
  }
  else if (SDWriteError) {
    code = 5;
  }
  /**else if (SDFull) {
    code = 6;
    }**/
  float value = 0;
  for (int i = 0; i < sizeof(sensors) / sizeof(Sensor); i++) {
    switch (sensors[i].id) {
      case 'L':
        if (sensorLightError)
          sensors[i].error = true;
        else {
          sensors[i].error = false;
          value = sensorLightValue;
          addSensorValue(sensors[i].values, value);
        }
        break;
      case 'T':
        if (sensorTempError || code == 3)
          sensors[i].error = true;
        else {
          sensors[i].error = false;
          value = sensorTempValue;
          addSensorValue(sensors[i].values, value);
        }
        break;
      case 'H':
        if (sensorHumError || code == 3)
          sensors[i].error = true;
        else {
          sensors[i].error = false;
          value = sensorHumValue;
          addSensorValue(sensors[i].values, value);
        }
        break;
      case 'P':
        if (sensorPresError || code == 3)
          sensors[i].error = true;
        else {
          sensors[i].error = false;
          value = sensorPresValue;
          addSensorValue(sensors[i].values, value);
        }
        break;
    }
    sensors[i].average = getSensorAverage(sensors[i].values);
  }
  return code;
}
unsigned long lastWrite(0);

void writeValues(bool sd) {
  if ((millis() - lastWrite) / 1000 > (60 * config.getValue((reinterpret_cast<const __FlashStringHelper *>(
# 268 "/home/nicolas/github/CESI-WorldWeatherWatcher/main/main.ino" 3
                                                           (__extension__({static const char __c[] __attribute__((__progmem__)) = (
# 268 "/home/nicolas/github/CESI-WorldWeatherWatcher/main/main.ino"
                                                           "LOG_INTERVAL"
# 268 "/home/nicolas/github/CESI-WorldWeatherWatcher/main/main.ino" 3
                                                           ); &__c[0];}))
# 268 "/home/nicolas/github/CESI-WorldWeatherWatcher/main/main.ino"
                                                           ))) * ((mode == 1) ? 2 : 1 )) ) {
    lastWrite = millis();
    if (rtc.begin()) {
      DateTime now = rtc.now();
      String text = (reinterpret_cast<const __FlashStringHelper *>(
# 272 "/home/nicolas/github/CESI-WorldWeatherWatcher/main/main.ino" 3
                   (__extension__({static const char __c[] __attribute__((__progmem__)) = (
# 272 "/home/nicolas/github/CESI-WorldWeatherWatcher/main/main.ino"
                   "["
# 272 "/home/nicolas/github/CESI-WorldWeatherWatcher/main/main.ino" 3
                   ); &__c[0];}))
# 272 "/home/nicolas/github/CESI-WorldWeatherWatcher/main/main.ino"
                   ));
      text += now.day();
      text += (reinterpret_cast<const __FlashStringHelper *>(
# 274 "/home/nicolas/github/CESI-WorldWeatherWatcher/main/main.ino" 3
             (__extension__({static const char __c[] __attribute__((__progmem__)) = (
# 274 "/home/nicolas/github/CESI-WorldWeatherWatcher/main/main.ino"
             "/"
# 274 "/home/nicolas/github/CESI-WorldWeatherWatcher/main/main.ino" 3
             ); &__c[0];}))
# 274 "/home/nicolas/github/CESI-WorldWeatherWatcher/main/main.ino"
             ));
      text += now.month();
      text += (reinterpret_cast<const __FlashStringHelper *>(
# 276 "/home/nicolas/github/CESI-WorldWeatherWatcher/main/main.ino" 3
             (__extension__({static const char __c[] __attribute__((__progmem__)) = (
# 276 "/home/nicolas/github/CESI-WorldWeatherWatcher/main/main.ino"
             "/"
# 276 "/home/nicolas/github/CESI-WorldWeatherWatcher/main/main.ino" 3
             ); &__c[0];}))
# 276 "/home/nicolas/github/CESI-WorldWeatherWatcher/main/main.ino"
             ));
      text += now.year();
      text += (reinterpret_cast<const __FlashStringHelper *>(
# 278 "/home/nicolas/github/CESI-WorldWeatherWatcher/main/main.ino" 3
             (__extension__({static const char __c[] __attribute__((__progmem__)) = (
# 278 "/home/nicolas/github/CESI-WorldWeatherWatcher/main/main.ino"
             " "
# 278 "/home/nicolas/github/CESI-WorldWeatherWatcher/main/main.ino" 3
             ); &__c[0];}))
# 278 "/home/nicolas/github/CESI-WorldWeatherWatcher/main/main.ino"
             ));
      text += now.hour();
      text += (reinterpret_cast<const __FlashStringHelper *>(
# 280 "/home/nicolas/github/CESI-WorldWeatherWatcher/main/main.ino" 3
             (__extension__({static const char __c[] __attribute__((__progmem__)) = (
# 280 "/home/nicolas/github/CESI-WorldWeatherWatcher/main/main.ino"
             ":"
# 280 "/home/nicolas/github/CESI-WorldWeatherWatcher/main/main.ino" 3
             ); &__c[0];}))
# 280 "/home/nicolas/github/CESI-WorldWeatherWatcher/main/main.ino"
             ));
      text += now.minute();
      text += (reinterpret_cast<const __FlashStringHelper *>(
# 282 "/home/nicolas/github/CESI-WorldWeatherWatcher/main/main.ino" 3
             (__extension__({static const char __c[] __attribute__((__progmem__)) = (
# 282 "/home/nicolas/github/CESI-WorldWeatherWatcher/main/main.ino"
             ":"
# 282 "/home/nicolas/github/CESI-WorldWeatherWatcher/main/main.ino" 3
             ); &__c[0];}))
# 282 "/home/nicolas/github/CESI-WorldWeatherWatcher/main/main.ino"
             ));
      text += now.second();
      text += (reinterpret_cast<const __FlashStringHelper *>(
# 284 "/home/nicolas/github/CESI-WorldWeatherWatcher/main/main.ino" 3
             (__extension__({static const char __c[] __attribute__((__progmem__)) = (
# 284 "/home/nicolas/github/CESI-WorldWeatherWatcher/main/main.ino"
             "]  "
# 284 "/home/nicolas/github/CESI-WorldWeatherWatcher/main/main.ino" 3
             ); &__c[0];}))
# 284 "/home/nicolas/github/CESI-WorldWeatherWatcher/main/main.ino"
             ));
      for (int i = 0; i < sizeof(sensors) / sizeof(Sensor); i++) {
        text += sensors[i].name + " : ";
        float averageValue = sensors[i].average;
        if (sensors[i].error || isnan((averageValue)))
          text += (reinterpret_cast<const __FlashStringHelper *>(
# 289 "/home/nicolas/github/CESI-WorldWeatherWatcher/main/main.ino" 3
                 (__extension__({static const char __c[] __attribute__((__progmem__)) = (
# 289 "/home/nicolas/github/CESI-WorldWeatherWatcher/main/main.ino"
                 "NA"
# 289 "/home/nicolas/github/CESI-WorldWeatherWatcher/main/main.ino" 3
                 ); &__c[0];}))
# 289 "/home/nicolas/github/CESI-WorldWeatherWatcher/main/main.ino"
                 ));
        else
          text += averageValue;
        text += (reinterpret_cast<const __FlashStringHelper *>(
# 292 "/home/nicolas/github/CESI-WorldWeatherWatcher/main/main.ino" 3
               (__extension__({static const char __c[] __attribute__((__progmem__)) = (
# 292 "/home/nicolas/github/CESI-WorldWeatherWatcher/main/main.ino"
               "  "
# 292 "/home/nicolas/github/CESI-WorldWeatherWatcher/main/main.ino" 3
               ); &__c[0];}))
# 292 "/home/nicolas/github/CESI-WorldWeatherWatcher/main/main.ino"
               ));
      }
      text += (reinterpret_cast<const __FlashStringHelper *>(
# 294 "/home/nicolas/github/CESI-WorldWeatherWatcher/main/main.ino" 3
             (__extension__({static const char __c[] __attribute__((__progmem__)) = (
# 294 "/home/nicolas/github/CESI-WorldWeatherWatcher/main/main.ino"
             "|"
# 294 "/home/nicolas/github/CESI-WorldWeatherWatcher/main/main.ino" 3
             ); &__c[0];}))
# 294 "/home/nicolas/github/CESI-WorldWeatherWatcher/main/main.ino"
             ));
      text += (reinterpret_cast<const __FlashStringHelper *>(
# 295 "/home/nicolas/github/CESI-WorldWeatherWatcher/main/main.ino" 3
             (__extension__({static const char __c[] __attribute__((__progmem__)) = (
# 295 "/home/nicolas/github/CESI-WorldWeatherWatcher/main/main.ino"
             "  "
# 295 "/home/nicolas/github/CESI-WorldWeatherWatcher/main/main.ino" 3
             ); &__c[0];}))
# 295 "/home/nicolas/github/CESI-WorldWeatherWatcher/main/main.ino"
             ));
      text += (reinterpret_cast<const __FlashStringHelper *>(
# 296 "/home/nicolas/github/CESI-WorldWeatherWatcher/main/main.ino" 3
             (__extension__({static const char __c[] __attribute__((__progmem__)) = (
# 296 "/home/nicolas/github/CESI-WorldWeatherWatcher/main/main.ino"
             "Latitude : "
# 296 "/home/nicolas/github/CESI-WorldWeatherWatcher/main/main.ino" 3
             ); &__c[0];}))
# 296 "/home/nicolas/github/CESI-WorldWeatherWatcher/main/main.ino"
             ));
      if (gpsLat == TinyGPS::GPS_INVALID_F_ANGLE || gpsLat == 0)
        text += (reinterpret_cast<const __FlashStringHelper *>(
# 298 "/home/nicolas/github/CESI-WorldWeatherWatcher/main/main.ino" 3
               (__extension__({static const char __c[] __attribute__((__progmem__)) = (
# 298 "/home/nicolas/github/CESI-WorldWeatherWatcher/main/main.ino"
               "NA"
# 298 "/home/nicolas/github/CESI-WorldWeatherWatcher/main/main.ino" 3
               ); &__c[0];}))
# 298 "/home/nicolas/github/CESI-WorldWeatherWatcher/main/main.ino"
               ));
      else
        text += gpsLat;
      text += (reinterpret_cast<const __FlashStringHelper *>(
# 301 "/home/nicolas/github/CESI-WorldWeatherWatcher/main/main.ino" 3
             (__extension__({static const char __c[] __attribute__((__progmem__)) = (
# 301 "/home/nicolas/github/CESI-WorldWeatherWatcher/main/main.ino"
             "  "
# 301 "/home/nicolas/github/CESI-WorldWeatherWatcher/main/main.ino" 3
             ); &__c[0];}))
# 301 "/home/nicolas/github/CESI-WorldWeatherWatcher/main/main.ino"
             ));
      text += (reinterpret_cast<const __FlashStringHelper *>(
# 302 "/home/nicolas/github/CESI-WorldWeatherWatcher/main/main.ino" 3
             (__extension__({static const char __c[] __attribute__((__progmem__)) = (
# 302 "/home/nicolas/github/CESI-WorldWeatherWatcher/main/main.ino"
             "Longitude : "
# 302 "/home/nicolas/github/CESI-WorldWeatherWatcher/main/main.ino" 3
             ); &__c[0];}))
# 302 "/home/nicolas/github/CESI-WorldWeatherWatcher/main/main.ino"
             ));
      if (gpsLon == TinyGPS::GPS_INVALID_F_ANGLE || gpsLon == 0)
        text += (reinterpret_cast<const __FlashStringHelper *>(
# 304 "/home/nicolas/github/CESI-WorldWeatherWatcher/main/main.ino" 3
               (__extension__({static const char __c[] __attribute__((__progmem__)) = (
# 304 "/home/nicolas/github/CESI-WorldWeatherWatcher/main/main.ino"
               "NA"
# 304 "/home/nicolas/github/CESI-WorldWeatherWatcher/main/main.ino" 3
               ); &__c[0];}))
# 304 "/home/nicolas/github/CESI-WorldWeatherWatcher/main/main.ino"
               ));
      else
        text += gpsLon;
      text += (reinterpret_cast<const __FlashStringHelper *>(
# 307 "/home/nicolas/github/CESI-WorldWeatherWatcher/main/main.ino" 3
             (__extension__({static const char __c[] __attribute__((__progmem__)) = (
# 307 "/home/nicolas/github/CESI-WorldWeatherWatcher/main/main.ino"
             "  "
# 307 "/home/nicolas/github/CESI-WorldWeatherWatcher/main/main.ino" 3
             ); &__c[0];}))
# 307 "/home/nicolas/github/CESI-WorldWeatherWatcher/main/main.ino"
             ));
      text += (reinterpret_cast<const __FlashStringHelper *>(
# 308 "/home/nicolas/github/CESI-WorldWeatherWatcher/main/main.ino" 3
             (__extension__({static const char __c[] __attribute__((__progmem__)) = (
# 308 "/home/nicolas/github/CESI-WorldWeatherWatcher/main/main.ino"
             "Altitude (m) : "
# 308 "/home/nicolas/github/CESI-WorldWeatherWatcher/main/main.ino" 3
             ); &__c[0];}))
# 308 "/home/nicolas/github/CESI-WorldWeatherWatcher/main/main.ino"
             ));
      if (GPS.altitude() == TinyGPS::GPS_INVALID_ALTITUDE)
        text += (reinterpret_cast<const __FlashStringHelper *>(
# 310 "/home/nicolas/github/CESI-WorldWeatherWatcher/main/main.ino" 3
               (__extension__({static const char __c[] __attribute__((__progmem__)) = (
# 310 "/home/nicolas/github/CESI-WorldWeatherWatcher/main/main.ino"
               "NA"
# 310 "/home/nicolas/github/CESI-WorldWeatherWatcher/main/main.ino" 3
               ); &__c[0];}))
# 310 "/home/nicolas/github/CESI-WorldWeatherWatcher/main/main.ino"
               ));
      else {
        float altitude = GPS.altitude() / 100;
        text += altitude;
      }
      text += (reinterpret_cast<const __FlashStringHelper *>(
# 315 "/home/nicolas/github/CESI-WorldWeatherWatcher/main/main.ino" 3
             (__extension__({static const char __c[] __attribute__((__progmem__)) = (
# 315 "/home/nicolas/github/CESI-WorldWeatherWatcher/main/main.ino"
             "  "
# 315 "/home/nicolas/github/CESI-WorldWeatherWatcher/main/main.ino" 3
             ); &__c[0];}))
# 315 "/home/nicolas/github/CESI-WorldWeatherWatcher/main/main.ino"
             ));
      text += (reinterpret_cast<const __FlashStringHelper *>(
# 316 "/home/nicolas/github/CESI-WorldWeatherWatcher/main/main.ino" 3
             (__extension__({static const char __c[] __attribute__((__progmem__)) = (
# 316 "/home/nicolas/github/CESI-WorldWeatherWatcher/main/main.ino"
             "Satelites : "
# 316 "/home/nicolas/github/CESI-WorldWeatherWatcher/main/main.ino" 3
             ); &__c[0];}))
# 316 "/home/nicolas/github/CESI-WorldWeatherWatcher/main/main.ino"
             ));
      if (GPS.satellites() == TinyGPS::GPS_INVALID_SATELLITES)
        text += (reinterpret_cast<const __FlashStringHelper *>(
# 318 "/home/nicolas/github/CESI-WorldWeatherWatcher/main/main.ino" 3
               (__extension__({static const char __c[] __attribute__((__progmem__)) = (
# 318 "/home/nicolas/github/CESI-WorldWeatherWatcher/main/main.ino"
               "NA"
# 318 "/home/nicolas/github/CESI-WorldWeatherWatcher/main/main.ino" 3
               ); &__c[0];}))
# 318 "/home/nicolas/github/CESI-WorldWeatherWatcher/main/main.ino"
               ));
      else
        text += GPS.satellites();

      if (sd) {
        //write in SD card
        SDWriteError = card.writeTextInSD(text);

      }
      Serial.println(text);
    }
  }
}

unsigned long lastSensorCheck(0);
byte errorCode(0);
bool showError = false;
void loop()
{
  checkPressedButton();
  if (mode != 3) {
    int timeCheck = (60 * config.getValue((reinterpret_cast<const __FlashStringHelper *>(
# 339 "/home/nicolas/github/CESI-WorldWeatherWatcher/main/main.ino" 3
                                         (__extension__({static const char __c[] __attribute__((__progmem__)) = (
# 339 "/home/nicolas/github/CESI-WorldWeatherWatcher/main/main.ino"
                                         "LOG_INTERVAL"
# 339 "/home/nicolas/github/CESI-WorldWeatherWatcher/main/main.ino" 3
                                         ); &__c[0];}))
# 339 "/home/nicolas/github/CESI-WorldWeatherWatcher/main/main.ino"
                                         ))) / (3 + 2) * ((mode == 1) ? 2 : 1 ));
    if ((millis() - lastSensorCheck) / 1000 > timeCheck ) {
      lastSensorCheck = millis();
      errorCode = getSensorValues();
      if (!errorCode) {
        lastSuccess == millis();
        showError = false;
      }
    }
    if ((millis() - lastSuccess) / 1000 > config.getValue((reinterpret_cast<const __FlashStringHelper *>(
# 348 "/home/nicolas/github/CESI-WorldWeatherWatcher/main/main.ino" 3
                                                         (__extension__({static const char __c[] __attribute__((__progmem__)) = (
# 348 "/home/nicolas/github/CESI-WorldWeatherWatcher/main/main.ino"
                                                         "TIMEOUT"
# 348 "/home/nicolas/github/CESI-WorldWeatherWatcher/main/main.ino" 3
                                                         ); &__c[0];}))
# 348 "/home/nicolas/github/CESI-WorldWeatherWatcher/main/main.ino"
                                                         ))) + timeCheck) {
      showError = true;
      switch (errorCode) {
        case 1:
          //rtc error
          leds.color((reinterpret_cast<const __FlashStringHelper *>(
# 353 "/home/nicolas/github/CESI-WorldWeatherWatcher/main/main.ino" 3
                    (__extension__({static const char __c[] __attribute__((__progmem__)) = (
# 353 "/home/nicolas/github/CESI-WorldWeatherWatcher/main/main.ino"
                    "RED"
# 353 "/home/nicolas/github/CESI-WorldWeatherWatcher/main/main.ino" 3
                    ); &__c[0];}))
# 353 "/home/nicolas/github/CESI-WorldWeatherWatcher/main/main.ino"
                    )), 1, (reinterpret_cast<const __FlashStringHelper *>(
# 353 "/home/nicolas/github/CESI-WorldWeatherWatcher/main/main.ino" 3
                                 (__extension__({static const char __c[] __attribute__((__progmem__)) = (
# 353 "/home/nicolas/github/CESI-WorldWeatherWatcher/main/main.ino"
                                 "BLUE"
# 353 "/home/nicolas/github/CESI-WorldWeatherWatcher/main/main.ino" 3
                                 ); &__c[0];}))
# 353 "/home/nicolas/github/CESI-WorldWeatherWatcher/main/main.ino"
                                 )), 1);
          break;
        case 2:
          //data error
          leds.color((reinterpret_cast<const __FlashStringHelper *>(
# 357 "/home/nicolas/github/CESI-WorldWeatherWatcher/main/main.ino" 3
                    (__extension__({static const char __c[] __attribute__((__progmem__)) = (
# 357 "/home/nicolas/github/CESI-WorldWeatherWatcher/main/main.ino"
                    "RED"
# 357 "/home/nicolas/github/CESI-WorldWeatherWatcher/main/main.ino" 3
                    ); &__c[0];}))
# 357 "/home/nicolas/github/CESI-WorldWeatherWatcher/main/main.ino"
                    )), 1, (reinterpret_cast<const __FlashStringHelper *>(
# 357 "/home/nicolas/github/CESI-WorldWeatherWatcher/main/main.ino" 3
                                 (__extension__({static const char __c[] __attribute__((__progmem__)) = (
# 357 "/home/nicolas/github/CESI-WorldWeatherWatcher/main/main.ino"
                                 "GREEN"
# 357 "/home/nicolas/github/CESI-WorldWeatherWatcher/main/main.ino" 3
                                 ); &__c[0];}))
# 357 "/home/nicolas/github/CESI-WorldWeatherWatcher/main/main.ino"
                                 )), 2);
          break;
        case 3:
          //sensor error
          leds.color((reinterpret_cast<const __FlashStringHelper *>(
# 361 "/home/nicolas/github/CESI-WorldWeatherWatcher/main/main.ino" 3
                    (__extension__({static const char __c[] __attribute__((__progmem__)) = (
# 361 "/home/nicolas/github/CESI-WorldWeatherWatcher/main/main.ino"
                    "RED"
# 361 "/home/nicolas/github/CESI-WorldWeatherWatcher/main/main.ino" 3
                    ); &__c[0];}))
# 361 "/home/nicolas/github/CESI-WorldWeatherWatcher/main/main.ino"
                    )), 1, (reinterpret_cast<const __FlashStringHelper *>(
# 361 "/home/nicolas/github/CESI-WorldWeatherWatcher/main/main.ino" 3
                                 (__extension__({static const char __c[] __attribute__((__progmem__)) = (
# 361 "/home/nicolas/github/CESI-WorldWeatherWatcher/main/main.ino"
                                 "GREEN"
# 361 "/home/nicolas/github/CESI-WorldWeatherWatcher/main/main.ino" 3
                                 ); &__c[0];}))
# 361 "/home/nicolas/github/CESI-WorldWeatherWatcher/main/main.ino"
                                 )), 1);
          break;
        case 4:
          //gps error
          leds.color((reinterpret_cast<const __FlashStringHelper *>(
# 365 "/home/nicolas/github/CESI-WorldWeatherWatcher/main/main.ino" 3
                    (__extension__({static const char __c[] __attribute__((__progmem__)) = (
# 365 "/home/nicolas/github/CESI-WorldWeatherWatcher/main/main.ino"
                    "RED"
# 365 "/home/nicolas/github/CESI-WorldWeatherWatcher/main/main.ino" 3
                    ); &__c[0];}))
# 365 "/home/nicolas/github/CESI-WorldWeatherWatcher/main/main.ino"
                    )), 1, (reinterpret_cast<const __FlashStringHelper *>(
# 365 "/home/nicolas/github/CESI-WorldWeatherWatcher/main/main.ino" 3
                                 (__extension__({static const char __c[] __attribute__((__progmem__)) = (
# 365 "/home/nicolas/github/CESI-WorldWeatherWatcher/main/main.ino"
                                 "YELLOW"
# 365 "/home/nicolas/github/CESI-WorldWeatherWatcher/main/main.ino" 3
                                 ); &__c[0];}))
# 365 "/home/nicolas/github/CESI-WorldWeatherWatcher/main/main.ino"
                                 )), 1);
          break;
        case 5:
          //SD card write error
          leds.color((reinterpret_cast<const __FlashStringHelper *>(
# 369 "/home/nicolas/github/CESI-WorldWeatherWatcher/main/main.ino" 3
                    (__extension__({static const char __c[] __attribute__((__progmem__)) = (
# 369 "/home/nicolas/github/CESI-WorldWeatherWatcher/main/main.ino"
                    "RED"
# 369 "/home/nicolas/github/CESI-WorldWeatherWatcher/main/main.ino" 3
                    ); &__c[0];}))
# 369 "/home/nicolas/github/CESI-WorldWeatherWatcher/main/main.ino"
                    )), 1, (reinterpret_cast<const __FlashStringHelper *>(
# 369 "/home/nicolas/github/CESI-WorldWeatherWatcher/main/main.ino" 3
                                 (__extension__({static const char __c[] __attribute__((__progmem__)) = (
# 369 "/home/nicolas/github/CESI-WorldWeatherWatcher/main/main.ino"
                                 "WHITE"
# 369 "/home/nicolas/github/CESI-WorldWeatherWatcher/main/main.ino" 3
                                 ); &__c[0];}))
# 369 "/home/nicolas/github/CESI-WorldWeatherWatcher/main/main.ino"
                                 )), 2);
          break;
        case 6:
          //SD card full
          leds.color((reinterpret_cast<const __FlashStringHelper *>(
# 373 "/home/nicolas/github/CESI-WorldWeatherWatcher/main/main.ino" 3
                    (__extension__({static const char __c[] __attribute__((__progmem__)) = (
# 373 "/home/nicolas/github/CESI-WorldWeatherWatcher/main/main.ino"
                    "RED"
# 373 "/home/nicolas/github/CESI-WorldWeatherWatcher/main/main.ino" 3
                    ); &__c[0];}))
# 373 "/home/nicolas/github/CESI-WorldWeatherWatcher/main/main.ino"
                    )), 1, (reinterpret_cast<const __FlashStringHelper *>(
# 373 "/home/nicolas/github/CESI-WorldWeatherWatcher/main/main.ino" 3
                                 (__extension__({static const char __c[] __attribute__((__progmem__)) = (
# 373 "/home/nicolas/github/CESI-WorldWeatherWatcher/main/main.ino"
                                 "WHITE"
# 373 "/home/nicolas/github/CESI-WorldWeatherWatcher/main/main.ino" 3
                                 ); &__c[0];}))
# 373 "/home/nicolas/github/CESI-WorldWeatherWatcher/main/main.ino"
                                 )), 1);
          break;
      }
    }
    if (mode == 0) {
      if (!showError)
        leds.color((reinterpret_cast<const __FlashStringHelper *>(
# 379 "/home/nicolas/github/CESI-WorldWeatherWatcher/main/main.ino" 3
                  (__extension__({static const char __c[] __attribute__((__progmem__)) = (
# 379 "/home/nicolas/github/CESI-WorldWeatherWatcher/main/main.ino"
                  "GREEN"
# 379 "/home/nicolas/github/CESI-WorldWeatherWatcher/main/main.ino" 3
                  ); &__c[0];}))
# 379 "/home/nicolas/github/CESI-WorldWeatherWatcher/main/main.ino"
                  )));
      //true = write in SD card
      writeValues(true);
    }
    else if (mode == 1) {
      if (!showError)
        leds.color((reinterpret_cast<const __FlashStringHelper *>(
# 385 "/home/nicolas/github/CESI-WorldWeatherWatcher/main/main.ino" 3
                  (__extension__({static const char __c[] __attribute__((__progmem__)) = (
# 385 "/home/nicolas/github/CESI-WorldWeatherWatcher/main/main.ino"
                  "BLUE"
# 385 "/home/nicolas/github/CESI-WorldWeatherWatcher/main/main.ino" 3
                  ); &__c[0];}))
# 385 "/home/nicolas/github/CESI-WorldWeatherWatcher/main/main.ino"
                  )));
      //true = write in SD card
      writeValues(true);
    }
    else if (mode == 2) {
      if (!showError)
        leds.color((reinterpret_cast<const __FlashStringHelper *>(
# 391 "/home/nicolas/github/CESI-WorldWeatherWatcher/main/main.ino" 3
                  (__extension__({static const char __c[] __attribute__((__progmem__)) = (
# 391 "/home/nicolas/github/CESI-WorldWeatherWatcher/main/main.ino"
                  "ORANGE"
# 391 "/home/nicolas/github/CESI-WorldWeatherWatcher/main/main.ino" 3
                  ); &__c[0];}))
# 391 "/home/nicolas/github/CESI-WorldWeatherWatcher/main/main.ino"
                  )));
      //true = write in SD card
      writeValues(false);
    }
  }
  else {
    leds.color((reinterpret_cast<const __FlashStringHelper *>(
# 397 "/home/nicolas/github/CESI-WorldWeatherWatcher/main/main.ino" 3
              (__extension__({static const char __c[] __attribute__((__progmem__)) = (
# 397 "/home/nicolas/github/CESI-WorldWeatherWatcher/main/main.ino"
              "YELLOW"
# 397 "/home/nicolas/github/CESI-WorldWeatherWatcher/main/main.ino" 3
              ); &__c[0];}))
# 397 "/home/nicolas/github/CESI-WorldWeatherWatcher/main/main.ino"
              )));
    unsigned long lastActivity = config.getLastActivity();
    //go to normal if inactivity > 30m
    if ((millis() - lastActivity) / 1000 > (30 * 60)) {
      changeMode(0);
    }
    config.waitValues();
  }
}

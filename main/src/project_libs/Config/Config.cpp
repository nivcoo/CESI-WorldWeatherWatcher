#include "Config.h"

Configuration conf[] {
  {0, "VERSION", 0, 20, 1},
  {1, "LUMINO", 0, 1, 1},
  {255, "LUMIN_LOW", 0, 1023, 2},
  {768, "LUMIN_HIGH", 0, 1023, 2},
  {1, "TEMP_AIR", 0, 1, 1},
  { -10, "MIN_TEMP_AIR", -40, 85, 2},
  {60, "MAX_TEMP_AIR", -40, 85, 2},
  {1, "HYGR", 0, 1, 1},
  {0, "HYGR_MINT", -40, 85, 2},
  {50, "HYGR_MAXT", -40, 85, 2},
  {1, "PRESSURE", 0, 1, 1},
  {850, "PRESSURE_MIN", 300, 1100, 2},
  {1080, "PRESSURE_MAX", 300, 1080, 2},
  {10, "LOG_INTERVAL", 1, 255, 2},
  {4096, "FILE_MAX_SIZE", 1024, 8192, 2},
  {30, "TIMEOUT", 1, 255, 2}
};

RTC_DS1307 time;

long lastActivity = 0;

Config::Config(byte version, String batchNumber) :
	_batchNumber(batchNumber)
{
	byte versionIndex = get("VERSION", 1);
	conf[versionIndex].value = version;
	if(version != getValue("VERSION")) 
		resetValues();
}

void Config::resetValues()
{
	int index = 0;
	for (int i = 0; i < sizeof(conf) / sizeof(Configuration); ++i) {
		writeInt(index, conf[i].value, conf[i].size);
		index += conf[i].size;
	}
}


String splitString(String data, char separator, int index)
{
  int found = 0;
  int strIndex[] = {0, -1};
  int maxIndex = data.length()-1;

  for(int i=0; i<=maxIndex && found<=index; i++){
    if(data.charAt(i)==separator || i==maxIndex){
        found++;
        strIndex[0] = strIndex[1]+1;
        strIndex[1] = (i == maxIndex) ? i+1 : i;
    }
  }
  return found>index ? data.substring(strIndex[0], strIndex[1]) : "";
}

void Config::getVersion() {
	Serial.print(F("The version of program is : "));
	Serial.println(getValue(F("VERSION")));
	Serial.print(F("The batch number is : "));
	Serial.println(_batchNumber);
}

long Config::getLastActivity() {
	return lastActivity;
}

void Config::waitValues() {
	String data;
	String name;
	int newValue;
	if (Serial.available() > 0) {
		lastActivity = millis();
		data = Serial.readString();
		name = splitString(data, '=', 0);
        name.trim();
        name.toUpperCase();
		if(name == "SHOW" || name == "RESET" || name == "VERSION") {

			if(name == "RESET") {
				resetValues();
			} else if(name == "VERSION") {
				getVersion();
				return;
			}
			showValues();
			return;
		}
		if(name == "CLOCK" || name == "DATE" || name == "DAY") {
		    DateTime now = time.now();
            if(name == "CLOCK") {
                String clock = splitString(data, '=', 1);
                int hour = splitString(clock, ':', 0).toInt();
                int minute = splitString(clock, ':', 1).toInt();
                int second = splitString(clock, ':', 2).toInt();
                if(hour > 23 || hour < 0 || minute > 59 || minute < 0 || second > 59 || second < 0) {
                    Serial.println(F("The correct format is : HOUR{0-23}:MINUTE{0-59}:SECOND{0-59}"));
                    return;
                }
                time.adjust(DateTime(now.year(), now.month(), now.day(), hour, minute, second));
            } else if(name == "DATE") {
                String date = splitString(data, '=', 1);
                int month = splitString(date, ',', 0).toInt();
                int day = splitString(date, ',', 1).toInt();
                int year = splitString(date, ',', 2).toInt();
                if(month > 12 || month < 1 || day > 31 || day < 1 || year > 2099 || year < 2000) {
                    Serial.println(F("The correct format is : MONTH{1-12},DAY{1-31},YEAR{2000-2099}"));
                    return;
                }
                time.adjust(DateTime(year, month, day, now.hour(), now.minute(), now.second()));
            }

            Serial.print(F("The new date is : "));
            now = time.now();
            Serial.print(now.day(), DEC);
            Serial.print(F("/"));
            Serial.print(now.month(), DEC);
            Serial.print(F("/"));
            Serial.print(now.year(), DEC);
            Serial.print(F(" ("));
            switch (now.dayOfTheWeek()) {
                case 0:
                    Serial.print(F("Sunday"));
                    break;
                case 1:
                    Serial.print(F("Monday"));
                    break;
                case 2:
                    Serial.print(F("Tuesday"));
                    break;
                case 3:
                    Serial.print(F("Wednesday"));
                    break;
                case 4:
                    Serial.print(F("Thursday"));
                    break;
                case 5:
                    Serial.print(F("Friday"));
                    break;
                case 6:
                    Serial.print(F("Saturday"));
                    break;
            }
            Serial.print(F(")"));
            Serial.print(F(" "));
            Serial.print(now.hour(), DEC);
            Serial.print(F(":"));
            Serial.print(now.minute(), DEC);
            Serial.print(F(":"));
            Serial.println(now.second(), DEC);
            return;
        }
        newValue = splitString(data, '=', 1).toInt();
		int configIndex = get(name, 1);
		Configuration c = conf[configIndex];
		if(!get(name, 2)) {
			Serial.println(F("This parameter doesn't exist !"));
			return;
		} else if (c.min > newValue || c.max < newValue) {
			Serial.print(F("Place set a number between "));
			Serial.print(c.min);
			Serial.print(F(" and "));
			Serial.println(c.max);
			return;
		} 
		setValue(name, newValue);
		Serial.print(F("New value for "));
		Serial.print(name);
		Serial.print(F(" = "));
		Serial.println(newValue); 
	}
}


int Config::getValue(String name) {
	int index = get(name, 0);
	return readInt(index, conf[get(name, 1)].size);
}



void Config::setValue(String name, int newValue){
	int index = get(name, 0);
	int indexConfig = get(name, 1);
	writeInt(index, newValue, conf[indexConfig].size);
}




int Config::get(String name, int type) {
	
	int result = 0;
	for (int i = 0; i < sizeof(conf) / sizeof(Configuration); ++i) {
		if(type == 1) // get index in config
			result = i;
		if(conf[i].name == name) {
		
			if(type == 2) // check if exist in config
				result = 1;
			break;
		}
		if(type == 0) { // check index in eeprom
			result += conf[i].size;
		}
	}
	return result;
}

void Config::writeInt(int index, int value, int _size) {
	for (int i = 0; i < _size; i++)
	{
		
		if(_size == 1)
			value = (int8_t) value;
		else if(_size == 2)
			value = (int16_t) value;
		EEPROM.update(index + i, ((byte) (value >> 8 * i)));
	}
}

int Config::readInt(int index, int _size) {
	
	int value = 0;	
	
	for (int i = 0; i < _size; i++)
	{
		value |= EEPROM.read(index + i) << (8 * i);
	}
	
	return value;
	
}

void Config::showValues() {
	Serial.println(F("------------------"));
	getVersion();
	for (int i = 0; i < sizeof(conf) / sizeof(Configuration); ++i) {
		//Serial.println(get(conf[i].name) readInt(index, conf[i].size));
		String name = conf[i].name;
		Serial.print(name);
		Serial.print(F(" : "));
		Serial.println(getValue(name));
	}
	
	if (time.begin()) {
	    DateTime now = time.now();
	    String date = F("DATE : ");
	    date += now.month();
	    date += F(",");
	    date += now.day();
	    date += F(",");
	    date += now.year();
	    String clock = F("CLOCK : ");
	    clock += now.hour();
	    clock += F(":");
	    clock += now.minute();
	    clock += F(":");
	    clock += now.second();
	    Serial.println(date);
	    Serial.println(clock);
	} else {
	    Serial.println(F("Please configure the CLOCK with config mode"));
	}
	
	Serial.println(F("------------------"));
}


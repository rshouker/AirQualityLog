#include "Logger.h"
#include "globals.h"

constexpr int col = 10;
constexpr int row = 1;

Logger::Logger()
{
    _isCallibrating = true;
    _isLogging = false;
}

void Logger::begin()
{
    if (!SD.begin(53)) {
        fatal("SD card error");
    }

    char filename[12];

    sprintf(filename,"log%05d.csv", eepromHeader.logNum);
    _file = SD.open(filename, FILE_WRITE);
    if (_file) {} else {
        Serial.println(filename);
        fatal("File Creation Error");
    }

}

void Logger::printNoLog()
{
    lcd.setCursor(col,row);
    
    if (_isCallibrating) {
        unsigned long now = millis();
        if (now >= callibrationTime) {
            _isCallibrating = false;
        } else {
            unsigned long timeLeft = (callibrationTime - now) / 1000; 
            char timeLeftStr[7];
            sprintf(timeLeftStr, "%02d:%02d ", static_cast<int>(timeLeft / 60),static_cast<int>(timeLeft % 60));
            lcd.print(timeLeftStr);
        }
    } else {
        lcd.print("No Log");
    }
}

void Logger::newData(uint16_t CO2, uint16_t VOC)
{
    if (_isLogging) {
        _file.print(eepromHeader.logNum);
        _file.print(", ");
        _file.print((millis() - _startTime)/1000);
        _file.print(", ");
        _file.print(CO2);
        _file.print(", ");
        _file.println(VOC);

        _file.flush();

    } else {
        printNoLog();
    }
}


void Logger::toggle()
{
    _isLogging = !_isLogging;
    if (_isLogging) {
        eepromHeader.logNum++;
        EepromUpdate(logNum);
        _startTime = millis();
        char logNumStr[7];
        sprintf(logNumStr, "%06d", eepromHeader.logNum);
        lcd.setCursor(col,row);
        lcd.print(logNumStr);
    } else {
        printNoLog();
    }
}

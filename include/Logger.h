#ifndef __Logger_h__
#define __Logger_h__
#include <SD.h>


class Logger {
public:
    Logger();
    void begin();
    void newData(uint16_t CO2, uint16_t VOC);
    void toggle(); // switch modes

private:
    void printNoLog();
    bool _isCallibrating;
    bool _isLogging;
    File _file;
    unsigned long _startTime;
};


#endif // __Logger_h__
#ifndef __globals_h__
#define __globals_h__

#include <stdlib.h>
#include <Arduino.h>
#include <EEPROM.h>
#include "Logger.h"
#include "LiquidCrystal.h"
#include "Logger.h"
#include "KeyCommander.h"


//#include <EEPROM.h>



extern char magic[4]; // = {'I','A', 'Q', 'L'};

struct EepromHeader {
  char magic[4];
  bool hasBaseline;
  uint16_t lastBaseline;
  unsigned int logNum;
  char resereved[20];
};

extern EepromHeader eepromHeader;

#define EepromAddr(field) (reinterpret_cast<char *>(&(eepromHeader.field)) - reinterpret_cast<char *>(&eepromHeader))
#define EepromUpdate(field) EEPROM.put(EepromAddr(field),eepromHeader.field)

extern Logger logger;
extern KeyCommander keyCommander;
extern LiquidCrystal lcd;

extern void fatal(const char *errMsg, ...);

constexpr unsigned long callibrationTime = 20L * 60L * 1000L;

#endif //__globals_h__

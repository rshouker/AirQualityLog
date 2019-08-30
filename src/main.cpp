
#include <Arduino.h>
#include <LiquidCrystal.h>

#include <EEPROM.h>
#include "globals.h"
#include <SimpleTimer.h>
#include <stdint.h>
#include "SparkFunBME280.h"
#include "SparkFunCCS811.h"

#include "Wire.h"
#include "SPI.h"
#include "globals.h"

Logger logger;
KeyCommander keyCommander;

// select the pins used on the LCD panel
LiquidCrystal lcd(8, 9, 4, 5, 6, 7);

//Global sensor object
BME280 myBME280;

#define CCS811_ADDR 0x5B //Default I2C Address
//#define CCS811_ADDR 0x5A //Alternate I2C Address

CCS811 myCCS811(CCS811_ADDR);

SimpleTimer timer;

char magic[4] = {'R','A', 'Q', 'L'};
EepromHeader eepromHeader;
bool firstTimer = false;
constexpr float tempDiff = 0.8;

static void sample();

void saveBaseline()
{
  // as a safety procotion if fail in the middle of operation, have the hasBaseline be false
  eepromHeader.hasBaseline = false;
  EepromUpdate(hasBaseline);
  eepromHeader.lastBaseline = myCCS811.getBaseline();
  EepromUpdate(lastBaseline);
  eepromHeader.hasBaseline = true;
  EepromUpdate(hasBaseline);
  Serial.print("saving basline: ");
  Serial.println(eepromHeader.lastBaseline);
}


//bool potMode;

#if 0
void console();
void setup() {
  pinMode(logPin, INPUT_PULLUP);
  pinMode(baselineRestorePin, INPUT_PULLUP);
  pinMode(consolePin, INPUT_PULLUP);
  pinMode(usePotPin, INPUT_PULLUP);
  // check whether EEPROM was ever initialized
  EEPROM.get(0, eepromHeader);
  if (memcmp(eepromHeader.magic, magic, sizeof(magic) != 0)) {
    firstTimer = true;
    memcpy(eepromHeader.magic, magic, sizeof(magic));
    eepromHeader.tempDiff = 15;
    memset(eepromHeader.resereved, 0, sizeof(eepromHeader.resereved));
    EEPROM.put(0, eepromHeader);
  }

  if (digitalRead(consolePin) == LOW) {
    console();
  }

  potMode = (digitalRead(usePotPin) == LOW);
}

void loop() {
  // put your main code here, to run repeatedly:
}

void console() {
  while (true) {};
}
#endif

/*
void printData()
{
  Serial.print(" CO2[");
  Serial.print(myCCS811.getCO2());
  Serial.print("]ppm");

  Serial.print(" TVOC[");
  Serial.print(myCCS811.getTVOC());
  Serial.print("]ppb");

  Serial.print(" temp[");
  Serial.print(myBME280.readTempC(), 1);
  Serial.print("]C");

  //Serial.print(" temp[");
  //Serial.print(myBME280.readTempF(), 1);
  //Serial.print("]F");

  Serial.print(" pressure[");
  Serial.print(myBME280.readFloatPressure(), 2);
  Serial.print("]Pa");

  //Serial.print(" pressure[");
  //Serial.print((myBME280.readFloatPressure() * 0.0002953), 2);
  //Serial.print("]InHg");

  //Serial.print("altitude[");
  //Serial.print(myBME280.readFloatAltitudeMeters(), 2);
  //Serial.print("]m");

  //Serial.print("altitude[");
  //Serial.print(myBME280.readFloatAltitudeFeet(), 2);
  //Serial.print("]ft");

  Serial.print(" humidity[");
  Serial.print(myBME280.readFloatHumidity(), 0);
  Serial.print("]%");

  Serial.println();
}
*/

void printData(uint16_t CO2, uint16_t VOC)
{
  lcd.setCursor(0,0);
  lcd.print("CO2:     ");
  lcd.setCursor(4,0);
  lcd.print(CO2);

  lcd.setCursor(0,1);
  lcd.print("VOC:     ");
  lcd.setCursor(4,1);
  lcd.print(VOC);
}



void printDriverError( CCS811Core::status errorCode )
{
  const char *reason;
  switch ( errorCode )
  {
    case CCS811Core::SENSOR_SUCCESS:
      reason = "SUCCESS";
      break;
    case CCS811Core::SENSOR_ID_ERROR:
      reason = "ID_ERROR";
      break;
    case CCS811Core::SENSOR_I2C_ERROR:
      reason = "I2C_ERROR";
      break;
    case CCS811Core::SENSOR_INTERNAL_ERROR:
      reason = "INTERNAL_ERROR";
      break;
    case CCS811Core::SENSOR_GENERIC_ERROR:
      reason = "GENERIC_ERROR";
      break;
    default:
      reason = "Unspecified";
  }

  fatal ("CCS811 error: %s.", reason);
}


void setup()
{
  Serial.begin(9600);
  lcd.begin(16, 2);              // start the library
  lcd.clear();
  lcd.noAutoscroll();
  // check whether EEPROM was ever initialized
  EEPROM.get(0, eepromHeader);
  if (memcmp(eepromHeader.magic, magic, sizeof(magic) != 0)) {
    firstTimer = true;
    memcpy(eepromHeader.magic, magic, sizeof(magic));
    eepromHeader.logNum = 1000;
    eepromHeader.hasBaseline = false;
    // eepromHeader.tempDiff = 15;
    memset(eepromHeader.resereved, 0, sizeof(eepromHeader.resereved));
    EEPROM.put(0, eepromHeader);
  }
  
    //***Driver settings********************************//
    //commInterface can be I2C_MODE
    //specify I2C address.  Can be 0x77(default) or 0x76
#ifndef I2C_MODE
#define I2C_MODE 0
#endif
    //For I2C, enable the following
    myBME280.settings.commInterface = I2C_MODE;
    myBME280.settings.I2CAddress = 0x77;

    //***Operation settings*****************************//

    //runMode can be:
    //  0, Sleep mode
    //  1 or 2, Forced mode
    //  3, Normal mode
    myBME280.settings.runMode = 3; //Forced mode

    //tStandby can be:
    //  0, 0.5ms
    //  1, 62.5ms
    //  2, 125ms
    //  3, 250ms
    //  4, 500ms
    //  5, 1000ms
    //  6, 10ms
    //  7, 20ms
    myBME280.settings.tStandby = 0;

    //filter can be off or number of FIR coefficients to use:
    //  0, filter off
    //  1, coefficients = 2
    //  2, coefficients = 4
    //  3, coefficients = 8
    //  4, coefficients = 16
    myBME280.settings.filter = 0;

    //tempOverSample can be:
    //  0, skipped
    //  1 through 5, oversampling *1, *2, *4, *8, *16 respectively
    myBME280.settings.tempOverSample = 1;

    //pressOverSample can be:
    //  0, skipped
    //  1 through 5, oversampling *1, *2, *4, *8, *16 respectively
    myBME280.settings.pressOverSample = 1;

    //humidOverSample can be:
    //  0, skipped
    //  1 through 5, oversampling *1, *2, *4, *8, *16 respectively
    myBME280.settings.humidOverSample = 1;
    delay(10);  //Make sure sensor had enough time to turn on. BME280 requires 2ms to start up.         

    //Serial.print("Starting BME280... result of .begin(): 0x");
    //Calling .begin() causes the settings to be loaded
    int status = myBME280.begin();
    if (status != 0x60) {
      fatal("BME280 error :%#x", status);
    }
    myCCS811.begin();

    logger.begin();
    // restore CCS811 baseline

    if (eepromHeader.hasBaseline) {
      Serial.print("restoring baseline: ");
      Serial.println(eepromHeader.lastBaseline);
      myCCS811.setBaseline(eepromHeader.lastBaseline);
    }

    timer.setInterval(1000, sample);
    timer.setInterval(20L*60*1000, saveBaseline);
}

void sample()
{
  //Check to see if data is available
  if (myCCS811.dataAvailable())
  {
    //Calling this function updates the global tVOC and eCO2 variables
    myCCS811.readAlgorithmResults();
    //printData fetches the values of tVOC and eCO2
    uint16_t CO2 = myCCS811.getCO2();
    uint16_t VOC = myCCS811.getTVOC();
    printData(CO2, VOC);
    logger.newData(CO2, VOC);
    float BMEtempC = myBME280.readTempC();
    float BMEhumid = myBME280.readFloatHumidity();

/*
    Serial.print("Applying new values (deg C, %): ");
    Serial.print(BMEtempC);
    Serial.print(",");
    Serial.println(BMEhumid);
    Serial.println();
*/
    //This sends the temperature data to the CCS811
    myCCS811.setEnvironmentalData(BMEhumid, BMEtempC + tempDiff);
  }
  else if (myCCS811.checkForStatusError())
  {
    Serial.println(myCCS811.getErrorRegister()); //Prints whatever CSS811 error flags are detected
  }

}


void loop()
{
  timer.run();
  keyCommander.sample();
}

void fatal(const char *errMsg, ...)
{
  va_list ap;
  char msg[40];  
  const char *secondLine = "";
  char firstLine[17];

  va_start(ap, errMsg);
  vsprintf(msg, errMsg, ap);

  strncpy(firstLine, msg, 16);
  firstLine[16] = '\0';

  if (strlen(msg) > 16) {
    secondLine = msg + 16;
    char *pSpace = strrchr(firstLine, ' ');
    if (pSpace!=0) {
      Serial.println("Space found:");
      Serial.println(pSpace);
      *pSpace = '\0';
      secondLine = msg + (pSpace - firstLine) + 1;
    } else {
      Serial.println("No space found");
    }
  }
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print(firstLine);

  lcd.setCursor(0,1);
  lcd.print(secondLine);

  while(true);  // halt
}

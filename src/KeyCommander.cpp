#include "KeyCommander.h"
#include <Arduino.h>
#include "globals.h"

const KeyCommander::btn KeyCommander::_sequence[] = {btnRIGHT, btnNONE, btnLEFT, btnNONE, btnSELECT, btnNONE};
const size_t KeyCommander::_seqSize = sizeof(KeyCommander::_sequence) / sizeof(KeyCommander::btn);

KeyCommander::KeyCommander()
{
    _isDebouncing = false;
    _isPressed = false;
    _placeInSequence = 0;
}

KeyCommander::btn KeyCommander::readBtn()
{
    int adc_key_in = analogRead(0);      // read the value from the sensor
    // my buttons when read are centered at these valies: 0, 144, 329, 504, 741
    // we add approx 50 to those values and check to see if we are close
    if (adc_key_in > 1000) return btnNONE; // We make this the 1st option for speed reasons since it will be the most likely result
    // For V1.1 us this threshold
    if (adc_key_in < 50)   return btnRIGHT;
    if (adc_key_in < 250)  return btnUP;
    if (adc_key_in < 450)  return btnDOWN;
    if (adc_key_in < 650)  return btnLEFT;
    if (adc_key_in < 850)  return btnSELECT;

    // For V1.0 comment the other threshold and use the one below:
    /*
    if (adc_key_in < 50)   return btnRIGHT;
    if (adc_key_in < 195)  return btnUP;
    if (adc_key_in < 380)  return btnDOWN;
    if (adc_key_in < 555)  return btnLEFT;
    if (adc_key_in < 790)  return btnSELECT;
    */

    return btnNONE;  // when all others fail, return this...
}


void KeyCommander::sample()
{
    if (_isDebouncing) {
        if (millis() >= _debuncingUntil) {
            _isDebouncing = false;
        } else {
            return;
        }
    }
    btn button = readBtn();
    if(_isPressed && button == _sequence[_placeInSequence-1]) {
        return;
    }

    if (button == _sequence[_placeInSequence]) {
        _isPressed = true;
        _placeInSequence++;
        if (_placeInSequence == _seqSize) {
            command();
        } else {
            _isDebouncing = true;
            _debuncingUntil = millis() + 50;
        }
    } else {
        _placeInSequence = 0;
        _isPressed = false;
    }
}


void KeyCommander::command()
{
    logger.toggle();
}


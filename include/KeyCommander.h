#ifndef __KeyCommander_h__
#define __KeyCommander_h__

#include <stdlib.h>

class KeyCommander {
public:
    enum btn {btnRIGHT, btnUP, btnDOWN, btnLEFT, btnSELECT, btnNONE};
    KeyCommander();
    void sample();
    void command();
private:
    static const btn _sequence[];
    static const size_t _seqSize;
    btn readBtn();
    bool _isDebouncing;
    bool _isPressed;
    unsigned long _debuncingUntil;
    int _placeInSequence;
};



#endif //__KeyCommander_h__
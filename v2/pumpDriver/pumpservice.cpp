#include "pumpdriver.h"
#include <Arduino.h>


PumpService::PumpService(long defaultTimer)
{
    _defaultTimer = defaultTimer;
    
    //for(int i=0; i < MAXPUMPS> i++){
    //   _pumps[i] = Pump(-1, _defaultTimer);
    //}
}

void attach_pump(int pin)
{
    pinMode(pin, OUTPUT);
    _pumps.insert(Pump(pin, _defaultTimer));

}

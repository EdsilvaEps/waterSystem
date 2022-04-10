/*
    Pumpservice.h - a library to manage the pumps attached to the watering system 
    Created by Edson Siva, April 10, 2022.

*/

#ifndef Pumpservice_h
#define Pumpservice_h

#define MAXPUMPS 5


#include <Arduino.h>
#include <list>
#include "pump.h"

class PumpService {
    public:
        PumpService(long defaultTimer=0);
        void attach_pump(int pin);
        void detach_pump(int pin);
        void configure_pump(int pin, int timer);
        void execute_pump_action_all();
        void execute_pump_action(int pin);

    private:
        long _defaultTimer;
        std::list<Pump> _pumps;
}


#endif
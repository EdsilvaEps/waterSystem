/*
    Pump.h - a library with functions of a single water pump in the system 
    Created by Edson Siva, April 10, 2022.

*/

#ifndef Pump_h
#define Pump_h

#include <Arduino.h>

class Pump {
    public:
        Pump(int pin, long timer); // constructor, requires physical pin reference and pumping timer value in millisseconds 
        void set_timer(int timer);
        void set_pin(int pin); 
        int get_timer();
        int get_pin();
        void execute_pump_action(); // trigger the actual pumping TODO: requires multithreading

    private:
        int _pin;
        long _timer;

}

#endif
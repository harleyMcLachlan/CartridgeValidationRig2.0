//How to use: put your device name in the enum list. Each additional shift register board will
//add another 8 possible devices. The position in the enum list will determine which letter
//on the shift register it`s output will be mapped to.
//Then use the shiftRegister_*** functions to control it

#ifndef _shiftRegisterHandler_h_
#define _shiftRegisterHandler_h_

#include "_task_cfg.h"
#include "global.h"
#include "Arduino.h"

/// Used to update the shift register to control the relay pack
void shiftRegisterHandler();          // called from scheduler (probably don't change it, it decides what function to run)
void shiftRegisterHandler_init();     // called once when starting (for initializing things, starting timers etc. can be empty)
void shiftRegisterHandler_runnable(); // called every loop (must have no delays)
void shiftRegisterHandler_deinit();   // called once when stopping (to stop pumps or upload data for example, can be empty)

/******** Variables ********/
byte shiftRegister_value_old = 0;
byte shiftRegister_value_new = 0;

enum shiftRegister_enum{        //Letters match to letters on the shift register board
    RELAY_24V_CO2VALVE1_NC,         // H
    RELAY_24V_CO2VALVE2_NC,         // G
    RELAY_12V_SUPPLYPUMP,           // F
    RELAY_12V_WASTEVALVE_NO,        // E
    RELAY_12V_SAMPLEVALVE_NC,       // D
    RELAY_12V_BOTTLEFILLVALVE_NC,   // C
    RELAY_12V_WASTEPUMP,            // B
    RELAY_12V_OUTPUTPUMP,           // A
};
/***************************/

/******** Functions ********/
/// Call to set one bit of the output. Will be updated next program loop
void shiftRegister_turnOn(shiftRegister_enum shiftRegisterEnum);

/// Call to clear one bit of the output. Will be updated next program loop
void shiftRegister_turnOff(shiftRegister_enum shiftRegisterEnum);

/// Call to immediately clear all bits
void shiftRegister_shutDown();

/// Not for user. Call to send the newest bits to the register
void shiftRegister_update();
/***************************/

void shiftRegisterHandler(){

    task_enum i = SHIFTREGISTERHANDLER;

    switch (task_list[i].state)
    {
    case STATE_STARTING:
        shiftRegisterHandler_init();
        task_list[i].hardStart();
        break;
    
    case STATE_RUNNING:
        shiftRegisterHandler_runnable();
        break;
    
    case STATE_STOPPING:
        shiftRegisterHandler_deinit();
        task_list[i].hardStop();
        break;
    } 

    
}

void shiftRegisterHandler_init(){
    pinMode(PIN_SHIFTREG_RCLCK, OUTPUT);
    pinMode(PIN_SHIFTREG_SERIN, OUTPUT);
    pinMode(PIN_SHIFTREG_SRCLK, OUTPUT);
    shiftRegister_shutDown();
    print("Shift Register Pins setup");
    }

void shiftRegisterHandler_runnable(){
    if(shiftRegister_value_new != shiftRegister_value_old){
        shiftRegister_value_old = shiftRegister_value_new;
        // print("Shift Register Update sent");
    }
    shiftRegister_update();
}

void shiftRegisterHandler_deinit(){
    shiftRegister_shutDown();
}

void shiftRegister_update()
{
   digitalWrite(PIN_SHIFTREG_RCLCK, LOW);
   shiftOut(PIN_SHIFTREG_SERIN, PIN_SHIFTREG_SRCLK, LSBFIRST, shiftRegister_value_new);
   digitalWrite(PIN_SHIFTREG_RCLCK, HIGH);
}

void shiftRegister_turnOn(shiftRegister_enum shiftRegisterEnum){
    bitClear(shiftRegister_value_new, shiftRegisterEnum);
}

void shiftRegister_turnOff(shiftRegister_enum shiftRegisterEnum){
    bitSet(shiftRegister_value_new, shiftRegisterEnum);
}

void shiftRegister_shutDown(){
    shiftRegister_value_new = 0b11111111;
    shiftRegister_update();
    print("Shift Register shutdown");
}
#endif


#ifndef _draining_h_
#define _draining_h_

#include "_task_cfg.h"
#include "memory.h" //for using EEPROM
#include "Arduino.h" //for arduino related commands
#include "shiftRegisterHandler.h" //for controlling the relay output
// #include "switch_cfg.h" //for accessing switch states


/// Drains water from the bottle
void draining();
void draining_init();
void draining_runnable();
void draining_deinit();

/******** Functions ********/

/***************************/

/******** Variables ********/
unsigned long draining_timeStarted;
uint16_t draining_drainLength = 10000;
/***************************/

void draining_init(){
    print("Starting draining!");
    draining_timeStarted = millis();
    draining_drainLength = memoryRead(draining_EEPROM_drainLength) * 1000; //get ms of draining time from memory from UI
    shiftRegister_turnOff(RELAY_12V_WASTEVALVE_NO); //make sure waste valve open to let water into waste
    shiftRegister_turnOff(RELAY_12V_BOTTLEFILLVALVE_NC); //make sure bottlefill valve closed to suck from bottle
    shiftRegister_turnOn(RELAY_12V_OUTPUTPUMP); //turn on ouput pump
    task_list[DRAINING].allowWifiReconnect = false;
}

void draining_runnable(){
    //Check if ready to stop
    if(millis() - draining_timeStarted > draining_drainLength){
        task_list[DRAINING].stop();
    }
    //open sample valve briefly to clear sample lines
    if(millis() - draining_timeStarted > draining_drainLength-2000){
        shiftRegister_turnOn(RELAY_12V_SAMPLEVALVE_NC);
        shiftRegister_turnOn(RELAY_12V_WASTEPUMP);
    }
    //be able to stop from UI
    if(switch_list[SWITCH_DRAINING].switched() && !switch_list[SWITCH_DRAINING].pushed()){
        task_list[DRAINING].stop();
    }
}

void draining_deinit(){
    task_list[DRAINING].allowWifiReconnect = true;
    print("Stopping draining!");
    shiftRegister_turnOff(RELAY_12V_WASTEPUMP);
    shiftRegister_turnOff(RELAY_12V_SAMPLEVALVE_NC);
    shiftRegister_turnOff(RELAY_12V_OUTPUTPUMP); //turn off ouput pump
}

/**********************************/

void draining(){

    task_enum i = DRAINING; //draining //put your task enum from task_cfg.h here

    switch (task_list[i].state)
    {
    case STATE_STARTING:
        draining_init();
        task_list[i].hardStart();
        break;
    
    case STATE_RUNNING:
        draining_runnable();
        break;
    
    case STATE_STOPPING:
        draining_deinit();
        task_list[i].hardStop();
        break;
    } 
}

#endif


// Use this as a template for making tasks
// REPLACE "EXAMPLETASK" with your task enum name in _task_cfg.h
// Replace "IDLE" with your task enum name in _task_cfg.h
// RELACE ALL "exampleTask" with your task name

#ifndef _exampleTask_h_
#define _exampleTask_h_

#include "_task_cfg.h"
#include "memory.h" //for using EEPROM
// #include "global.h" //for pin names and other global things
#include "Arduino.h" //for arduino related commands
// #include "shiftRegisterHandler.h" //for controlling the relay output
// #include "switch_cfg.h" //for accessing switch states
// #include "data" //for uploading to grafana

/// * Describe the task here *
/// Used as a "do nothing" task, for switches and buttons to call
void exampleTask();          // called from scheduler (probably don't change it, it decides what function to run)
void exampleTask_init();     // called once when starting (for initializing things, starting timers etc. can be empty)
void exampleTask_runnable(); // called every loop (must have no delays)
void exampleTask_deinit();   // called once when stopping (to stop pumps or upload the collected data for example, can be empty)

/******** Functions ********/
void exampleTask_myFunction(); // example of your minor function
/***************************/

/******** Variables ********/

/***************************/

void exampleTask_init(){
    //*do stuff once when starting
}

void exampleTask_runnable(){
    //*do stuff every program cycle
}

void exampleTask_deinit(){
    //*do stuff once when stopping
}

void exampleTask_myFunction(){
    // *do minor function stuff here
}

/**********************************/

void exampleTask(){

    task_enum i = IDLE; //EXAMPLETASK //put your task enum from task_cfg.h here

    switch (task_list[i].state)
    {
    case STATE_STARTING:
        exampleTask_init();
        task_list[i].hardStart();
        break;
    
    case STATE_RUNNING:
        exampleTask_runnable();
        break;
    
    case STATE_STOPPING:
        exampleTask_deinit();
        task_list[i].hardStop();
        break;
    } 
}

#endif


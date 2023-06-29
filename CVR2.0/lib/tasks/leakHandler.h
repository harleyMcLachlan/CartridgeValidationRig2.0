//Leak handler is triggered by a switch (leak sensor)

#ifndef _leakHandler_h_
#define _leakHandler_h_

#include "_task_cfg.h"
#include "Arduino.h" //for arduino related commands
#include "switch_cfg.h" //for accessing switch states

void leakHandler();          // called from scheduler (probably don't change it, it decides what function to run)
void leakHandler_init();     // called once when starting (for initializing things, starting timers etc. can be empty)
void leakHandler_runnable(); // called every loop (must have no delays)
void leakHandler_deinit();   // called once when stopping (to stop pumps or upload the collected data for example, can be empty)

/******** Functions ********/
/***************************/

/******** Variables ********/
const uint16_t leakHandler_DEBOUNCETIME = 100;
uint64_t leakHandler_bounceTimer = 0;
uint64_t leakHandler_clearTimer = 0;
/***************************/

void leakHandler_init(){
  //Leak maybe detected at this point
  print("LEAK DETECTED? ",false);
  leakHandler_clearTimer = millis();
  leakHandler_bounceTimer = millis();  
}

void leakHandler_runnable(){
  //double check if leak is real with debounce timer
  if(millis() - leakHandler_bounceTimer > leakHandler_DEBOUNCETIME && millis() - leakHandler_bounceTimer < leakHandler_DEBOUNCETIME + 5){
    if(switch_list[SWITCH_leakHandler].on()){
      errorState(LEAK); 
      print("YES");
      leakHandler_bounceTimer = 0;
    } 
    else{
      task_list[LEAKHANDLER].stop();
      print("NO");
    }
  }

  //if pause switched flipped twice fast, unpause (clears error)
  if(switch_list[SWITCH_PAUSE].switched()){ 
    if(millis() - leakHandler_clearTimer < 500 && millis() - leakHandler_clearTimer > 1){
      task_list[LEAKHANDLER].stop();
    }
    else{
      leakHandler_clearTimer = millis();
    }
  }
}

void leakHandler_deinit(){
    print("Resuming");
    errorState(NOT_RUNNING);
}

/**********************************/

void leakHandler(){

    task_enum i = LEAKHANDLER; //leakHandler //put your task enum from task_cfg.h here

    switch (task_list[i].state)
    {
    case STATE_STARTING:
        leakHandler_init();
        task_list[i].hardStart();
        break;
    
    case STATE_RUNNING:
        leakHandler_runnable();
        break;
    
    case STATE_STOPPING:
        leakHandler_deinit();
        task_list[i].hardStop();
        break;
    } 
}

#endif


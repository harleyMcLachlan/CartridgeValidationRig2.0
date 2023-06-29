#ifndef _idle_h_
#define _idle_h_

#include "_task_cfg.h"
#include "Arduino.h" 
#include "switch_cfg.h"
#include "global.h"

void idle();          // called from scheduler (probably don't change it, it decides what function to run)
void idle_init();     // called once when starting (for initializing things, starting timers etc. can be empty)
void idle_runnable(); // called every loop (must have no delays)
void idle_deinit();   // called once when stopping (to stop pumps or upload the collected data for example, can be empty)

/******** Functions ********/

/***************************/

/******** Variables ********/
uint64_t idleTask_time_lastPrint = 0; //exmple of global variable that your functions use
unsigned long idleTask_pauseDebounceTimer = 0;
bool idleTask_pauseLastState = false;
/***************************/

void idle_init(){
    idleTask_pauseLastState = switch_list[SWITCH_PAUSE].on(); //get pause switch position
}

void idle_runnable(){
    //Print running every couple seconds
    if(millis() - idleTask_time_lastPrint > 5000){
        print("\nRunning...");
        idleTask_time_lastPrint = millis();
        if(paused){
            uploadToGrafana("status",PAUSED);} //update grafana pause
        else{
            uploadToGrafana("status",errorState());}
            
    }
    
    


    //If pause switch is flicked twice fast it clears errors
    if(switch_list[SWITCH_PAUSE].switched()){ 
        if(millis() - leakHandler_clearTimer < 500 && millis() - leakHandler_clearTimer > 1){
            errorState(NOT_RUNNING);
        }
        else{
            leakHandler_clearTimer = millis();
        }
    }

//Pause switch handling
    if(switch_list[SWITCH_PAUSE].switched()){ //get pause switch time
        idleTask_pauseDebounceTimer = millis();
    }
    //debouncing pause switch
    //after 20ms of pause switch check to see if the switch state really changed
    if(millis() - idleTask_pauseDebounceTimer > 200 && millis() - idleTask_pauseDebounceTimer < 300){
        if(switch_list[SWITCH_PAUSE].on() && idleTask_pauseLastState == false){
            paused = true;
            idleTask_pauseLastState = true;
            idleTask_pauseDebounceTimer = 0;
        }
        else if(!switch_list[SWITCH_PAUSE].on() && idleTask_pauseLastState == true){
            paused = false;
            idleTask_pauseLastState = false;
            idleTask_pauseDebounceTimer = 0;
        }
    }

    //Buzzer when there is error
    if(errorState() != RUNNING && errorState() != SAMPLERERROR && errorState() != NOT_RUNNING){
        if(millis() - timeErrorOccured < 750){
            analogWrite(PIN_BUZZER, 255);
        }
        else
        {
            analogWrite(PIN_BUZZER, 0);
        }
    }
    else
    {
        analogWrite(PIN_BUZZER, 0);
    }
    
}

void idle_deinit(){
    //*do stuff once when stopping
}


/**********************************/

void idle(){

    task_enum i = IDLE;

    switch (task_list[i].state)
    {
    case STATE_STARTING:
        idle_init();
        task_list[i].hardStart();
        break;
    
    case STATE_RUNNING:
        idle_runnable();
        break;
    
    case STATE_STOPPING:
        idle_deinit();
        task_list[i].hardStop();
        break;
    } 
}

#endif


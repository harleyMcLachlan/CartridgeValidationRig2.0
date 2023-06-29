//how to use: put your task name in the enum list in the task cfg.h and then instantiate the task
// in the task_list here using the example

#include "btnHandler.h"
#include "_task_cfg.h"
#include <Arduino.h>

//INCLUDE ALL YOUR TASKS
#include "fillITK.h"
#include "wifi.h"
#include "idle.h"
#include "shiftRegisterHandler.h"
#include "priming.h"
#include "leakHandler.h"
#include "draining.h"
#include "sampling.h"
#include "sensors.h"
#include "global.h"

errorState_enum _errorState = NOT_RUNNING;

///Use to read error status
errorState_enum errorState(){return _errorState;}

///Used to set error status and will also send status to server. Will auto start draining task also to make sure the bottle is empty, before the user starts the machine again
//Setting an error will cause the machine to stop and need to be started again after clearing the error.
//Clear the error by setting the error state to not running or running.
//Add error states in the _task_cfg.h
errorState_enum errorState(errorState_enum state){
    timeErrorOccured = millis();
    _errorState = state;
    wifi_publish("/CVR2_state_to_server", String(errorState()));
    uploadToGrafana("status",_errorState);
    digitalWrite(PIN_LED, LOW);
    if(state == TOOFAST ||
        state == TOOSLOW ||
        state == HIGHPRESSURE ||
        state == LOWCO2PRESSURE ||
        state == FAILTOFILL){

            task_list[DRAINING].start();

    }
    return _errorState;
}

///returns 1 if there is an error, else 0. 
bool isError(){
    if(errorState() != RUNNING && errorState() != NOT_RUNNING){
        return 1;
    }
    else{
        return 0;
    }
}
/**********************************************/

Task task_list[MAX_TASKS] = {
    /*
    {   ** EXAMPLE **
        FUNCTION NAME,
        Default state = stopped
        looping speed = 0
        starting timer = 0
    },
    */
    {
        priming,
        STATE_STOPPED,
        100,
    },

    {
        fillITK,
        STATE_STOPPED,
        100,
    },

    {
        leakHandler,
        STATE_STOPPED,
    },

    {
        sampling,
        STATE_STOPPED,
        100,
    },

    {
        draining,
        STATE_STOPPED,
    },

    {
        sensors,
        STATE_STOPPED,
    },
    
    ///// System Tasks ////
    {
        btnHandler,
        STATE_STARTING,
    },

    {
        shiftRegisterHandler,
        STATE_STARTING,
        0,
    },

    {   
        idle,
        STATE_STARTING,
        0,
    },

    {
        wifi,
        STATE_STARTING,
        1000,
    },
};

///Constructor
Task::Task(void (*_fptr)(), int _state, unsigned long _loop_ms, unsigned long timer){
    fptr = _fptr;
    state = _state;
    loop_ms = _loop_ms;
    timer = 0;

}

///Sets the task to STATE_STARTING to be run next loop.
void Task::start(){
    if(this->state == STATE_STOPPED || this->state == STATE_STOPPING){
        this->state = STATE_STARTING;
    }
}

void Task::hardStart(){
    this->state = STATE_RUNNING;
}

///Sets the task to STATE_STOPPING to deinit the next loop. Then stop the next loop.
void Task::stop(){
    if(this->state != STATE_STOPPED){
        this->state = STATE_STOPPING;
        this->timer = this->loop_ms;
    }
}

void Task::hardStop(){
    this->state = STATE_STOPPED;
}

///Returns if the task is active by checking state is lower than "waiting"
bool Task::active(){
    if(this->state <= STATE_WAITING)
        return true;
    else
        return false;
}
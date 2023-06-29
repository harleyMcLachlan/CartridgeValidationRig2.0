#ifndef _fillitk_h_
#define _fillitk_h_

#include <Arduino.h>
#include "_task_cfg.h"
#include "shiftRegisterHandler.h"
#include "switch_cfg.h"
#include "memory.h"
#include <EEPROM.h>
#include "data.h"

///Fills the ITK when possible
void fillITK();
void fillITK_runnable();
void fillITK_init();
void fillITK_deinit();

/******** Functions ********/
bool fillITK_failToFill();

/******** Variables ********/
unsigned long fillITK_time_startFillingITK; //Time the itk started filling to keep track of pump health and error
unsigned long fillITK_time_lastWaterSensed; //The last time that the capacitive water sensor sensed water

int fillITK_stagnationTime = 0;

/******* DATA **************/
int fillITK_timeTookToFill;
/***************************/

void fillITK_init(){
    digitalWrite(PIN_LED,HIGH);
    if(!isError() && errorState() != RUNNING) errorState(RUNNING); //Refresh state to running, if there is no error. (upates the GUI running status)
    if(!paused){
        print("Starting fillITK");
        fillITK_time_lastWaterSensed = millis();
        fillITK_time_startFillingITK = millis();

        if(!switch_list[SWITCH_ITK_HIGH].on()){ //Turn on pump if tank is not full
            if(!switch_list[SWITCH_ITK_LOW].on()){ //reset pumping timer if the tank was full empty. To keep track of supply pump health
                fillITK_time_startFillingITK = millis();
            }
            task_list[FILLITK].allowWifiReconnect = false;
            shiftRegister_turnOn(RELAY_12V_SUPPLYPUMP);
        }
        task_list[FILLITK].hardStart();
    }
}


void fillITK_runnable(){
    if(!isError()){ //Stop if theres an error
        //Check if failed to fill
        if(fillITK_failToFill()){
            errorState(FAILTOFILL);
            print("ITK Fail to fill");
            task_list[FILLITK].stop();
        }

        //check if Finished filling
        if(switch_list[SWITCH_ITK_HIGH].on()){
            task_list[FILLITK].stop();
            print("ITK Filled!");
            if(fillITK_time_startFillingITK){ //gather time took to fill, if it started empty to keep track of supply pump health
                fillITK_timeTookToFill = (millis() - fillITK_time_startFillingITK)/1000; //seconds
                uploadToGrafana("timeTookToFill", fillITK_timeTookToFill);}
            task_list[PRIMING].start();
        }
    }
    else task_list[FILLITK].stop();
    
}

void fillITK_deinit(){
    print("Stopping fillITK");
    task_list[FILLITK].allowWifiReconnect = true;
    shiftRegister_turnOff(RELAY_12V_SUPPLYPUMP);
}

bool fillITK_failToFill(){
    if(!switch_list[SWITCH_CAPACITIVEWATERSENSOR].on() || !switch_list[SWITCH_CAPACITIVEWATERSENSOR].ticking_state){ //Update lastWaterSensed when there is water or the sensor is disabled
        fillITK_time_lastWaterSensed = millis();
    }
    if(millis() - fillITK_time_lastWaterSensed > 5000){ //fail to fill if no water sensed for 5 seconds
        return true;
    }
    if(millis() - fillITK_time_startFillingITK > 100000){ //fail to fill if pumping taking longer the 100 seconds
        return true;
    }
    return false;
}

void fillITK(){

    task_enum i = FILLITK;

    switch (task_list[i].state)
    {
    case STATE_STARTING:
        fillITK_init();
        // task_list[i].hardStart();
        break;
    
    case STATE_RUNNING:
        fillITK_runnable();
        break;
    
    case STATE_STOPPING:
        fillITK_deinit();
        task_list[i].hardStop();
        break;
    } 

    
}

#endif
#ifndef _sampling_h_
#define _sampling_h_

#include "_task_cfg.h"
#include "memory.h"
#include <Arduino.h>
#include "shiftRegisterHandler.h"
#include "SampleControl.h"
#include "global.h"
#include "data.h"
#include "wifi.h"
#include "priming.h"
#include "draining.h"

/// Instructs sampling machine and takes a sample
void sampling();  
void sampling_init();   
void sampling_runnable(); 
void sampling_deinit();   


/******** Functions ********/
bool sampling_sampling();
void sampling_resetSampler();
void sampling_error();
void sampling_sendNumSample();
/******** Variables ********/
unsigned long sampling_timeLastStep;
int sampling_period;
int sampling_length;
int samplling_sampleState;
int sampling_bottleCounter;
uint16_t sampling_drainLenth2;
String sampling_numSample = "N1";
int sampling_numSampleTaken;
int sampling_testID;
bool sampling_samplingEnabled;
Sampler sampler(sampling_error); //creates a sampler object and what function to call when theres an error

enum sampling_stateEnum{
    sampling_state_drainingPending,
    sampling_state_DRAINING,
    sampling_state_CLEARSAMPLERLINES,
    sampling_state_CLEARSAMPLERWASTE,
    sampling_state_SAMPLING,
    sampling_state_DRAINING1,
    sampling_state_DRAINING2,
};
uint8_t sampling_state = sampling_state_DRAINING; 
/***************************/

void sampling_init(){
    print("Starting Sampling...");
    sampling_testID = memoryRead(sampling_EEPROM_testID);
    sampling_state = 0;
    samplling_sampleState = 0;
    sampler.state = 0;
    sampling_period = memoryRead(sampling_EEPROM_period);
    sampling_length = memoryRead(sampling_EEPROM_length);
    sampling_drainLenth2 = memoryRead(draining_EEPROM_drainLength) * 1000;
    sampling_bottleCounter = memoryRead(PRIMING_EEPROM_BOTTLECOUNTER);
    sampling_samplingEnabled = memoryRead(sampling_EEPROM_samplingEnabled);
    Serial1.setTimeout(900); //time given to read string from serial, machine will hang during this time
    if(sampling_samplingEnabled){
        sampler.askNumSample();
    }
}

void sampling_runnable(){
    if (isError() && errorState() != SAMPLERERROR){ //stop if there was an error other than sampler error
        task_list[SAMPLING].stop();
    }
    else if(errorState() == SAMPLERERROR){//fix sampler if sampler error
        shiftRegister_turnOff(RELAY_12V_OUTPUTPUMP);
        //turn off fill itk because sampler reset will hang machine. Priming shouldn't start while sampling is running anyway
        priming_deinit();
        task_list[FILLITK].hardStop();
        sampling_resetSampler(); //long hanging reset function
        task_list[FILLITK].start(); //restart 
    }
    else { 
        switch(sampling_state){
            case sampling_state_drainingPending: ////do nothing if paused, else Start draining

                if(!paused){
                    task_list[SAMPLING].allowWifiReconnect = false;
                    sampling_timeLastStep = millis();
                    sampling_state = sampling_state_DRAINING;
                }
                
            break;
            case sampling_state_DRAINING: //Start draining task for 10 seconds, to clear main lines
                // task_list[DRAINING].start();
                shiftRegister_turnOn(RELAY_12V_OUTPUTPUMP);
                if(millis() - sampling_timeLastStep > 10000){
                    // if(sampling_samplingEnabled && (sampling_bottleCounter % sampling_period == 0 || sampling_bottleCounter == 1)){
                    //if sampling
                    shiftRegister_turnOn(RELAY_12V_SAMPLEVALVE_NC); //open sample valve to let water into sampler
                    shiftRegister_turnOn(RELAY_12V_WASTEVALVE_NO); //close waste valve to force water into sampler
                    // }
                    shiftRegister_turnOn(RELAY_12V_WASTEPUMP); //waste pump to help drain the sampler waste
                    sampling_state = sampling_state_CLEARSAMPLERLINES;
                    sampling_timeLastStep = millis();
                }
            break;

            case sampling_state_CLEARSAMPLERLINES: //Clear sampler lines for 3 seconds
                shiftRegister_turnOn(RELAY_12V_OUTPUTPUMP); //keep pump on
                if(millis() - sampling_timeLastStep > 3000){
                    // task_list[DRAINING].stop(); //output pump off
                    shiftRegister_turnOff(RELAY_12V_OUTPUTPUMP);
                    sampling_state = sampling_state_CLEARSAMPLERWASTE;
                    sampling_timeLastStep = millis();
                }
            break;

            case sampling_state_CLEARSAMPLERWASTE: //Clear sampler waste for 2 seconds
                if(millis() - sampling_timeLastStep > 2000){
                    shiftRegister_turnOff(RELAY_12V_WASTEPUMP); //waste pump off
                    sampling_state = sampling_state_SAMPLING;
                    sampling_timeLastStep = millis();
                }
            break;

            case sampling_state_SAMPLING: //take sample if needed
                if(!sampling_samplingEnabled || (sampling_bottleCounter % sampling_period && sampling_bottleCounter != 1)){ //Skip sample
                    print("skipping sample!");
                    shiftRegister_turnOn(RELAY_12V_OUTPUTPUMP); //keep pump on
                    shiftRegister_turnOff(RELAY_12V_SAMPLEVALVE_NC);
                    shiftRegister_turnOff(RELAY_12V_WASTEVALVE_NO);
                    sampling_state = sampling_state_DRAINING1;
                }
                else if(sampling_sampling()){ //keep looping sampler machine process, returns true when finished
                    sampling_state = sampling_state_DRAINING1;
                    // sampling_timeLastStep = millis();
                    // sampling_drainLenth2 = 30000; // make drain time shorter
                }
            break;

            case sampling_state_DRAINING1: //finish draining bottle
                    sampling_timeLastStep = millis();
                    sampling_state = sampling_state_DRAINING2;
            break;

            case sampling_state_DRAINING2: //finish draining bottle
                if(millis() - sampling_timeLastStep > (sampling_drainLenth2 - 15000)){
                    shiftRegister_turnOff(RELAY_12V_OUTPUTPUMP);
                    print("sampling finished!");
                    task_list[SAMPLING].stop();
                }
            break;
        }
    }
}

void sampling_deinit(){
    task_list[SAMPLING].allowWifiReconnect = true;
    print("Stopping Sampling...");
    shiftRegister_turnOff(RELAY_12V_SAMPLEVALVE_NC);
    shiftRegister_turnOff(RELAY_12V_WASTEVALVE_NO);
    shiftRegister_turnOff(RELAY_12V_WASTEPUMP);
    shiftRegister_turnOff(RELAY_12V_OUTPUTPUMP);
}

///Controls the sampling machine procedure, returns true when finished
bool sampling_sampling(){
    switch (samplling_sampleState)
    {
    case 0: //Move sample needle to last position
        sampler.lastPosition();
        if(sampler.samplerReady()){
            print("Step 0");
            samplling_sampleState++;
        }
        break;
    
    case 1: //Wait untill ready and turn on pump to sample
        if(sampler.samplerReady()){
            print("Step 1");
            shiftRegister_turnOn(RELAY_12V_SAMPLEVALVE_NC);
            shiftRegister_turnOn(RELAY_12V_OUTPUTPUMP);
            samplling_sampleState++;
            sampling_timeLastStep = millis();
        }
        break;

    case 2: //Wait sample time and switch valves to output rest of bottle to waste. also send number of sample
        shiftRegister_turnOn(RELAY_12V_OUTPUTPUMP); //make sure output on
        if(millis() - sampling_timeLastStep > sampling_length){
            print("Step 2");
            shiftRegister_turnOff(RELAY_12V_WASTEVALVE_NO);
            shiftRegister_turnOff(RELAY_12V_SAMPLEVALVE_NC);
            shiftRegister_turnOn(RELAY_12V_OUTPUTPUMP); //make sure output on
            sampling_sendNumSample();
            samplling_sampleState++;
        }
        break;

    case 3: //moves sampler to next position
        sampler.nextPosition();
        if(sampler.samplerReady()){
            print("Step 3");
            shiftRegister_turnOff(RELAY_12V_WASTEPUMP);
            samplling_sampleState++;
        }
        break;

    case 4: //Put sampler to rinse position
        shiftRegister_turnOn(RELAY_12V_OUTPUTPUMP); //make sure output on
        sampler.rinsePosition();
        if(sampler.samplerReady()){
            print("Step 4");
            sampler.askNumSample();
            samplling_sampleState = 0;
            return true;
        }
        break;
    }
    return false;
}

void sampling_sendNumSample(){
    sampling_numSample = sampler.askNumSample();
    String temp = sampling_numSample;
    temp.remove(0,1);
    sampling_numSampleTaken = temp.toInt();
    double values[] = {
        sampling_numSampleTaken,
        sampling_testID,
    };
    String fields[] = {
        "sample_num",
        "test_id",
    };
    for(int i = 0; i < 2; i++){uploadToGrafana(fields[i],values[i]);}
}

void sampling_error(){
    print("Sampler error!");
    errorState(SAMPLERERROR);
}

void sampling_resetSampler(){
    sampler.reset();
    errorState(RUNNING);
}
/**********************************/

void sampling(){

    task_enum i = SAMPLING; //sampling //put your task enum from task_cfg.h here

    switch (task_list[i].state)
    {
    case STATE_STARTING:
        sampling_init();
        task_list[i].hardStart();
        break;
    
    case STATE_RUNNING:
        sampling_runnable();
        break;
    
    case STATE_STOPPING:
        sampling_deinit();
        task_list[i].hardStop();
        break;
    } 
}

#endif


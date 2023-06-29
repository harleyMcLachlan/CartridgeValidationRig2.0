#ifndef _priming_h_
#define _priming_h_

#include "_task_cfg.h"
#include "global.h" //for pin names
#include "Arduino.h" //for arduino related commands
#include "shiftRegisterHandler.h" //for controlling the relay output
#include "switch_cfg.h" //for accessing switch states
#include "memory.h"
#include "data.h"

void priming();
void priming_init();
void priming_runnable();
void priming_deinit(); 

/******** Functions ********/
void priming_calculatePWM();
void priming_startPriming();
void priming_upload();
/******** Variables ********/
enum priming_state_enum{
    PRIMING_WAITING,
    PRIMING_IDLE,
    PRIMING_OPEN1,
    PRIMING_CLOSE1,
    PRIMING_OPEN2,
    PRIMING_CLOSE2,
    PRIMING_DOSINGFINISHED,
};

unsigned long priming_doseTime;
unsigned long priming_timeOpen1;
int priming_timeOpen2;
int priming_timeClose1;
int priming_numDoses;
unsigned long priming_time_start;
bool priming_hasStarted = false;
unsigned long priming_time_endPumping = 0;
int priming_state;
unsigned long priming_timeLastStep;
int SWITCH_ITK_LOW_Counter = 0;
const int priming_TARGET_TIME_DIFFERENCE = 3000; //The target time(ms) that the pump should run after the last prime pulse

// int priming_allowance;
uint8_t priming_doseCounter;
int priming_stagnationTime[3];
uint8_t priming_numBatches;
uint16_t priming_numBottlesPerBatch;
int priming_timeBetweenBatches[3];
bool priming_CO2enabled;
bool priming_ContinuesPrimingEnabled;
bool priming_PWMadjust;

/******* DATA **************/
int priming_bottleCounter;
int priming_PWM;
int priming_pumpPrimeDifference;
int priming_actualTimeStagnated;
unsigned long pump_time;
float flow_rate;
int priming_batchCounter;
/***************************/

void priming_init(){
    priming_PWM = memoryRead(PRIMING_EEPROM_PWM);
    priming_doseTime = memoryRead(PRIMING_EEPROM_DOSETIME);
    priming_timeOpen1 = memoryRead(PRIMING_EEPROM_TIMEOPEN1);
    priming_timeOpen2 = memoryRead(PRIMING_EEPROM_TIMEOPEN2);
    priming_timeClose1 = memoryRead(PRIMING_EEPROM_TIMECLOSE1);
    priming_numDoses = memoryRead(PRIMING_EEPROM_NUMDOSES);
    priming_stagnationTime[0] = memoryRead(PRIMING_EEPROM_stagnationTime);
    priming_stagnationTime[1] = memoryRead(PRIMING_EEPROM_stagnationTime_2);
    priming_stagnationTime[2] = memoryRead(PRIMING_EEPROM_stagnationTime_3);
    priming_bottleCounter = memoryRead(PRIMING_EEPROM_BOTTLECOUNTER);
    priming_batchCounter = memoryRead(PRIMING_EEPROM_BATCHCOUNTER);
    priming_numBatches = memoryRead(PRIMING_EEPROM_NUMBATCHES);
    priming_numBottlesPerBatch = memoryRead(PRIMING_EEPROM_NUMBOTTLESPERBATCH);
    priming_timeBetweenBatches[0] = memoryRead(PRIMING_EEPROM_TIMEBETWEENBATCHES);
    priming_timeBetweenBatches[1] = memoryRead(PRIMING_EEPROM_TIMEBETWEENBATCHES_2);
    priming_timeBetweenBatches[2] = memoryRead(PRIMING_EEPROM_TIMEBETWEENBATCHES_3);
    priming_CO2enabled = memoryRead(PRIMING_EEPROM_CO2ENABLED);
    priming_ContinuesPrimingEnabled = memoryRead(PRIMING_EEPROM_CONTPRIMENABLED);
    priming_PWMadjust = memoryRead(PRIMING_EEPROM_PWMADJUST);
    priming_state = PRIMING_WAITING;
    
    //LOADING PWM
    if(priming_PWM < 140 || priming_PWM > 255 || isnan(priming_PWM)){ //if pwm from memory is out of range, reset to a reasonable value
        priming_PWM = 200; 
        memoryWrite(PRIMING_EEPROM_PWM,priming_PWM);
    }
    print("Preparing to Prime... with ");
    print(priming_stagnationTime[priming_batchCounter % 3],false);
    print(" seconds stagnation",false);
}

void priming_runnable(){
    if(!isError()){ //Stop if error state
        //Turn off pump if tank empty OR if taking longer than 80 seconds (some sort of error, water drop stuck on sensor or high pressure, pump failing)
        if(!switch_list[SWITCH_ITK_LOW].on() || (millis() - priming_time_start > 80000L && priming_hasStarted)){
                if(SWITCH_ITK_LOW_Counter++ > 10){ //end priming if Low level water sensor signal is stable 
                    priming_time_endPumping = millis();
                    priming_pumpPrimeDifference = (priming_time_endPumping  - priming_time_start - priming_numDoses*priming_doseTime); //calculate the time between end of pumping and end of pulsing
                    pump_time = (priming_time_endPumping  - priming_time_start);
                    flow_rate = (51600.0/pump_time);
                    priming_upload();//upload values to grafana
                    if(priming_PWMadjust && millis() - priming_time_start < 80000L) priming_calculatePWM(); //if adjustment is on, and the 80 second timeout didn't occur, adjust pwm
                    print("Finished Pumping!");
                    memoryWrite(PRIMING_EEPROM_BOTTLECOUNTER, ++priming_bottleCounter); //increment and store in mememory the bottle number
                    task_list[FILLITK].start(); //restart itk filling, stop priming, start sampling
                    task_list[PRIMING].stop(); 
                    task_list[SAMPLING].start();
                    SWITCH_ITK_LOW_Counter = 0;
                }
        }
        else{
            SWITCH_ITK_LOW_Counter = 0; //reset debounce counter
        }

        if(priming_state != PRIMING_WAITING) shiftRegister_turnOn(RELAY_12V_BOTTLEFILLVALVE_NC); //keeps filling valve open

        switch (priming_state)
        {
        case PRIMING_WAITING: //Waiting for stagnation time to end
            if(priming_batchCounter < priming_numBatches){
                if(task_list[SAMPLING].state == STATE_STOPPED && task_list[FILLITK].state == STATE_STOPPED){//wait for sampling and fillitk to stop
                    if(priming_bottleCounter % priming_numBottlesPerBatch == 0 && priming_bottleCounter){ //check if ready for next batch
                        //waiting for batch stagnation time
                        if(millis() - priming_time_endPumping > (long(priming_timeBetweenBatches[priming_batchCounter % 3]) * 1000 * 60)){
                            if(wifi_client.connected()){ //wait for wifi to be connected
                                priming_startPriming(); //begin priming sequence
                                print("Next Batch starting");
                                memoryWrite(PRIMING_EEPROM_BATCHCOUNTER, ++priming_batchCounter); //increment and store in mememory the batch number
                            }
                        }
                    }//waiting normal stagnation time
                    else if(millis() - priming_time_endPumping > long(priming_stagnationTime[priming_batchCounter % 3]) * 1000UL || priming_bottleCounter == 0){
                        if(wifi_client.connected()){ //wait for wifi connection
                            priming_startPriming(); //begin priming sequence
                        }
                    }
                }
            }
            else{
                errorState(FINISHED);
            }
            break;

        case PRIMING_IDLE: //Waiting for next dose to start
            analogWrite(PIN_FEEDPUMP,priming_PWM);
                if(!priming_ContinuesPrimingEnabled){ //if continuous priming is disabled
                    if(millis() - priming_timeLastStep > (priming_doseTime - priming_timeOpen1
                        - priming_timeClose1 - priming_timeOpen2)){
                        priming_timeLastStep = millis();
                        if(priming_CO2enabled){
                            priming_state = PRIMING_OPEN1; //If CO2 enabled, continue through dosing
                        }
                        shiftRegister_turnOn(RELAY_12V_BOTTLEFILLVALVE_NC);//Make sure valve stays open
                    }
                            
                }
                else{ //continous priming, dont progress through the priming sequence
                    shiftRegister_turnOn(RELAY_12V_BOTTLEFILLVALVE_NC);//Make sure valve stays open
                    shiftRegister_turnOn(RELAY_24V_CO2VALVE1_NC); // Open valve one
                    shiftRegister_turnOn(RELAY_24V_CO2VALVE2_NC); // Open valve two
                }
            break;

        case PRIMING_OPEN1: //Open valve one
            shiftRegister_turnOn(RELAY_24V_CO2VALVE1_NC);
            priming_state = PRIMING_CLOSE1;
            break;

        case PRIMING_CLOSE1: //Close Valve one
            if(millis() - priming_timeLastStep > priming_timeOpen1){
                priming_timeLastStep = millis();
                shiftRegister_turnOff(RELAY_24V_CO2VALVE1_NC);
                priming_state = PRIMING_OPEN2;
            }
            break;

        case PRIMING_OPEN2: //Open valve two
            if(millis() - priming_timeLastStep > priming_timeClose1){
                priming_timeLastStep = millis();
                shiftRegister_turnOn(RELAY_24V_CO2VALVE2_NC);
                priming_state = PRIMING_CLOSE2;
            }
            break;

        case PRIMING_CLOSE2: //Close Valve two
            if(millis() - priming_timeLastStep > priming_timeOpen2){
                priming_timeLastStep = millis();
                shiftRegister_turnOff(RELAY_24V_CO2VALVE2_NC);
                priming_state = PRIMING_IDLE;

                if(++priming_doseCounter == priming_numDoses){
                    print("Finished Dosing!");
                    priming_state = PRIMING_DOSINGFINISHED;
                }
            }
            break;
        case PRIMING_DOSINGFINISHED:
            //Done dosing, waiting for pump to finish
            break;
        }
    }
    else task_list[PRIMING].stop();
}

void priming_deinit(){
    priming_hasStarted = false;
    shiftRegister_turnOff(RELAY_24V_CO2VALVE1_NC); //reset all the valves
    shiftRegister_turnOff(RELAY_24V_CO2VALVE2_NC);
    shiftRegister_turnOff(RELAY_12V_BOTTLEFILLVALVE_NC);
    analogWrite(PIN_FEEDPUMP, 0); //turn off pump
    task_list[PRIMING].allowWifiReconnect = true;
    print("Stopped Priming");
}

void priming_calculatePWM(){
    //calculating new PWM    adjustment = (time took to pump) / (time it should take)      new PWM = old PWM * adjustmnet
    float pwmAdjustment = double(priming_time_endPumping - priming_time_start) / double(((double(priming_numDoses)*double(priming_doseTime)) + priming_TARGET_TIME_DIFFERENCE));
    priming_PWM = priming_PWM*pwmAdjustment;

    //check if the new pwm is too slow or fast, error if so
    if(priming_PWM < 140){
        memoryWrite(PRIMING_EEPROM_PWM,200);
        errorState(TOOFAST);
        print("ERROR pump was too fast!");
    }
    else if(priming_PWM > 255){
        errorState(TOOSLOW);
        memoryWrite(PRIMING_EEPROM_PWM,200);
        print("ERROR pump was too slow!");
    }
    else{
        memoryWrite(PRIMING_EEPROM_PWM,priming_PWM);  
    }
}

void priming_startPriming(){
    task_list[SENSORS].start(); //start collecting sensor data
    task_list[PRIMING].allowWifiReconnect = false; //disable wifi connection during priming
    task_list[DRAINING].stop(); //stop draining if for some reason it was on, maybe user forgot it on
    priming_hasStarted = true;
    print("Started Priming...PWM ");
    print(priming_PWM,false);
    priming_timeLastStep = millis();
    priming_time_start = millis();
    priming_actualTimeStagnated = (millis() - priming_time_endPumping) / 1000; //seconds since last priming = stagnation time
    priming_doseCounter = 0;
    priming_state = PRIMING_IDLE; //begin pulse cycles
}

void priming_upload(){
    double values[] = {
        priming_PWM,
        priming_bottleCounter,
        priming_pumpPrimeDifference,
        pump_time,
        flow_rate,
        priming_actualTimeStagnated,
        priming_batchCounter + 1
    };
    String fields[] = {
        "pwm",
        "bottle_count",
        "pmp_pr_dif",
        "pump_time",
        "flow_rate",
        "time_stg",
        "batch_count"
    };
    for(int i = 0; i < 7; i++){uploadToGrafana(fields[i],values[i]);}
    
}
/**********************************/

void priming(){

    task_enum i = PRIMING; //priming //put your task enum from task_cfg.h here

    switch (task_list[i].state)
    {
    case STATE_STARTING:
        priming_init();
        task_list[i].hardStart();
        break;
    
    case STATE_RUNNING:
        priming_runnable();
        break;
    
    case STATE_STOPPING:
        priming_deinit();
        task_list[i].hardStop();
        break;
    } 
}

#endif


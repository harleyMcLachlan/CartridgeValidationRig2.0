//Contains the sensors that run during priming, except the flow sensor.
#ifndef _sensors_h_
#define _sensors_h_

#include "_task_cfg.h"
#include "memory.h" //for using EEPROM
#include "Arduino.h" //for arduino related commands
#include "pressureSensor.h"
#include "tempSensor.h"
#include "global.h"
#include "data.h"
#include "priming.h"

/// Collects sensory data, temp and pressure
void sensors();
void sensors_init();
void sensors_runnable();
void sensors_deinit();

/******** Functions ********/

/***************************/

/******** Variables ********/
TempThermistorSensor sensors_temp(PIN_TEMPERATURESENSOR);
PressureSensor sensors_preMucPressure(PIN_PRESSURE1,0,10);
PressureSensor sensors_postMucPressure(PIN_PRESSURE2,0,2.5);
PressureSensor sensors_CO2Pressure(PIN_CO2PRESSURE,0,10);

unsigned long sensors_pressureLastTime = 0;
unsigned long sensors_stopTime = 0;
int sensors_CO2PressureLowCounter = 0;
int sensors_preMucPressureHighCounter = 0;

/***************************/

void sensors_init(){
    print("Starting Sensors!");
    uploadToGrafana("temperature_itk", sensors_temp.readTemperature());
}

void sensors_runnable(){ //Reads pressure every 250 ms
    if(!isError()){//Stop if error 
        if(millis() - sensors_pressureLastTime > 250){
            double p[3] = {
                sensors_preMucPressure.readPressure(),
                sensors_postMucPressure.readPressure(),
                sensors_CO2Pressure.readPressure()
            };
            String s[3] = {
                "pressure1",
                "pressure2",
                "pressureCO2"
            };
            for(int i = 0; i < 3; i++){
                uploadToGrafana(s[i],p[i]);
            }
            
            if(p[0] > 4) {
                if (sensors_preMucPressureHighCounter++ >10){
                    errorState(HIGHPRESSURE); //error if pre muc pressure higher than 4
                    print("ERROR: High pressure!");
                    sensors_preMucPressureHighCounter = 0;
                }              
            }
            else{
                sensors_preMucPressureHighCounter = 0;
            }
            if(p[2] < 6.2 && priming_CO2enabled) {//Counts how many times the pressure is low
                if(sensors_CO2PressureLowCounter++ > 10){ //error if CO2 Pressure too low for too long
                    errorState(LOWCO2PRESSURE); 
                    print("ERROR: LOW CO2 pressure!");
                    sensors_CO2PressureLowCounter = 0;
                }
            }
            else{
                sensors_CO2PressureLowCounter = 0;
            }
            sensors_pressureLastTime = millis();
        }

        //keep running 30 seconds after priming stops
        if(task_list[PRIMING].state != STATE_STOPPED) sensors_stopTime = millis();
        else if(millis() - sensors_stopTime > 30000){
            task_list[SENSORS].stop();
        }
    }
    else task_list[SENSORS].stop();
}

void sensors_deinit(){
    uploadToGrafana("temperature_itk", sensors_temp.readTemperature());
    print("Stopping Sensors");
}

/**********************************/

void sensors(){

    task_enum i = SENSORS;

    switch (task_list[i].state)
    {
    case STATE_STARTING:
        sensors_init();
        task_list[i].hardStart();
        break;
    
    case STATE_RUNNING:
        sensors_runnable();
        break;
    
    case STATE_STOPPING:
        sensors_deinit();
        task_list[i].hardStop();
        break;
    } 
}

#endif


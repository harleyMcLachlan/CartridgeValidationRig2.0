#ifndef _global_h
#define _global_h

#include "_task_cfg.h"
#include <Arduino.h>
#include "memory.h"

///Prints message if printing is enabled. Used for debugging outputs probably.
void print(String msg, bool newLine = true);
///Returns integer of which tasks are running. decode into binary to read. For example 00000000 0000101 means task enums 1 and 3 are running
int tasksRunning();

unsigned long timeErrorOccured;

int tasksRunning(){
    int tasks = 0;
    for(int i = 0; i < MAX_TASKS; i++){
        if(task_list[i].state != STATE_STOPPED){
            bitSet(tasks, i);
        }
    }
    return tasks;
}

template <class T>
void print(T msg, bool newLine = true){
    if(memoryRead(system_EEPROM_serialPrintsEnabled)){
        if(newLine) {Serial.println();}
        Serial.print(msg);
    }
}

//UNO WIFI REV2 PINS
#define PIN_BTN1                8
#define PIN_ITK_LOW             2
#define PIN_ITK_HIGH            A1 //rig3000 uses A5, other rigs use A1 due to electrical problem on PCB
#define PIN_SWITCH1             6
#define PIN_CAPACATIVESENSOR    4
#define PIN_TEMPERATURESENSOR   A0
#define PIN_PRESSURE1           A4
#define PIN_PRESSURE2           A2
#define PIN_CO2PRESSURE         A3
#define PIN_LEAK                7
#define PIN_SWITCH2             5 
#define PIN_LED                 9
//DIGITAL PWM                = 10, 9, 6, 5, 3
#define PIN_BUZZER              3
#define PIN_FEEDPUMP            10
     
///clock pin
#define PIN_SHIFTREG_SRCLK      11
///latch pin
#define PIN_SHIFTREG_RCLCK      12 
///data pin
#define PIN_SHIFTREG_SERIN      13


          

#endif
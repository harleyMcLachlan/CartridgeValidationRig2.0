// How to use: put your switch device name in the enum list and instantiate in the switch_list following the example

#ifndef _switch_cfg_h_
#define _switch_cfg_h_
#include "Switch.h"
#include "global.h"
#include "_task_cfg.h"
#include "_task_EXAMPLE.h"

enum switch_enum{
    /* insert switch enums here */
    SWITCH_ITK_HIGH,
    SWITCH_ITK_LOW,
    SWITCH_CAPACITIVEWATERSENSOR,
    SWITCH_leakHandler,
    SWITCH_PAUSE,
    SWITCH_DRAINING,

    /****************************/
    MAX_SWITCHES,
};

Switch switch_list[MAX_SWITCHES] = {
    /*
    {   ** EXAMPLE **
        task_enum that will get started from _task_cfg.h, (use "IDLE" when you dont want to attach a function)
        pin, 
        pin input mode = INPUT_PULLUP,
        polarity = LOW
        debounce time = 50ms
    },
    */
   //Switches are default active, you can call mySwitchName.stop() or start() to change this
    {
        IDLE,
        PIN_ITK_HIGH,
        INPUT,
        LOW,
    },

    {
        IDLE,
        PIN_ITK_LOW,
        INPUT,
        LOW,
    },

    {
        IDLE,
        PIN_CAPACATIVESENSOR, //capacitive sensor
    },

    {
        LEAKHANDLER,
        PIN_LEAK,
    },
    
    {
        IDLE,
        PIN_SWITCH1, //pause
    },

    {
        DRAINING,
        PIN_SWITCH2,
    },

};

#endif
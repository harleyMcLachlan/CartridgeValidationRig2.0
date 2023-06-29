//How to use: put your button name in the enum list, then instantiate it in the btn_list following the example

#ifndef _btn_cfg_h_
#define _btn_cfg_h_

#include "OneButton.h"
#include "btn_cfg.h"
#include "_task_cfg.h"
#include "global.h"
#include "leakHandler.h"

enum btn_enum{
    /* insert button enums here */
    BTN_START,
    /****************************/
    MAX_BTNS,
};

OneButton btn_list[MAX_BTNS] = {
    /*
    {   ** EXAMPLE **
        task_enum that will get started from _task_cfg.h, (use "IDLE" when you dont want to attach a function),
        pin, 
        ground ON = true,
        internal pullup active = false
    },
    */
    {
        FILLITK,
        PIN_BTN1, //start
    },
   //Buttons are updated every loop, you can call myBtnName.stop() or start() to change this
};

#endif
#ifndef _btnhandler_h_
#define _btnhandler_h_

#include "btn_cfg.h"
#include "btnHandler.h"
#include "switch_cfg.h"

///Updates the states of buttons and switches
void btnHandler();
void btnHandler_init();
void btnHandler_runnable();
void btnHandler_deinit();

/******** Functions ********/

/***************************/

/******** Variables ********/

/***************************/

void btnHandler(){

    task_enum i = BTNHANDLER;

    switch (task_list[i].state)
    {
    case STATE_STARTING:
        btnHandler_init();
        task_list[i].hardStart();
        break;
    
    case STATE_RUNNING:
        btnHandler_runnable();
        break;
    
    case STATE_STOPPING:
        btnHandler_deinit();
        task_list[i].hardStop();
        break;
    } 

    
}

void btnHandler_init(){
    print("Button Handler started");
}

void btnHandler_runnable(){
    for(int i = 0; i < MAX_BTNS; i++){

        if(btn_list[i].ticking_state == BTN_TICKING_ACTIVE){
            btn_list[i].tick();
        }

        if(switch_list[i].ticking_state == SW_TICKING_ACTIVE){
            switch_list[i].poll();
        }

    }

    for(int i = 0; i < MAX_SWITCHES; i++){
    
        if(switch_list[i].ticking_state == SW_TICKING_ACTIVE){
            switch_list[i].poll();
        }

    }
}

void btnHandler_deinit(){
    
}

#endif
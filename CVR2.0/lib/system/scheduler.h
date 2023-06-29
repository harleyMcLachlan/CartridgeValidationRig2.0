#ifndef _scheduler_h_
#define _scheduler_h_

#include <_task_cfg.h>
#include <Arduino.h>


using namespace std;

///Call from main loop to run tasks
void scheduler_run();

///Flags the task to run if ready
void scheduler_updateTasks();

///Runs the flagged tasks
void scheduler_runTasks();

///Flags task as waiting
void scheduler_flagTasksToWaiting();

///Calls the function with a pointer to the function
void start_function(void (*functionPTR)());


void scheduler_run(){
    scheduler_updateTasks();
    scheduler_runTasks();
    scheduler_flagTasksToWaiting();
}

void scheduler_updateTasks(){
    //Decrements the timer on the task if its waiting
    //Sets it ready to run and resets timer
   for (int i = 0; i < MAX_TASKS; i++)
    { 
        if(task_list[i].state == STATE_WAITING){
            if(--task_list[i].timer < 1){
                task_list[i].state = STATE_READY;
                task_list[i].timer = task_list[i].loop_ms;
            }
        }
    //Flags to running
        if(task_list[i].state == STATE_READY){
            task_list[i].state = STATE_RUNNING;
        }
    }
}

void scheduler_runTasks(){
    for (int i = 0; i < MAX_TASKS; i++)
    {   
        //Runs task init
        if(task_list[i].state == STATE_STARTING){
            start_function(task_list[i].fptr);
        }
        
        //Runs task runnable
        if(task_list[i].state == STATE_RUNNING){
            start_function(task_list[i].fptr);
        }

        //Runs task deinit
        if(task_list[i].state == STATE_STOPPING){
            start_function(task_list[i].fptr);
        }
    }
}

void scheduler_flagTasksToWaiting(){
    for (int i = 0; i < MAX_TASKS; i++)
    {
        if(task_list[i].state == STATE_RUNNING && task_list[i].loop_ms != 0){
            task_list[i].state= STATE_WAITING;
        }
    }
}

void start_function(void (*functionPTR)()){
    functionPTR();
}
#endif
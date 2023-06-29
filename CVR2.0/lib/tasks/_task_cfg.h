//put your task names in the enum list

#ifndef _task_cfg_h_
#define _task_cfg_h_

enum errorState_enum{
    NOT_RUNNING,
    RUNNING,
    TOOFAST,
    TOOSLOW,
    SAMPLERERROR,
    FAILTOFILL,
    PAUSED,
    LEAK,
    LOWCO2PRESSURE,
    HIGHPRESSURE,
    FINISHED,
};

errorState_enum errorState();
errorState_enum errorState(errorState_enum state);
bool isError();

enum task_state{
    //Active states
    STATE_STARTING,
    STATE_RUNNING,
    STATE_READY,
    STATE_WAITING,
    //Inactive states
    STATE_STOPPING,
    STATE_STOPPED,
};

enum task_enum{
    /* insert task enums here */
    PRIMING,
    FILLITK,
    LEAKHANDLER,
    SAMPLING,
    DRAINING,
    SENSORS,
    /*******System tasks*******/
    BTNHANDLER,
    SHIFTREGISTERHANDLER,
    IDLE,
    WIFI,
    MAX_TASKS,
};

class Task
{
public:

    ///constructor
    Task(void (*fptr)() = nullptr, int state = STATE_STOPPING, unsigned long loop_ms = 0, unsigned long timer = 0);

    ///Other tasks can call this to start running this task. calls the init of task
    void start();

    ///Internal start function *do not call from other tasks* (skips the deinit)
    void hardStart();
    
    ///Other tasks can call this to stop running this task. Calls the deinit of task
    void stop();

    ///Internal stop function *do not call from other tasks* (skips the deinit)
    void hardStop();

    ///Returns true if the task is starting ready running or waiting
    bool active();

    //Variables
    int state;
    int loop_ms;
    int timer;
    void (*fptr)();
    bool allowWifiReconnect = true; //Let the wifi know if its safe to try to reconnect to wifi. Because wifi connection causes a hang, it can get stuck with a pump on and cause a flood. This prevents that

private:
    
};

extern Task task_list[MAX_TASKS];

#endif


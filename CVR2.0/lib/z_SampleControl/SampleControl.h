#ifndef _sampler_h_
#define _sampler_h_
#include <Arduino.h>

class Sampler
{
public:
    Sampler(void (*errorFunctionPtr)());
    String askNumSample();
    String getNumSample();
    void gotoPosition(String pos);
    void nextPosition();
    void rinsePosition();
    void lastPosition();
    void write(char s[], uint8_t bufferSize);
    void write(char s);
    bool samplerReady();
    void reset();
    int state = 0;

private:
    #define MAX_COMM_ATTEMPTS 40 //number of attempts to ask for status or send a signal before restarting
    String numSample;
    int comm_attemp_counter = 0;
    bool cmdSuccess(char c = 'Z');
    String readString();
    void init();
    void flush();
    
    void (*fptr)();
};

#endif

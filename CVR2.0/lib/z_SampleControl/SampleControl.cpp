#include "SampleControl.h"
#include <Arduino.h>

String Sampler::readString(){
    // return Serial1.readStringUntil('\r');
    return Serial1.readString();
}

void Sampler::flush(){
    Serial1.flush();
}

void Sampler::write(char s[], uint8_t bufferSize){
    Serial.print("\nwriting ");
    Serial.print(s);
    Serial1.write(s);
}

void Sampler::write(char s){
    Serial1.write(s);
}

Sampler::Sampler(void (*errorFunctionPtr)()){
    fptr = errorFunctionPtr;
}

///initialize probe to starting sample. Must do before starting, or sampler will not work.
void Sampler::init(){
    switch (state)
    {
    case 0:
        write("I\r",2);
        if(this->cmdSuccess()) state++;
        break;
    
    case 1:
        write("G1\r",3);
        if(this->cmdSuccess()) state++;
        break;

    case 2:
        write("Ta200\r",6);
        if(this->cmdSuccess()) state = 0;
        break;
    }
}

///asks sampler for the sample number. Returns -1 if no response
String Sampler::askNumSample(){ //
    String temp;
    do {
        write("N\r",2);
        temp = this->readString();
        if(comm_attemp_counter++ > MAX_COMM_ATTEMPTS){fptr(); comm_attemp_counter = 0; break;}
        }
    while(temp.charAt(0) != 'N');
    if(temp.charAt(0) == 'N') numSample = temp;
    Serial.println("numSample is now " + numSample);
    return numSample;
}

///Reads the last sample number requested (does not ask for the current one)
String Sampler::getNumSample(){ 
    return numSample;
}

///Moves the sampler one position forward
void Sampler::nextPosition(){
    switch (state)
    {
    case 0:
        state++;
    case 1:
        if(numSample.charAt(1) == '0') write("G1\r",3); //Fixes the 0 to 68 sample jump
        else write("Gr1\r",4);
        if(this->cmdSuccess()) state++;
        break;
    
    case 2:
        write("Ta200\r",6);
        if(this->cmdSuccess()) state = 0;
        break;
    }
}

///move sampler over the waste hole
void Sampler::rinsePosition(){
    switch (state)
    {
    case 0:
        state++;
    case 1:
        write("GSp\r",4);
        if(this->cmdSuccess()) state++;
        break;
    
    case 2:
        write("Ta500\r",6);
        if(this->cmdSuccess()) state = 0;
        break;
    }
}

///Moves sampler to the last place askNumSample was called
void Sampler::lastPosition(){
    String numStr = numSample;
    if(numSample == ("NO")){numSample.replace('0','1');}
    char num[4];
    numStr.replace("N","G"); //replace N with
    numStr.concat("\r");
    numStr.toCharArray(num,5);

    switch (state)
    {
    case 0:
        state++;
    case 1:
        this->write(num, 4);
        if(this->cmdSuccess()) state++;
        break;
    
    case 2:
        this->write("Ta500\r",6);
        if(this->cmdSuccess()) state = 0;
        break;
    }
}

///checks if the command was recieved. return true if 'Z' recieved from sampler
bool Sampler::cmdSuccess(char c = 'Z'){
    if(comm_attemp_counter > MAX_COMM_ATTEMPTS){
        comm_attemp_counter = 0;
        fptr(); //call error function
        return true;
    }
    String response;
    response = this->readString();
    Serial.print("Response: " + response);
    if(response.charAt(0) == c){
        comm_attemp_counter = 0;
        return true;}
    else if(response == "E01"){
        this->reset();
        return false;}
    else{
        comm_attemp_counter++;
        return false;}
}

///Initializes sampler and sets to last position
void Sampler::reset(){
    comm_attemp_counter = 0;
    state = 0;
    Serial.print("Initializing");
    write("I\r",2);
    delay(100); //init
    write("I\r",2);
    Serial.print("Moving to last position");
    delay(40000);
    do{this->lastPosition();}
    while(!this->samplerReady());
    Serial.print("Moving to Rinse position");
    do{this->rinsePosition();}
    while(!this->samplerReady());
}

bool Sampler::samplerReady(){
    if(state)
        return false;
    return true;
}
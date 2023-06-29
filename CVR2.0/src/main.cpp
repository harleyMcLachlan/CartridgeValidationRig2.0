#include <Arduino.h>
#include <scheduler.h>
#include <EEPROM.h>

//function for writing the rig number into memory
void memoryWritem(int address, int number)
{ 
  EEPROM.update(address*2, number >> 8);
  EEPROM.update(address*2 + 1, number & 0xFF);
}

void setup() {
  Serial.begin(9600); //Serial Printing
  Serial1.begin(9600); //Sampler comms
  Serial.println("Starting machine");

  // memoryWritem(0,3009); //PUT RIG NUMBER HERE INTO MEMORY ONE TIME
}

void loop()
{
    while(true){
        scheduler_run();
    }
} 
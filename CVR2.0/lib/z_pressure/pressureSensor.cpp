#include "pressureSensor.h"

PressureSensor::PressureSensor(int readPin, float minSensorValueRelBar, float maxSensorValueRelBar){
    this->readPin = readPin;
    this->minValue = minSensorValueRelBar;
    this->maxValue = maxSensorValueRelBar;
}

float PressureSensor::readPressure(){
    int sensorReading = analogRead(readPin); //enter top and exit bottom pressure gauge
    
    return ((sensorReading/1023.0)*(maxValue-minValue)-minValue); 
}
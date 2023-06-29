#include "tempSensor.h"

TempThermistorSensor::TempThermistorSensor (int readPin, unsigned int thermistor_nominal, int series_resistor, int b_coefficient){
    this->readPin = readPin;
    this->thermistor_nominal = thermistor_nominal;
    this->series_resistor = series_resistor;
    this->b_coefficient = b_coefficient;
    this->NUM_SAMPLES = 5;
    this->TEMPERATURE_NOMINAL = 25;

}

float TempThermistorSensor::readTemperature(){

    // average all the samples out
    float value = analogRead(readPin);
    
    value = (1023.0 / value) - 1;
    value = series_resistor / value;

    // calculate
    float finalValue = calculateValue(value);
    
    return finalValue; 
}

float TempThermistorSensor::calculateValue(float rawValue){
    float steinhart;
    // T = B / (ln(R/Rn) + B/Tn), R: measured(value)
    steinhart = rawValue / thermistor_nominal; // (R/Rn)
    steinhart = log(steinhart); // ln(R/Rn)
    steinhart = steinhart + (b_coefficient/(TEMPERATURE_NOMINAL+273.15)); // ln(R/Rn) + B/Tn
    steinhart = b_coefficient/steinhart; // B/ (ln(R/Rn) + B/Tn)
    steinhart -= 273.15; // convert to C°
    
    float CurrentTemp = round(steinhart);
    return CurrentTemp;
}
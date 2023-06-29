#if defined(ARDUINO) && (ARDUINO >= 100)
#include <Arduino.h>
#else
#include <WProgram.h>
#endif

class TempThermistorSensor
{
public:
    TempThermistorSensor (int readPin, unsigned int thermistor_nominal=10000, int series_resistor=10000, int b_coefficient=3988);
    float readTemperature();
private:
    float calculateValue(float rawValue=1);
    int readPin;
    unsigned int thermistor_nominal;
    int series_resistor;
    int b_coefficient;
    int NUM_SAMPLES = 5;
    int TEMPERATURE_NOMINAL = 25;
};

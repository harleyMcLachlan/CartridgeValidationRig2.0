#if defined(ARDUINO) && (ARDUINO >= 100)
#include <Arduino.h>
#else
#include <WProgram.h>
#endif

class PressureSensor 
{
public:
    PressureSensor(int readPin, float minSensorValueRelBar, float maxSensorValueRelBar);
    float readPressure();
private:
    int readPin;
    float minValue;
    float maxValue;
};

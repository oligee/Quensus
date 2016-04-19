#ifndef Analogsensor_h
#define AnalogSensor_h
#include "Sensor.h"
#define BATTERY_INPUT_PIN 5
class AnalogSensor: public Sensor{
public:
  AnalogSensor(){
  }
  void measure(){value = analogRead(pinVAL);};
  void setUp(int pinVal){
    pinVAL=pinVal;
    pinMode(pinVAL, INPUT);

  };
private:
  int pinVAL = 0;
};
#endif

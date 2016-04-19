#ifndef Flowsensor_h
#define FlowSensor_h
#include "Arduino.h"
#include "Sensor.h"
#include <Wire.h>
#define FLOW_SENSOR_PIN  2
#define FLOW_SENSOR_INTERRUPT_PIN  0
#define refreshRate  1000

String pulseCountString;
volatile int pulseCount; //measuring the rising edges of the signal
float flowRate;
float dt;
int t_val = 0 ;
int pc = 0;
unsigned long oldTime;
void pulseCounter(){
    if (pulseCount==0)
        oldTime = millis();
    pulseCount++;
}
class FlowSensor:public Sensor{
public:
  FlowSensor(){};
  void measure(){
    // Reset variables
    dt = millis() - oldTime;
    // Read water sensor pulses if water flowing
    ressetData();
    pulseCountString = String(pulseCount);
    value = pulseCount;
  };
  void setUp(){
    pinMode(FLOW_SENSOR_PIN, INPUT);
    Wire.begin();
    //The Hall-effect sensor is connected to pin 2 which uses interrupt 0
    //initializes digital pin 2 as an input
    digitalWrite(FLOW_SENSOR_PIN, HIGH);
    pinMode(FLOW_SENSOR_INTERRUPT_PIN, INPUT);
    attachInterrupt(FLOW_SENSOR_INTERRUPT_PIN, pulseCounter, FALLING);
  };
private:
  void ressetData(){
      if ((pulseCount != 0) && (dt >= refreshRate)){
        // Disable water flow interrupt
        detachInterrupt (FLOW_SENSOR_INTERRUPT_PIN);
        // Reset variables
        t_val = millis();
        pc = pulseCount;
        pulseCount = 0;
        // re-enable interrupt
        attachInterrupt(FLOW_SENSOR_INTERRUPT_PIN, pulseCounter, FALLING);
    }
  };
};
#endif

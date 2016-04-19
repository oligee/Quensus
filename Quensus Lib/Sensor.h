#ifndef Sensor_h
#define Sensor_h
#include "Arduino.h"
//sensor base class
class Sensor{
public:
  Sensor(){};
  double getValue(){return value;}
  void measure(){};
  void setUp(){};
protected:
  double value;
};
#endif

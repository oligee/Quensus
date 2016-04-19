#ifndef valveLib_h
#define valveLib_h
#include "Arduino.h"
//valve
#define IN_VALVE_OPEN_PIN  3  //analog
#define IN_VALVE_CLOSE_PIN  4 // analog
#define OUT_RELAY_RED_PIN  5
#define OUT_RELAY_BLACK_PIN  9
#define OUT_STATUS_PIN  6
#define OPENED 1
#define CLOSED 2
#define OPENING 5
#define CLOSING 6


class Valve{
public:
//constructor
//valve attributes
long time = 0;
long debounce = 500;    // added time to allow button to be re-pressed
int pulseWidth = 2000;  // time to set the width of the output pulse
int valveStatus;
int high = 3.7/0.005;
Valve(){};
void valveSetup(){
  //valve set up -----------------------------
  pinMode(OUT_RELAY_RED_PIN, OUTPUT);
  pinMode(OUT_RELAY_BLACK_PIN, OUTPUT);
  pinMode(OUT_STATUS_PIN, OUTPUT);
  //pin defaults
  digitalWrite(OUT_RELAY_RED_PIN, LOW);
  digitalWrite(OUT_RELAY_BLACK_PIN, LOW);
  // Check to see if valve is currently open or closed
  // set value that an analog pin input must be before it is defined as high
  obtainStartupStatus();
 //valve setup complete------------------------
};
void changeState(){
  //alter the state of the valve , from on to off / off to on
  String openMessage = "VALVE OPENING";
  String closeMessage = "VALVE CLOSING";
  if(millis() - time > pulseWidth+debounce){
    if(valveStatus == CLOSED){
      Serial.println(openMessage);
      valveStatus = OPENING;
      digitalWrite(OUT_RELAY_BLACK_PIN, HIGH);
    }
    else if (valveStatus == OPENED)
    {
      Serial.println(closeMessage);
      valveStatus = CLOSING;
      digitalWrite(OUT_RELAY_RED_PIN, HIGH);
    }
    time = millis();
  }
}
void updateValve(){
  isValveOpen();
  isValveClosed();
}
private:
void isValveOpen(){
  if(analogRead(IN_VALVE_OPEN_PIN) > high && valveStatus==OPENING){
    // stop current flow
    Serial.println("VALVE OPENED");
    valveStatus = OPENED;
    digitalWrite(OUT_RELAY_BLACK_PIN, LOW);
    digitalWrite(OUT_STATUS_PIN, LOW);
  }
}
void isValveClosed(){
  //int val = analogRead(IN_VALVE_CLOSE_PIN);
  //Serial.println(high+ "and "+ val);
  if(analogRead(IN_VALVE_CLOSE_PIN) > high && valveStatus==CLOSING){
    // stop current flow
    Serial.println("VALVE CLOSED");
    valveStatus = CLOSED;
    digitalWrite(OUT_RELAY_RED_PIN, LOW);
    digitalWrite(OUT_STATUS_PIN, HIGH);
  }
}
void obtainStartupStatus(){
  // Check to see if valve is currently open or closed
  if(analogRead(IN_VALVE_OPEN_PIN) > high)
  {
    Serial.println("VALVE IS OPEN");
    digitalWrite(OUT_STATUS_PIN, LOW);
    valveStatus = OPENED;
  } else if (analogRead(IN_VALVE_CLOSE_PIN) > high)
  {
    Serial.println("VALVE IS CLOSED");
    digitalWrite(OUT_STATUS_PIN, HIGH);
    valveStatus = CLOSED;
  } else
  {
    // choose the default position
    Serial.println("VALVE IS BETWEEN OPEN AND CLOSED");
    valveStatus = CLOSED;
  }
}
};
#endif

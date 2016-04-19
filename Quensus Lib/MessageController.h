#ifndef MessageController_h
#define MessageController_h
#include "Arduino.h"
#include <Messenger.h>
#include <LLAPSerial.h>
//#include <BatterySensor.h>
#include <AnalogSensor.h>
#include <FlowSensor.h>
#include <HeatSensor.h>
#include <Settings.h>
#include <valveLib.h>
#include <math.h>
#define BUZZER_PIN 1
class MessageController{
public:
  //messaging
  Messenger messenger;
  //buffer sensors
  FlowSensor flowSensor;
  HeatSensor heatSensor;
  AnalogSensor batterySensor;
  AnalogSensor thermoSensor;
  Valve valve;
  //attributes
  int volatile counter= 0;
  int missedAcknowledgementNumber = 0;
  boolean signalAcknowledgement = false;
  //constructor
  MessageController(){
    LLAP.init(DEVICEID);
  };
  void setup(){
    int batterySensorPin = 5;
    //sensors setup
    flowSensor.setUp();
    batterySensor.setUp(batterySensorPin);
    thermoSensor.setUp(batterySensorPin);
    heatSensor.setUp();
  }
  void listenToHub(){
      counter++;
      //listen to signal
      if(LLAPmessageReceiver()){
        //signal received
        signalAcknowledgement = true;
        //resset the counter since we are still in comunication with the hub
        missedAcknowledgementNumber=0;
      }else{
        //not received
      }
  }
  bool getTalkTime(){
    //can we talk?
    flowSensor.measure();
    const int  SEND_INTERVAL = 5;
    if(counter%SEND_INTERVAL == 0){
      //yes
      return true;
    }else{
      //no
      return false;
    }
  }
  void messageAcknowledgement(){
    //check if our signal has been accepted
    if(signalAcknowledgement == false){
      //signal not recived
      missedAcknowledgementNumber++;
    }else{
      signalAcknowledgement = false;
    }
    if(missedAcknowledgementNumber == 5){
      //if five messages have not been acknowlegd then make a beep
      tone(BUZZER_PIN,1000,250);
      missedAcknowledgementNumber=0;
    }
  }
  void sendData(){
    const int  NUMBER_DATA_VARIATIONS = 3;
    const int  DATA_INTERVAL = 5;
    const int maxOption = NUMBER_DATA_VARIATIONS*DATA_INTERVAL;
    int commandNumber = counter/DATA_INTERVAL;
    switch(commandNumber) {
       case 1:
          sendFlowData();
          break;
       case 2:
          heatSensor.measure();
          sendHeatData();
          break;
      case  3:
          batterySensor.measure();
          sendBatData();
          break;
       default :
       break;
    }
    if(counter == maxOption){
      counter = 0;
    }else{
      counter++;
    }
  }
private:
  boolean LLAPmessageReceiver(){
    //used only to acknolege a previous message;
    boolean messageAcknowledgement = false;
    if (LLAP.bMsgReceived) {
      //collects message
      String command = LLAP.sMessage;
      //handles comand
      messageAcknowledgement = messenger.fcnReceive(command);
      LLAP.bMsgReceived = false;  // if we do not clear the message flag then message processing will be blocked
    }
    return messageAcknowledgement;
  }
  void sendFlowData(){
      messenger.fcnSend("VAL",flowSensor.getValue());
  }
  void sendBatData(){
      messenger.fcnSend("BAT",batterySensor.getValue());
  }
  void sendHeatData(){
      //messenger.fcnSend("TMP",dtostrf(heatSensor.getValue(), 6, 2, buffer));
      sendThermoHeatData();
  }
  void sendThermoHeatData(){
    int RawADC = thermoSensor.getValue();
    double Temp;
    Temp = log(10000.0*((1024.0/RawADC-1)));
    //=log(10000.0/(1024.0/RawADC-1)) // for pull-up configuration
    Temp = 1 / (0.001129148 + (0.000234125 + (0.0000000876741 * Temp * Temp ))* Temp );
    Temp = Temp - 273.15;            // Convert Kelvin to Celcius
    messenger.fcnSend("TMP",Temp);
  }
};
#endif

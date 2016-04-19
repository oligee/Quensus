#ifndef Messenger_h
#define Messenger_h
#include "Arduino.h"
#include "ValveLib.h"
#define HUBID "A1" // this is the LLAP Hub ID
#define LED_PIN 12
class Messenger{
public:
  bool state = false;
  Valve valve;
  //constructor with passed in valve
  Messenger(){
    pinMode(LED_PIN, OUTPUT);     // led pin;
    digitalWrite(LED_PIN,HIGH );
    valve.valveSetup();
  };
  //send message to hub
  boolean fcnSend(String command,String value){
    //takes in recipiant ID and the Message
    String message = command+":"+value;
    if(message.length()<=9){
      // message is needed to be less than 9 characters
      String command = "a";
      int messageLength = 1;
      command.concat(HUBID);
      //required to be upper case so can be seen
      message.toUpperCase();
      command.concat(message);
      messageLength = command.length();
      //add padding
      while(messageLength <12){
        command.concat("-");
          messageLength = command.length();
      }
      Serial.println(command);
      //return true meaning the message was created okay
      return true;
    }
    //return true meaning the message was too long
     fcnSend(command,value.substring(0,5));
    return false;
  }
  void fcnSend(String command,int value){
      fcnSend(command,String(value));
  }
  void fcnSend(String command,double value){
      fcnSend(command,String(value));
  }
  void fcnSend(String command,float value){
      fcnSend(command,String(value));
  }
  //recive message from hub
  boolean fcnReceive(String incomingCommand){
    //takes in the command and checks it against the list of commands
    valve.updateValve();
    String  stateOnCMD   = "DONEON---";
    String  stateOffCMD  = "DONEOFF--";
    bool returnVal = false;
    if(incomingCommand == stateOnCMD){
      //MESSAGE ACKNOWLEDGED
      returnVal = stateOn();
    }
    else if(incomingCommand == stateOffCMD){
      //MESSAGE ACKNOWLEDGED
      returnVal =  stateOff();
    }else{
      //COMMAND NOT RECOGNISED
      returnVal = cmdNotFound();
    }
    return returnVal;
  }
  bool stateOn(){
    if(state == false){
      //change valve state
      valve.changeState();
      digitalWrite(LED_PIN, HIGH);
      state = true;
    }
    //returns true indicating messages was recived
    return true;
  }
  bool stateOff(){
    if(state == true){
      //change valve state
      valve.changeState();
      digitalWrite(LED_PIN, LOW);
      state = false;
    }
    //returns true indicating messages was recived
    return true;
  }
  bool cmdNotFound(){
    //just returns false if a command is not found , potential for more action to be taken here
    return false;
  }
private:
};
#endif

#include <LLAPSerial.h>
#define DEVICEID "A1"  // this is the LLAP device ID

char incomingByte;   // the command buffer
String commandBuilder = "";
int sensorPin = 2;
int sensorValue = 0;
void setup(){
  pinMode(12, OUTPUT);   // initialize pin 12 as digital output (LED)
  pinMode(8, OUTPUT);    // initialize pin 8 to control the radio
  digitalWrite(8, HIGH); // select the radio
  Serial.begin(115200);    // start the serial port at 115200 baud
  delay(1000);            // allow the radio to startup
  LLAP.init(DEVICEID);
  Serial.println("STARTED");
  //Start up
  Serial.print("ABCDEFGHIJKLMNOPQRSTUVWX");
  Serial.flush();
}
  
void loop(){
  //Check for messages
   LLAPmessageReceiver();
  //checkButtonPress();
  //LLAPmessageSender("A1","on");
  delay(1000);      
}
void commandHandler(String incomingCommand){
  //takes in the command
  if(incomingCommand == "ON-------"){
    //turn on light
    digitalWrite(12, HIGH);
    Serial.println("LED ON");
  }else if(incomingCommand =="OFF------"){
    //turn light off
    digitalWrite(12, LOW);
    Serial.println("LED OFF");
  }else{
    Serial.println("COMMAND NOT RECOGNISED");
  }
}
void LLAPmessageReceiver(){
  if (LLAP.bMsgReceived) {
    //collects message
    Serial.print("message is:");
    Serial.println(LLAP.sMessage);
    String command = LLAP.sMessage;
    commandHandler(command);
    LLAP.bMsgReceived = false;  // if we do not clear the message flag then message processing will be blocked
  }
}
boolean LLAPmessageSender(String ID,String message){
  //takes in recipiant ID and the Message 
  if(message.length()<=9){
    // message is needed to be less than 9 characters
    String command = "a";
    int messageLength = 1;
    command.concat(ID);
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
  return false;
}
void checkButtonPress(){
  sensorValue = analogRead(sensorPin); 
  Serial.print(sensorValue);
  if(sensorValue > 1000){
    Serial.print("threshold passed");
     LLAPmessageSender("A2","on");
  }else{
    Serial.print(sensorValue);
  }
}


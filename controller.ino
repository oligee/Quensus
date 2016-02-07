#include <LLAPSerial.h>
#define DEVICEID "A1"  // this is the LLAP device ID

char incomingByte;   // the command buffer
String commandBuilder = "";
int sensorValue = 0;
boolean lightOn;
void setup(){
  pinMode(12, OUTPUT);   // initialize pin 12 as digital output (LED)
  pinMode(8, OUTPUT);    // initialize pin 8 to control the radio 
  pinMode(5, OUTPUT);    // led pin;
  pinMode(1, INPUT);     // pin for variable Resistor input
  digitalWrite(5, LOW);  //start of the LED to off
  digitalWrite(8, HIGH); // select the radio
  Serial.begin(115200);  // start the serial port at 115200 baud
  delay(1000);           // allow the radio to startup
  LLAP.init(DEVICEID);
  //Start up
  Serial.println("STARTED");
  Serial.print("ABCDEFGHIJKLMNOPQRSTUVWX");
  Serial.flush();
}

void loop(){
  //Check for messages
  LLAPmessageReceiver();
  //get input
  sensorValue = analogRead(1); 
  //send input
  String valCommand = "VAL";
  // sending value from the resistor 
  valCommand.concat(String(sensorValue));
  LLAPmessageSender("A1",valCommand);
  delay(100);
}
void commandHandler(String incomingCommand){
  //takes in the command
  if(incomingCommand == "ON-------"){
    //turn on light
    digitalWrite(5, HIGH);
    Serial.println("LED ON");
  }else if(incomingCommand =="OFF------"){
    //turn light off
    digitalWrite(5, LOW);
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



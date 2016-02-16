#include <LLAPSerial.h>
#define DEVICEID "A2"  // this is the LLAP device ID

char incomingByte;   // the command buffer
String commandBuilder = "";
int sensorValue = 0;
boolean lightOn;
int volatile counter= 0;
int lastReading;
int missedAcknowledgementNumber = 0;
boolean signalAcknowledgement = false;;
//String extraCommands[10];
void setup(){
  //sleep mode stuff
  pinMode(4,OUTPUT);           // hardwired XinoRF / RFu328 SRF sleep pin 
  digitalWrite(4,LOW);          // pull sleep pin high - sleep 2 disabled
  Serial.print("+++");            // enter AT command mode
  delay(1500);                   // delay 1.5s
  Serial.println("ATSM2");         // enable sleep mode 2 <0.5uA
  delay(2000);
  Serial.println("ATDN");          // exit AT command mode*/
  delay(2000);
  //Done 

  //Pins and extras 
  pinMode(12, OUTPUT);   // initialize pin 12 as digital output (LED)
  pinMode(8, OUTPUT);    // initialize pin 8 to control the radio 
  pinMode(6, OUTPUT);    // led pin;
  pinMode(1, INPUT);     // pin for variable Resistor input
  digitalWrite(6,HIGH );  //start of the LED to off
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
  if(counter>10){
    //sending message to the hub
    sendDataToHub();
  }else{
    //listern to responses from the hub
    listernToHub();
  }
  delay(100);
}
void sendDataToHub(){
    sendSensorData();
    counter=0;
    //check if our signal has been accepted
    if(signalAcknowledgement == false){
      //signal not recived
      missedAcknowledgementNumber++;
    }else{
      signalAcknowledgement = false;
    }
    if(missedAcknowledgementNumber == 5){
      tone(5,1000,250);
      missedAcknowledgementNumber=0;
    }
}
void listernToHub(){
    counter++;
    if(LLAPmessageReceiver()){
      //not received
    }else{
      //signal received
      signalAcknowledgement = true;
      //resset the counter since we are still in comunication with the hub
      missedAcknowledgementNumber=0;
    }
}
boolean commandHandler(String incomingCommand){
  //takes in the command
  if(incomingCommand =="DONEON---"){
    //message Acknowledgement
    digitalWrite(6, HIGH);
    //Serial.println("MESSAGE ACKNOWLEDGED");
    return false;
  }
  else if(incomingCommand =="DONEOFF--"){
    //message Acknowledgement
    digitalWrite(6, LOW);
    //Serial.println("MESSAGE ACKNOWLEDGED");
    return false;
  }else{
    //command not recognised
    //Serial.println("COMMAND NOT RECOGNISED");
  }
  return true;
}
boolean LLAPmessageReceiver(){
  //used only to acknolege a previous message;
  boolean messageAcknowledgement;
  if (LLAP.bMsgReceived) {
    //collects message
    //Serial.print("message is:");
    //Serial.println(LLAP.sMessage);
    String command = LLAP.sMessage;
    messageAcknowledgement = commandHandler(command);
    LLAP.bMsgReceived = false;  // if we do not clear the message flag then message processing will be blocked
  }
  return messageAcknowledgement;
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
void sleepMode(){
  delay(10);                    // allow radio to finish sending
  digitalWrite(4, HIGH);        // pull sleep pin high to enter SRF sleep 2
  LLAP.sleepForaWhile(1000);    // sleep ATmega328 for 5s (ms)
  digitalWrite(4, LOW);         // when ATmega328 wakes up, wake up SRF Radio
  delay(10); 
}
void sendSensorData(){
  //read data
  sensorValue = analogRead(1); 
  //if(sensorValue!= lastReading){
    lastReading = sensorValue;
    //send input
    String valCommand = "VAL:";
    // sending value from the resistor 
    valCommand.concat(String(sensorValue));
    LLAPmessageSender("A1",valCommand);
  //}

}
  void sleepController(){
      delay(10);                    // allow radio to finish sending
      //Serial.println();
      digitalWrite(6, LOW);  
      digitalWrite(4, HIGH);        // pull sleep pin high to enter SRF sleep 2
      LLAP.sleepForaWhile(980);    // sleep ATmega328 for 5s (ms)
      digitalWrite(4, LOW);         // when ATmega328 wakes up, wake up SRF Radio
       digitalWrite(6, HIGH);  
      delay(10);                    // allow radio to wake up
  }



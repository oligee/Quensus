#include <LLAPSerial.h>
#define DEVICEID "A1"  // this is the LLAP device ID
#include <LCD16x2.h>
#include <Wire.h>
//LCD SCREEN
LCD16x2 lcd;
int buttons;

char incomingByte;   // the command buffer
String commandBuilder = "";
int sensorPin = 2;
int sensorValue = 0;
String lastCommand;

//Led on;
boolean isLedOn = false;
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

  Wire.begin();
  lcd.lcdClear();
  lcd.lcdGoToXY(1,1);
  lcd.lcdWrite("LED:");
  lcd.lcdGoToXY(1,2);
  lcd.lcdWrite("VALUE:");
  
}
  
void loop(){
  //Check for messages
   LLAPmessageReceiver();
   checkButtonPress();
  delay(100);      
}
void commandHandler(String incomingCommand){
  //takes in the command
  //splits the comand from the data 
  String command = incomingCommand.substring(0,4);
  String dataVal = incomingCommand.substring(4);
  
  //checks commands against list 
  
  if(command =="VAL:"){
    // converts data to a formate needed for the LCD
    char dataPointer[20];
    dataVal.toCharArray(dataPointer, 6);
    lcd.lcdGoToXY(8,2);
    lcd.lcdWrite(dataPointer);
    //displays the value of the resistance
    Serial.println(dataPointer);
    LLAPmessageSender("A2","DONE");
  }else if(command =="DNE:"){
    //message accepted
  }else{
    //command does not meet up to one expected for the hub
    LLAPmessageSender("A2",lastCommand);
    Serial.println("COMMAND NOT RECOGNISED");
    Serial.println(command);
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
  lastCommand = message;
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
    //return true meaning the message was created okay
    return true;
  }
  //return true meaning the message was too long
  return false;
}
void checkButtonPress(){
  //gets data for the buttons
   buttons = lcd.readButtons();
   //moves to x = 6 y = 1 possition on the LCD
   //Displays if the buton is pressed
   lcd.lcdGoToXY(6,1);
    if(buttons & 0x01){ 
      //if(isLedOn){
        lcd.lcdWrite("OFF");
        LLAPmessageSender("A2","OFF");
        isLedOn = false;
      //}
    }else{
      //if(!isLedOn){
        lcd.lcdWrite("ON ");
        LLAPmessageSender("A2","ON");
        isLedOn = true;
      //}
    }
}



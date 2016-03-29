#include <LLAPSerial.h>
#define DEVICEID "A1"  // this is the LLAP device ID
#include <LCD16x2.h>
#include <Wire.h>
//LCD SCREEN
LCD16x2 lcd;
int buttons;
String commandBuilder = "";
String lastCommand;
float total =0;
float flowRate = 0;
int refreshRate = 1000;
//Led on;
boolean isLedOn = false;
boolean screenOne = false;
//last sensor reading from the controller
float lastPulseCount = 0;
float heatVal = 0;
int batteryVal = 0;
int screenVal = 1;
//flow boundries
//amount of time in seconds a flow can be continueous for before action taken
double timeBoundry = 10200;
double timeCounter = 0;
// the max flow rate
double maxFlow = 5;
//buttons
boolean buttonFourDown = true;
boolean screenSELECTED = false;
int timeSelected = 0;
int blinker = 0;
//time set up
double hours = (timeBoundry -(fmod(timeBoundry,3600)))/3600;
double mins = (timeBoundry -(hours*3600)-(fmod((fmod(timeBoundry,3600)),60)))/60;
double secs = timeBoundry - (hours*3600)-(mins * 60);
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
  //LCD setup
  setUpLCD();
}
void setUpLCD(){
  //setup LCD
  applyScreenOne();
}
void loop(){
  //main loop
  //Check for messages
  LLAPmessageReceiver();
  //check if the button has been pressed
  checkButtonPress();
  blinkingTimeDisplay();
  delay(50);
}
void blinkingTimeDisplay(){
   if(screenSELECTED &&  screenVal == 3  ){
    if(blinker > 10){
      if(timeSelected == 1){
        lcd.lcdGoToXY(2,2);
        lcd.lcdWrite(" ");
      }else if(timeSelected == 2){
        lcd.lcdGoToXY(7,2);
        lcd.lcdWrite(" ");
      }else if(timeSelected == 3){
        lcd.lcdGoToXY(12,2);
        lcd.lcdWrite(" ");
      }
      blinker++;
      if(blinker == 14)
      blinker=0;
    }else{
      if(timeSelected == 1){
        lcd.lcdGoToXY(2,2);
        lcd.lcdWrite(":");
      }else if(timeSelected == 2){
        lcd.lcdGoToXY(7,2);
        lcd.lcdWrite(":");
      }else if(timeSelected == 3){
        lcd.lcdGoToXY(12,2);
        lcd.lcdWrite(":");
      }
        blinker++;
    }
  }
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
    //displays the value of the resistance
    lastPulseCount = dataVal.toFloat();
    updateFlowTotal();
    //increament pulse time counter
    updatePulseCounter();
    messageAcknowledgement();
  }else if (command =="TMP:"){
    // converts data to a formate needed for the LCD
    char dataPointer[20];
    dataVal.toCharArray(dataPointer, 6);
    //displays the value of the resistance
    heatVal = dataVal.toFloat();
    updateFlowTotal();
    messageAcknowledgement();
  }else if (command == "BAT:"){
    char dataPointer[20];
    dataVal.toCharArray(dataPointer, 6);
    batteryVal = dataVal.toInt();
    updateFlowTotal();
    messageAcknowledgement();
  }else{
    //command does not meet up to one expected for the hub
    LLAPmessageSender("A2",lastCommand);
    Serial.println("COMMAND NOT RECOGNISED");
    Serial.println(command);
  }

}
void messageAcknowledgement(){
  if(isLedOn){
      //send the on state
      LLAPmessageSender("A2","DONEON");
    }else{
       LLAPmessageSender("A2","DONEOFF");
    }
}
void updatePulseCounter(){
  //used to monitor the pulses over time and controll if they have passed a set threshhold
  if(lastPulseCount>0){
      timeCounter++;
      if(timeCounter>=timeBoundry && screenVal != 3 &&  screenSELECTED == false){
        isLedOn = true;
      }
    }else{
      timeCounter = 0;
    }
}
void LLAPmessageReceiver(){
  if (LLAP.bMsgReceived) {
    //collects message
    //Serial.println(LLAP.sMessage);
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
    Serial.println(command);
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
    //button one
    if(buttons & 0x01){
    }else{
     buttonOnePressed();
    }
    //button two
    if(buttons & 0x02){
    }else{
      buttonTwoPressed();
    }
    //button three
    if(buttons & 0x04){
    }else{
       buttonThreePressed();
    }
    //button four
    if(buttons & 0x08){
      if(!buttonFourDown){
        buttonFourDown = true;
      }
    }else{
      buttonFourPressed();
    }
}
void buttonOnePressed(){
  //button one pressed
  if(screenSELECTED & screenVal == 3 ){
    //increase the max flow time by an hour
    incrementTime();
  }else if(!isLedOn){
     isLedOn = true;
  }
}
void buttonTwoPressed(){
  //button two pressed
  if(screenSELECTED & screenVal == 3 ){
    //increase the max flow time by an minute
    decrementTime();
  }else if(isLedOn){
    isLedOn = false;
  }
}
void buttonThreePressed(){
  //button three has been pressed
  if(!screenSELECTED)
  screenSELECTED = true;
  //change the selected time slot
  if(screenSELECTED & screenVal == 3 ){
    timeSelected++;
    if(timeSelected>3){
      timeSelected =1;
    }
    screenChange();
  }
}
void buttonFourPressed(){
  //button four pressed
  if(buttonFourDown){
    screenSELECTED = false;
    buttonFourDown = false;
    screenIncrement();
  }
}
void incrementTime(){
  static const int TWENTYFOUR_HOUR_VAL = 86400;
  static const int SIXTY_MIN_VAL = 3600;
  static const int SIXTY_SEC_VAL = 60;
  if(timeSelected==1){
           hours++;
           if(hours>23){
            hours = 0;
            timeBoundry = timeBoundry  - TWENTYFOUR_HOUR_VAL;
           }else{
            timeBoundry = timeBoundry  + SIXTY_MIN_VAL;
           }
        }
        if(timeSelected==2){
           mins++;
           if(mins>59){
            mins = 0;
            timeBoundry = timeBoundry  - SIXTY_MIN_VAL;
           }else{
             timeBoundry = timeBoundry  + SIXTY_SEC_VAL;
           }
        }
        if(timeSelected==3){
           secs++;
           if(secs>59){
            secs = 0;
            timeBoundry = timeBoundry  - SIXTY_SEC_VAL;
           }else{
            timeBoundry = timeBoundry  + 1;
           }
        }
        //update screen
        screenChange();
}
void decrementTime(){
  static const int TWENTYFOUR_HOUR_VAL = 86400;
  static const int SIXTY_MIN_VAL = 3600;
  static const int SIXTY_SEC_VAL = 60;
  if(timeSelected==1){
           hours--;
           if(hours<0){
            hours = 23;
            timeBoundry = timeBoundry  + TWENTYFOUR_HOUR_VAL;
           }else{
            timeBoundry = timeBoundry  - SIXTY_MIN_VAL;
           }
        }
        if(timeSelected==2){
           mins--;
           if(mins<0){
            mins = 59;
            timeBoundry = timeBoundry  + SIXTY_MIN_VAL;
           }else{
             timeBoundry = timeBoundry  - SIXTY_SEC_VAL;
           }
        }
        if(timeSelected==3){
           secs--;
           if(secs<0){
            secs = 59;
            timeBoundry = timeBoundry  + SIXTY_SEC_VAL;
           }else{
            timeBoundry = timeBoundry  - 1;
           }
        }
        //update screen
        screenChange();
}
void updateFlowTotal(){
    //prepare varriable for display
    char dataPointer[20];
    char dataPointer2[20];
    //create flow rate
    flowRate = (1000 / refreshRate) * (lastPulseCount / 7.5);
    //update total
    total += flowRate * (float)refreshRate / 1000.0 / 60.0;
    //if the flow rate exceeds a set value then change state
    if(flowRate>maxFlow && screenVal != 4 &&  screenSELECTED == false){
        isLedOn = true;
    }
    //display flow rate and total
    if(screenVal==1){
      String(flowRate).toCharArray(dataPointer, 6);
      String(total).toCharArray(dataPointer2, 6);
      //check the flow rate is safe
      writeValuesToScreen(dataPointer,dataPointer2);
    }else if(screenVal==2){
      //display flow rate and heat
      String(flowRate).toCharArray(dataPointer, 6);
      String(heatVal).toCharArray(dataPointer2, 6);
      //check the flow rate is safe
      writeValuesToScreen(dataPointer,dataPointer2);
    }else if(screenVal == 4 && screenSELECTED && flowRate>maxFlow){
      //used to set the new max flow rate
      //if screen is set and the selected button has been pressed
      // then the new flow rate will be used as the new max if greater than value before
      // the value is then displayed
      maxFlow = flowRate;
      char dataPointer[20];
      String(maxFlow).toCharArray(dataPointer, 5);
      lcd.lcdGoToXY(6,2);
      lcd.lcdWrite(dataPointer);
    }else if(screenVal == 5){
      char dataPointer[20];
      String(batteryVal).toCharArray(dataPointer, 5);
      lcd.lcdGoToXY(10,1);
      lcd.lcdWrite(dataPointer);
      batteryStatusDisplay();
    }
}
void writeValuesToScreen(char dataPointer[],char dataPointer2[] ){
    //displays the value to the screen using a char array
    //used in displaying the values passed from the controller
    lcd.lcdGoToXY(8,1);
    lcd.lcdWrite(dataPointer);
    lcd.lcdGoToXY(8,2);
    lcd.lcdWrite(dataPointer2);
}
void screenChange(){
  //apply a screen display dependent on the screenVal; that keeps track of the users current screen
  // and allows for the rotational display
  switch (screenVal) {
    case 1:
      applyScreenOne();
      break;
    case 2:
      applyScreenTwo();
      break;
    case 3:
      applyScreenThree();
    break;
     case 4:
      applyScreenFour();
    break;
    case 5:
     applyScreenFive();
   break;
  }
}
void screenIncrement(){
  //increase screen counter , if at max then start from begining
  lcd.lcdClear();
  screenVal++;
  if(screenVal == 6 )
    screenVal = 1;
  screenChange();
}
void screenDecrement(){
  //decreasse screen couter , if at lowest then start at the top
  lcd.lcdClear();
  screenVal--;
  if(screenVal == 0 )
    screenVal = 6;
  screenChange();
}
void applyScreenOne(){
  //display screen one
  lcd.lcdClear();
  lcd.lcdGoToXY(1,1);
  lcd.lcdWrite("VALUE:");
  lcd.lcdGoToXY(1,2);
  lcd.lcdWrite("TOTAL:");
  lcd.lcdGoToXY(8,1);
  lcd.lcdWrite("-");
  lcd.lcdGoToXY(8,2);
  lcd.lcdWrite("-");

}
void applyScreenTwo(){
  //dsplay screen two
  lcd.lcdClear();
  lcd.lcdGoToXY(1,1);
  lcd.lcdWrite("VALUE:");
  lcd.lcdGoToXY(1,2);
  lcd.lcdWrite("HEAT :");
  lcd.lcdGoToXY(8,1);
  lcd.lcdWrite("-");
  lcd.lcdGoToXY(8,2);
  lcd.lcdWrite("-");
}
void applyScreenThree(){
  //display screen three
  char dataPointer[20];
  lcd.lcdGoToXY(1,1);
  lcd.lcdWrite("MAX FLOW TIME");

  //display hours
  lcd.lcdGoToXY(1,2);
  lcd.lcdWrite("H:0");
  String(hours).toCharArray(dataPointer, 3);
  if(hours>9)
    lcd.lcdGoToXY(3,2);
  else
    lcd.lcdGoToXY(4,2);
  lcd.lcdWrite(dataPointer);
  lcd.lcdGoToXY(5,2);
  lcd.lcdWrite(" ");

  //display mins
  lcd.lcdGoToXY(6,2);
  lcd.lcdWrite("M:0");
  String(mins).toCharArray(dataPointer, 3);
  if(mins>9)
    lcd.lcdGoToXY(8,2);
  else
    lcd.lcdGoToXY(9,2);

  lcd.lcdWrite(dataPointer);
  lcd.lcdGoToXY(10,2);
  lcd.lcdWrite(" ");

  //displaying seconds
  lcd.lcdGoToXY(11,2);
  lcd.lcdWrite("S:0");
  String(secs).toCharArray(dataPointer, 3);
  if(secs>9)
    lcd.lcdGoToXY(13,2);
  else
    lcd.lcdGoToXY(14,2);
  lcd.lcdWrite(dataPointer);

  lcd.lcdGoToXY(15,2);
  lcd.lcdWrite(" ");

}
void applyScreenFour(){
  //display screen four
  lcd.lcdClear();
  lcd.lcdGoToXY(1,1);
  lcd.lcdWrite("MAX FLOW VALUE");
  lcd.lcdGoToXY(1,2);
  lcd.lcdWrite("VAL: ");
  char dataPointer[20];
  String(maxFlow).toCharArray(dataPointer, 5);
  lcd.lcdGoToXY(6,2);
  lcd.lcdWrite(dataPointer);
}
void applyScreenFive(){
  //display screen four
  lcd.lcdClear();
  lcd.lcdGoToXY(1,1);
  lcd.lcdWrite("VOLTAGE: ");
  lcd.lcdGoToXY(1,2);
  lcd.lcdWrite("STATUS: ");
  char dataPointer[20];
  String(batteryVal).toCharArray(dataPointer, 5);
  lcd.lcdGoToXY(10,1);
  lcd.lcdWrite(dataPointer);
  batteryStatusDisplay();
}
void batteryStatusDisplay(){
  int batteryVoltageInt = (int)batteryVal;
  lcd.lcdGoToXY(9,2);
  if(batteryVoltageInt > 810){
    lcd.lcdWrite("FULL");
  }else if(batteryVoltageInt > 750){
    lcd.lcdWrite("MEDIUM");
  }else{
    lcd.lcdWrite("LOW");
  }
}

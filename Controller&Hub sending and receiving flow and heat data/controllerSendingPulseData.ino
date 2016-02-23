#include <LLAPSerial.h>
#define DEVICEID "A2"  // this is the LLAP device ID
#include <Wire.h>

char incomingByte;   // the command buffer
String commandBuilder = "";
int sensorValue = 0;
boolean lightOn;
int volatile counter= 0;
int lastReading;
int missedAcknowledgementNumber = 0;
boolean signalAcknowledgement = false;;
//String extraCommands[10];
//globals 
String pulseCountString;
float calibrationFactor = 5.5;
volatile int pulseCount; //measuring the rising edges of the signal
byte sensorInterrupt = 0;  // 0 = digital pin 2
byte sensorPin       = 2;  //The pin location of the sensor
float flowRate;
float dt;
int refreshRate = 1000;
unsigned long oldTime;
float total;
String flowRate_str;
String total_str;
char flowRate_char[6];
char total_char[6];
long lastDebounce = 0;
long debounceDelay = 10;    // Ignore bounces under this amount of ms
int t_val = 0 ;
int pc = 0;
boolean sendType = false;

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
  flowPinSetup();
}
void flowPinSetup(){
  Wire.begin();
  // The Hall-effect sensor is connected to pin 2 which uses interrupt 0
  pinMode(sensorPin, INPUT); //initializes digital pin 2 as an input
  digitalWrite(sensorPin, HIGH);
  pinMode(sensorInterrupt, INPUT);
  attachInterrupt(sensorInterrupt, pulseCounter, FALLING);
  total = 0.0;
}
void loop(){
  //Check for messages
  if(counter>5){
    //sending message to the hub
      sendDataToHub();
  }else{
    //listern to responses from the hub
    listernToHub();
  }
  getFlowData();
  delay(100);
}
void sendDataToHub(){
    if(sendType){
      sendSensorData();
      sendType = false;
    }else{
      sendHeatData();
      sendType = true;
    }
    //sendFlowData();
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
   LLAPmessageSender(ID,message.substring(0,9));
  return false;
}
void sendSensorData(){
    LLAPmessageSender("A1","VAL:"+pulseCountString);
  //}
}
void sendHeatData(){
  //read data
  sensorValue = analogRead(1); 
  //if(sensorValue!= lastReading){
    lastReading = sensorValue;
    //send input
    String valCommand = "TMP:";
    // sending value from the resistor 
    valCommand.concat(String(sensorValue));
    //LLAPmessageSender("A1",valCommand);
    LLAPmessageSender("A1",valCommand);
  //}
}
void getFlowData(){
    // Reset variables
    dt = millis() - oldTime;
    // Read water sensor pulses if water flowing
    ressetData();
    //flowRate = (1000 / dt) * (pulseCount / calibrationFactor);
    //total += flowRate * (float)refreshRate / 1000.0 / 60.0;
    pulseCountString = String(pulseCount);
    //Serial.print (pulseCountString);
    //LLAPmessageSender("A1","VAL:"+pulseCountString);
    //output to the LCD
    //LCDOutput();
}
void ressetData(){
    if ((pulseCount != 0) && (dt >= refreshRate)){
      // Disable water flow interrupt
      detachInterrupt (sensorInterrupt);
      // Reset variables
      t_val = millis();
      pc = pulseCount;
      pulseCount = 0;
      // re-enable interrupt
      attachInterrupt(sensorInterrupt, pulseCounter, FALLING);
  }
}
void pulseCounter(){
    if (pulseCount==0)
        oldTime = millis();
    pulseCount++;
}




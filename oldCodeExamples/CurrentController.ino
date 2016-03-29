
/*CONTROLLER*/

#include <LLAPSerial.h>
#include <Wire.h>
#include <OneWire.h>
#include <DallasTemperature.h>
//Defines
#define DEVICEID "A2"  // this is the LLAP device ID
#define HUBID "A1" // this is the LLAP Hub ID
#define calibrationFactor 5.5
#define OPENED 1
#define CLOSED 2
//#define OPENING_PULSE 3
//#define CLOSING_PULSE 4
#define OPENING 5
#define CLOSING 6
//heat sensor
#define ONE_WIRE_BUS 3
//flow sensor
#define refreshRate  1000
#define LED_PIN 12
#define RADIO_PIN 8
//valve
#define IN_BUTTON_PIN  1
#define IN_VALVE_OPEN_PIN  13
#define IN_VALVE_CLOSE_PIN  10
#define OUT_RELAY_RED_PIN  11
#define OUT_RELAY_BLACK_PIN  12
#define OUT_STATUS_PIN  0
#define FLOW_SENSOR_PIN  2
#define FLOW_SENSOR_INTERRUPT_PIN  0  //?
//Defines
/* DS18S20 Temperature chip i/o */
char buffer[25];
byte maxsensors = 0;
OneWire  ds(ONE_WIRE_BUS);
DallasTemperature sensors(&ds);
//heat sensor complete
//messaging
int volatile counter= 0;
int missedAcknowledgementNumber = 0;
boolean signalAcknowledgement = false;
//messaging
//flow---
String pulseCountString;
volatile int pulseCount; //measuring the rising edges of the signal
float flowRate;
float dt;
unsigned long oldTime;
//flow---
long lastDebounce = 0;
long debounceDelay = 10;    // Ignore bounces under this amount of ms
int t_val = 0 ;
int pc = 0;
boolean sendType = false;
//valve stuff -----------------------
int status;
long time = 0;
long debounce = 500;    // added time to allow button to be re-pressed
int pulseWidth = 2000;  // time to set the width of the output pulse
//valve stuff complete---------------
void setup(){
  //Pins setup
  pinMode(IN_BUTTON_PIN, INPUT);
  pinMode(IN_VALVE_OPEN_PIN, INPUT);
  pinMode(IN_VALVE_CLOSE_PIN, INPUT);
  pinMode(OUT_RELAY_RED_PIN, OUTPUT);
  pinMode(OUT_RELAY_BLACK_PIN, OUTPUT);
  pinMode(OUT_STATUS_PIN, OUTPUT);
  pinMode(RADIO_PIN, OUTPUT);   // initialize pin 8 to control the radio
  digitalWrite(RADIO_PIN, HIGH); // select the radio
  pinMode(LED_PIN, OUTPUT);     // led pin;
  pinMode(FLOW_SENSOR_PIN, INPUT);
  //pin defaults
  digitalWrite(OUT_RELAY_RED_PIN, LOW);
  digitalWrite(OUT_RELAY_BLACK_PIN, LOW);
  digitalWrite(LED_PIN,HIGH );  //start of the LED to off

  //pin defaults complete
  Serial.begin(115200);  // start the serial port at 115200 baud
  delay(1000);           // allow the radio to startup
  //heat sensor setup
  scanSensors();
  //valve sensors setup
  valveSetup();
  //flow sensor setup
  flowSensorSetup();
  //give device ID
  LLAP.init(DEVICEID);
  Serial.println("STARTED");
  Serial.print("ABCDEFGHIJKLMNOPQRSTUVWX");
  Serial.flush();
}
void flowSensorSetup(){
  Wire.begin();
  // The Hall-effect sensor is connected to pin 2 which uses interrupt 0
  //initializes digital pin 2 as an input
  digitalWrite(FLOW_SENSOR_PIN, HIGH);
  pinMode(FLOW_SENSOR_INTERRUPT_PIN, INPUT);
  attachInterrupt(FLOW_SENSOR_INTERRUPT_PIN, pulseCounter, FALLING);
}
void valveSetup(){
  //valve set up -----------------------------
  // Check to see if valve is currently open or closed
  if(digitalRead(IN_VALVE_OPEN_PIN) == HIGH){
      Serial.println("VALVE IS OPEN");
      digitalWrite(OUT_STATUS_PIN, LOW);
      status = OPENED;
  } else if (digitalRead(IN_VALVE_CLOSE_PIN) == HIGH){
      Serial.println("VALVE IS CLOSED");
      digitalWrite(OUT_STATUS_PIN, HIGH);
      status = CLOSED;
  } else{
      // choose the default position
      Serial.println("VALVE IS BETWEEN OPEN AND CLOSED");
      status = CLOSED;
  }
 //valve setup complete------------------------
}
void loop(){
  //Check for messages
  if(talkCheck()){
  }else{
    //we did not talk so listern instead
    listernToHub();
  }
  //get the current flow value;
  getFlowData();
  batteryValue();
  //Serial.println(counter);
  //valve test
  //delay to prevent to create refresh time and prevent uneeded amount work
  //test code ---
  //newValveCode();
  //testing done ---
  delay(100);
}
bool talkCheck() {
  //test if we can talk
  bool talkTime = getTalkTime();
  if(talkTime){
    //if we can talk , send data to the hub
    sendData();
    //check for acknowledgement
    messageAcknowledgement();
    return true;
  }else{
    //let the loops know we did not talk and so we should listern instead
    return false;
  }
}
void sendData(){
  int maxOption = 15;
  int commandNumber = counter/5;
  switch(commandNumber) {
     case 1:
        sendSensorData();
        break;
     case 2:
        sendHeatData();
        break;
    case  3:
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
bool getTalkTime(){
  if(counter%5 == 0){
    return true;
  }else{
    return false;
  }
}
void listernToHub(){
    counter++;
    //listern to signal
    if(LLAPmessageReceiver()){
      //signal received
      signalAcknowledgement = true;
      //resset the counter since we are still in comunication with the hub
      missedAcknowledgementNumber=0;
    }else{
      //not received
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
    tone(5,1000,250);
    missedAcknowledgementNumber=0;
  }
}
boolean fcnReceive(String incomingCommand){
  #define stateOn  "DONEON---"
  #define stateOff "DONEOFF--"
  //takes in the command
  if(incomingCommand == stateOn){
    //message Acknowledgement
    digitalWrite(LED_PIN, HIGH);
    //Serial.println("MESSAGE ACKNOWLEDGED");
    changestate();
    return true;
  }
  else if(incomingCommand == stateOff){
    //message Acknowledgement
    digitalWrite(LED_PIN, LOW);
    //Serial.println("MESSAGE ACKNOWLEDGED");
    changestate();
    return true;
  }else{
    //command not recognised
    //Serial.println("COMMAND NOT RECOGNISED");
    return false;
  }

}
boolean LLAPmessageReceiver(){
  //used only to acknolege a previous message;
  boolean messageAcknowledgement = false;
  if (LLAP.bMsgReceived) {
    //collects message
    String command = LLAP.sMessage;
    messageAcknowledgement = fcnReceive(command);
    LLAP.bMsgReceived = false;  // if we do not clear the message flag then message processing will be blocked
  }
  return messageAcknowledgement;
}
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
boolean fcnSend(String command,int value){
    fcnSend(command,String(value));
}
boolean fcnSend(String command,double value){
    fcnSend(command,String(value));
}
boolean fcnSend(String command,float value){
    fcnSend(command,String(value));
}
void sendSensorData(){
    fcnSend("VAL",pulseCountString);
}
void sendBatData(){
  int sensorValue = analogRead(A5);
    fcnSend("BAT",sensorValue);
}
void sendHeatData(){
    //get heat sensor data
    sensors.requestTemperatures();
    //for each output data
    for (int i=0;i<maxsensors;i++){
      //read data in
      float f = sensors.getTempCByIndex(i);
      // sending values to hub
      fcnSend("TMP",dtostrf(f, 6, 2, buffer));
    }
}
void getFlowData(){
    // Reset variables
    dt = millis() - oldTime;
    // Read water sensor pulses if water flowing
    ressetData();
    pulseCountString = String(pulseCount);
}
void ressetData(){
    if ((pulseCount != 0) && (dt >= refreshRate)){
      // Disable water flow interrupt
      detachInterrupt (FLOW_SENSOR_INTERRUPT_PIN);
      // Reset variables
      t_val = millis();
      pc = pulseCount;
      pulseCount = 0;
      // re-enable interrupt
      attachInterrupt(FLOW_SENSOR_INTERRUPT_PIN, pulseCounter, FALLING);
  }
}
void pulseCounter(){
    if (pulseCount==0)
        oldTime = millis();
    pulseCount++;
}
void scanSensors(void){
  byte i;
  byte present = 0;
  byte data[12];
  byte addr[8];
  //creating heat sensors
  sensors.begin();
  while (ds.search(addr))
  {
    Serial.print("R=");
    for( i = 0; i < 8; i++) {
      Serial.print(addr[i], HEX);
      Serial.print(" ");
    }
    if ( OneWire::crc8( addr, 7) != addr[7]) {
        Serial.print("CRC is not valid!\n");
        return;
    }
    if ( addr[0] == 0x10) {
        Serial.print("Device is a DS18S20 family device.\n");
        maxsensors++;
    }
    else {
      if (addr[0] == 0x28) {
        Serial.print("Device is a DS18B20 family device.\n");
        maxsensors++;
      }
      else {
        Serial.print("Device is unknown!\n");
        Serial.print("Device ID: ");
        Serial.print(addr[0],HEX);
        Serial.println();
        return;
      }
    }
    // The DallasTemperature library can do all this work for you!
    ds.reset();
    ds.select(addr);
    ds.write(0x44,1);         // start conversion, with parasite power on at the end
    delay(1000);     // maybe 750ms is enough, maybe not
    // we might do a ds.depower() here, but the reset will take care of it.
    present = ds.reset();
    ds.select(addr);
    ds.write(0xBE);         // Read Scratchpad
    Serial.print("P=");
    Serial.print(present,HEX);
    Serial.print(" ");
    for ( i = 0; i < 9; i++) {           // we need 9 bytes
      data[i] = ds.read();
      Serial.print(data[i], HEX);
      Serial.print(" ");
    }
    Serial.print(" CRC=");
    Serial.print( OneWire::crc8( data, 8), HEX);
    Serial.println();
  }
  Serial.print("No more addresses.\n");
  ds.reset_search();
  delay(250);
}
void checkStateChange(){
  /*
 if(status == OPENING_PULSE && millis() - time > pulseWidth) {
   Serial.println("END OPEN PULSE");
   digitalWrite(outRelayOpen, LOW);
   status = OPENING;
 }
 if(status == CLOSING_PULSE && millis() - time > pulseWidth) {
   Serial.println("END CLOSE PULSE");
   digitalWrite(outRelayClose, LOW);
   status = CLOSING;
 }
 if(digitalRead(IN_VALVE_OPEN_PIN) == HIGH && status==OPENING)
 {
   // stop current flow
   Serial.println("VALVE OPENED");
   status = OPENED;
   digitalWrite(outSupply, LOW);
 }
 if(digitalRead(IN_VALVE_CLOSE_PIN) == HIGH && status==CLOSING)
 {
   // stop current flow
   Serial.println("VALVE CLOSED");
   status = CLOSED;
   digitalWrite(outSupply, LOW);
 }*/
}
void changestate(){
/*  if(status == OPENED){
    if( millis() - time > pulseWidth+debounce){
      if(status == CLOSED){
        Serial.println("START OPEN PULSE");
        status = OPENING_PULSE;
        digitalWrite(outSupply, HIGH);
        digitalWrite(outRelayOpen, HIGH);
      }
      else if (status == OPENED)
      {
        Serial.println("START CLOSE PULSE");
        status = CLOSING_PULSE;
        digitalWrite(outSupply, HIGH);
        digitalWrite(outRelayClose, HIGH);
      }
      time = millis();
    }
  }*/
}
void newValveCode(){
  if(digitalRead(IN_BUTTON_PIN) == HIGH && millis() - time > pulseWidth+debounce){
    if(status == CLOSED){
      Serial.println("VALVE OPENING");
      status = OPENING;
      digitalWrite(OUT_RELAY_BLACK_PIN, HIGH);
    }
    else if (status == OPENED){
      Serial.println("VALVE CLOSING");
      status = CLOSING;
      digitalWrite(OUT_RELAY_RED_PIN, HIGH);
    }
    time = millis();
  }
  if(digitalRead(IN_VALVE_OPEN_PIN) == HIGH && status==OPENING){
    // stop current flow
    Serial.println("VALVE OPENED");
    status = OPENED;
    digitalWrite(OUT_RELAY_BLACK_PIN, LOW);
    digitalWrite(OUT_STATUS_PIN, LOW);
  }
  if(digitalRead(IN_VALVE_CLOSE_PIN) == HIGH && status==CLOSING){
    // stop current flow
    Serial.println("VALVE CLOSED");
    status = CLOSED;
    digitalWrite(OUT_RELAY_RED_PIN, LOW);
    digitalWrite(OUT_STATUS_PIN, HIGH);
  }
}
void batteryValue(){
  int sensorValue = analogRead(A5);
  //Serial.print("Value = ");
  //Serial.print(sensorValue*0.005 *2);  // 0.005V sensitivity, *2 from potential divider
  //Serial.print(sensorValue);  // 0.005V sensitivity, *2 from potential divider
  //Serial.println(" V ");
}

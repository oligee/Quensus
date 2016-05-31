
/*CONTROLLER*/
#include <settings.h>
#include <MessageController.h>
//Defines
#define BUZZER_PIN 1
#define RADIO_PIN 8 //?
//messaging
MessageController messageController;
void setup(){
  //Pins setup
  pinMode(RADIO_PIN, OUTPUT);   // initialize pin 8 to control the radio
  digitalWrite(RADIO_PIN, HIGH); // select the radio
  Serial.begin(115200);  // start the serial port at 115200 baud
  delay(1000);           // allow the radio to startup
  //give device ID
  LLAP.init(DEVICEID);
  Serial.println("STARTED");
  Serial.flush();
  messageController.setup();
  //sleep mode stuff
  pinMode(4,OUTPUT);           // hardwired XinoRF / RFu328 SRF sleep pin
  digitalWrite(4,LOW);          // pull sleep pin high - sleep 2 disabled
  Serial.print("+++");            // enter AT command mode
  delay(1500);                   // delay 1.5s
  Serial.println("ATSM2");         // enable sleep mode 2 <0.5uA
  delay(2000);
  Serial.println("ATDN");          // exit AT command mode*/
  delay(2000);
  //done
}
void loop(){
  //Check for message
  if(talkCheck()){
    //we have talked to the hub
  }else{
    //we did not talk so listen instead
    messageController.listenToHub();
  }
  //delay to prevent to create refresh time and prevent uneeded amount work
  delay(100);
//sleepController();
}
bool talkCheck() {
  //test if we can talk
  bool talkTime = messageController.getTalkTime();
  if(talkTime){
    //if we can talk , send data to the hub
    messageController.sendData();
    //check for acknowledgement
    messageController.messageAcknowledgement();
    return true;
  }else{
    //let the loops know we did not talk and so we should listern instead
    return false;
  }
}
void sleepController(){
    delay(10);                    // allow radio to finish sending
    //Serial.println();
    digitalWrite(6, LOW);
    digitalWrite(4, HIGH);        // pull sleep pin high to enter SRF sleep 2
    LLAP.sleepForaWhile(80);
    digitalWrite(4, LOW);
     digitalWrite(6, HIGH);
    delay(10);                    // allow radio to wake up
}

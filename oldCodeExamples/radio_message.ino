//BlinkwithRadio
void setup() {
  pinMode(12, OUTPUT);
  pinMode(8, OUTPUT);    // initialize pin 8 to control the radio.
  digitalWrite(8, HIGH); // select the radio
  Serial.begin(115200);

}
void loop() {
  digitalWrite(12, HIGH);   // set the LED on
  Serial.println("LED ON");
  delay(1000);              // wait for a second
  digitalWrite(12, LOW);    // set the LED off
  Serial.println("LED OFF");
  delay(100); 
}
void sendingData (){
  
}


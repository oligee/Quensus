/*
Use LCD screen to display flow rate information
Created by: Dan Simmons
Last modified: 18/01/2014
*/
#include <LCD16x2.h>
#include <Wire.h>
LCD16x2 lcd;
int buttons;
// The hall-effect flow sensor outputs approximately 7.5 pulses per second for a litre/minute of flow
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

void setup()
{
  pinSetup();
  // LCD setup
  Wire.begin();
  lcdScreenSetup();
  total = 0.0;
}
void lcdScreenSetup(){
  lcd.lcdClear();
  lcd.lcdGoToXY(1, 1);
  lcd.lcdWrite("Flow rate:");
  lcd.lcdGoToXY(1, 2);
  lcd.lcdWrite("Total mL:");
}
void pinSetup(){
  // The Hall-effect sensor is connected to pin 2 which uses interrupt 0
  pinMode(sensorPin, INPUT); //initializes digital pin 2 as an input
  digitalWrite(sensorPin, HIGH);
  Serial.begin(9600);
  pinMode(sensorInterrupt, INPUT);
  attachInterrupt(sensorInterrupt, pulseCounter, FALLING);
}
void loop ()
{
    // Reset variables
    dt = millis() - oldTime;
    // Read water sensor pulses if water flowing
    ressetData();
    flowRate = (1000 / dt) * (pulseCount / calibrationFactor);
    total += flowRate * (float)refreshRate / 1000.0 / 60.0;
    //output to the LCD
    LCDOutput();
}
void pulseCounter(){
    if (pulseCount==0)
        oldTime = millis();
    pulseCount++;
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
void LCDOutput(){
  // Print output
  Serial.print (flowRate, DEC);
  Serial.print (" L/min \r\nTotal= ");
  Serial.print (total, DEC);
  Serial.print (" L \r\n");
  //Get strings for output
  flowRate_str = String(flowRate, DEC);
  flowRate_str.toCharArray(flowRate_char, 6);
  total_str = String(total, DEC);
  total_str.toCharArray(total_char, 6);
  //issue - removing this stops code from working ?
  Serial.println ("Strings");
  Serial.println (flowRate_str);
  Serial.println (total_str);
  //output to LCD
  lcd.lcdGoToXY(12, 1);
  lcd.lcdWrite(flowRate_char);
  lcd.lcdGoToXY(12, 2);
  lcd.lcdWrite(total_char);
}


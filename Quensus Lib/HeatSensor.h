#ifndef Heatsensor_h
#define HeatSensor_h
#define ONE_WIRE_BUS 3
#include "Arduino.h"
#include "Sensor.h"
#include <OneWire.h>
#include <DallasTemperature.h>

/* DS18S20 Temperature chip i/o */
char buffer[25];
byte maxsensors = 0;
OneWire  ds(ONE_WIRE_BUS);
DallasTemperature sensors(&ds);

class HeatSensor: public Sensor{
public:
  HeatSensor(){};
  void measure(){
    sensors.requestTemperatures();
    //for each output data
    value = sensors.getTempCByIndex(0);
  };
  void setUp(){
    scanSensors();
  };
private:
  void scanSensors(){
    //deals with the heat sensor
    byte i;
    byte present = 0;
    byte data[12];
    byte addr[8];
    //creating heat sensors
    sensors.begin();
    while (ds.search(addr)){
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
      delay(1000);              // maybe 750ms is enough, maybe not
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
  };
};
#endif

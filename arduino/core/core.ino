// Core program
#include <Arduino.h>
#include <SoftwareSerial.h>
#include "WiFly.h"

#define SSID      "monssid"
#define KEY       "macle"
#define AUTH      WIFLY_AUTH_WPA1

SoftwareSerial myGPSSerial(7, 6); // RX, TX
SoftwareSerial myWIFISerial(5, 4);
WiFly wifly(&myWIFISerial);

void setup() {
  // put your setup code here, to run once
  
  myGPSSerial.begin(9600);
  myWIFISerial.begin(9600);
  Serial.begin(9600);
  
  delay(3000);
  wifly.reset();
  
  
}

void loop() {
  // Lecture des donnees GPS

  // 
  
}

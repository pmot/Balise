// Core program
#include <Arduino.h>
#include <SoftwareSerial.h>
#include <WiFly.h>

#include "parameters.h"
#include "wifi_scan_ap.h"

SoftwareSerial wifiSerial(WIFI_RX, WIFI_TX);
WiFly wifly(&wifiSerial);
struct apEntry* apList;
int nbAP = 0;

// the setup function runs once when you press reset or power the board
void setup() {
  // initialize digital pin 13 as an output.
  pinMode(13, OUTPUT);
}

// the loop function runs over and over again forever
void loop() {
  digitalWrite(13, HIGH);   // turn the LED on (HIGH is the voltage level)
  delay(1000);              // wait for a second
  digitalWrite(13, LOW);    // turn the LED off by making the voltage LOW
  delay(1000);              // wait for a second
  nbAP = wifiScanAp(&apList, wifly);
  // if (nbAP > 0)
  // {
  // Premier SSID : apList[0].ssid (une chaine)
  // - RSSI : apList[0].rssi (c'est un int...)
  // - MAC : apList[0].mac (une chaine)
  // }
}

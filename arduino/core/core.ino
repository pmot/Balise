// Core program
#include <Arduino.h>
#include <SoftwareSerial.h>
#include <WiFly.h>

#include "wifi_scan_ap.h"

// the setup function runs once when you press reset or power the board
void setup() {
  bool test_ap_lib = false;
  // initialize digital pin 13 as an output.
  pinMode(13, OUTPUT);
  test_ap_lib = dummy();
}

// the loop function runs over and over again forever
void loop() {
  digitalWrite(13, HIGH);   // turn the LED on (HIGH is the voltage level)
  delay(1000);              // wait for a second
  digitalWrite(13, LOW);    // turn the LED off by making the voltage LOW
  delay(1000);              // wait for a second
}

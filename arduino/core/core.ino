// Core program
#include <Arduino.h>
#include <SoftwareSerial.h>
#include <TinyGPS.h>
#include <Wire.h>

#include "core_parameters.h"

// GPS
#include "gps.h"
#include "cellular.h"

// WIFI
#include <WiFly.h>
#include "wifi_scan_ap.h"
SoftwareSerial wifiSerial(WIFI_RX, WIFI_TX);
WiFly wifly(&wifiSerial);
struct apEntry* apList;
int nbAP = 0;

// ACCEL
#include <LIS331.h>
#include "accelerometer.h"
LIS331 lis;

// Console
SoftwareSerial consoleSerial(CONSOLE_RX, CONSOLE_TX);

// GPS
SoftwareSerial gpsSerial(GPS_RX, GPS_TX);
TinyGPS gps;

// the setup function runs once when you press reset or power the board
void setup() {
  // initialize digital pin 13 as an output.
  pinMode(13, OUTPUT);
  // allumage de la LED le temps de l'initialisation
  digitalWrite(13, HIGH);
  
  // Initialisation des lignes serial, i2c
  consoleSerial.begin(9600);
  consoleSerial.println("INIT : Begin");
  gpsSerial.begin(9600);
  Wire.begin();

  // Initialisation du Wifi
  wifiSerial.begin(9600);
  wifly.reset();

  // Accéléromètre
  accelerometerSetup(lis);
  attachInterrupt(0, movment, RISING);

  // extinction de la LED à la fin de l'initialisation
  // on pourrait la faire clignoter en cas d'erreur durant cette phase
  digitalWrite(13, LOW);
  delay(3000);
  consoleSerial.println("INIT : Done");
}

// the loop function runs over and over again forever
void loop() {

  float flat, flon, speed;
  unsigned long age;
  
  while(1)
  {
    // Acquisition GPS
    gpsSerial.listen();
	gpsRead(gps, gpsSerial, 1000);
	gps.f_get_position(&flat, &flon, &age);
	speed = gps.f_speed_kmph();
	
	// Scan WIFI
	wifiSerial.listen();
	nbAP = wifiScanAp(&apList, wifly);

	// if (nbAP > 0)
	// {
	// Premier SSID : apList[0].ssid (une chaine)
	// - RSSI : apList[0].rssi (c'est un int...)
	// - MAC : apList[0].mac (une chaine)
	// }
  }
}

void movment()
{
  sei();
  int16_t y;
  lis.getYValue(&y);
  cli();

  Serial.print("Y Value: ");
  Serial.print(y);
  Serial.println(" milli Gs");
}

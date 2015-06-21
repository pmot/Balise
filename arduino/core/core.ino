// Core program
#include <Arduino.h>
#include <SoftwareSerial.h>
#include <WiFly.h>
#include <TinyGPS.h>
#include <LIS331.h>
#include <Wire.h>

#include "parameters.h"
#include "wifi_scan_ap.h"
#include "gps.h"

// Accel
LIS331 lis;

// Wifi
SoftwareSerial wifiSerial(WIFI_RX, WIFI_TX);
WiFly wifly(&wifiSerial);
struct apEntry* apList;
int nbAP = 0;

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
  wifiSerial.begin(9600);
  gpsSerial.begin(9600);
  Wire.begin();
  
  // Initialisation du Wifi
  wifly.reset();

  // Accéléromètre
  lis.setPowerStatus(LR_POWER_NORM);
  lis.setYEnable(true);
  // Registres
  lis.writeReg(0x30, 0x08);
  lis.writeReg(0x32, 0x01);
  lis.writeReg(0x33, 0x06);
  // Interruption
  attachInterrupt(0, movment, CHANGE);
  
  // extinction de la LED à la fin de l'initialisation
  // on pourrait la faire clignoter en cas d'erreur durant cette phase
  digitalWrite(13, LOW);
  delay(3000);
}

// the loop function runs over and over again forever
void loop() {

  float flat, flon, speed;
  unsigned long age;
  
  while(1)
  {
    // Acquisition GPS
	gpsRead(gps, gpsSerial, 1000);
	gps.f_get_position(&flat, &flon, &age);
	speed = gps.f_speed_kmph();
	
	// Scan WIFI
	nbAP = wifiScanAp(&apList, wifly);

#ifdef DEBUG_TO_CONSOLE
	// if (nbAP > 0)
	// {
	// Premier SSID : apList[0].ssid (une chaine)
	// - RSSI : apList[0].rssi (c'est un int...)
	// - MAC : apList[0].mac (une chaine)
	// }
#endif

  }
}

void movment()
{
  sei();
  int16_t y;
  lis.getYValue(&y);
  cli();
#ifdef DEBUG_TO_CONSOLE
  Serial.print("Y Value: ");
  Serial.print(y);
  Serial.println(" milli Gs");
#endif
}

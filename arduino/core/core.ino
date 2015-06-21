// Core program
#include <Arduino.h>
#include <SoftwareSerial.h>
#include <WiFly.h>

#include "parameters.h"
#include "wifi_scan_ap.h"
#include "gps.h"

SoftwareSerial wifiSerial(WIFI_RX, WIFI_TX);
WiFly wifly(&wifiSerial);
struct apEntry* apList;
int nbAP = 0;

// the setup function runs once when you press reset or power the board
void setup() {
  // initialize digital pin 13 as an output.
  pinMode(13, OUTPUT);
  // allumage de la LED le temps de l'initialisation
  digitalWrite(13, HIGH);
  
  
  // extinction de la LED à la fin de l'initialisation
  // on pourrait la faire clignoter en cas d'erreur durant cette phase
  digitalWrite(13, LOW);
}

// the loop function runs over and over again forever
void loop() {
  while(1)
  {
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

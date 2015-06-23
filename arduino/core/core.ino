// Core program

#define DEBUG_TO_CONSOLE

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
  struct apEntry* apList;				// Liste des AP
  int nbAP = 0;							// Nombre d'AP
  unsigned long nextWifiScanMillis;		// timestamp du prochain scan
  unsigned long nextWifiScanGetResultMillis;			// timestamp de la lecture du scan en cours
  unsigned long nextGPSReadMillis;		// timestamp de la prochaine lecture des coordonnées GPS

  int16_t y;	// Accélération en mg selon l'axe y
  
  float flat, flon, speed;
  unsigned long age;
  
  wifiScanSetup(wifly);
  nextWifiScanMillis = millis();
  nextWifiScanGetResultMillis = 0;
  nextGPSReadMillis = millis();
  
  while(1)
  {

    // La lecture de l'accéléromètre est prioritaire
    lis.getYValue(&y);
#ifdef DEBUG_TO_CONSOLE
    consoleSerial.print("ACCEL - Axe Y : ");
    consoleSerial.print(y);
    consoleSerial.println(" milli Gs");
#endif

	// Acquisition GPS
	if (millis() >= nextGPSReadMillis)
	{
      nextGPSReadMillis = millis() + GPS_READ_DELAY;
      gpsSerial.listen();
      gpsRead(gps, gpsSerial, GPS_READ_TIME); // On lit les données pend GPS_TIME ms
      gps.f_get_position(&flat, &flon, &age);
      speed = gps.f_speed_kmph();
#ifdef DEBUG_TO_CONSOLE
	  consoleSerial.println(flat);
	  consoleSerial.println(flon);
	  consoleSerial.println(speed);
#endif
	}
	
	// Scan WIFI et on rend la main
	if ((nextWifiScanGetResultMillis == 0) && (millis() >= nextWifiScanMillis))
	{
	  nextWifiScanMillis = millis() + WIFI_SCAN_DELAY;
	  if (wifiScanAp(wifly))
	  {
		nextWifiScanGetResultMillis = millis() + WIFI_SCAN_TIME;
	  }
	  else nextWifiScanGetResultMillis = 0;
	}
	// Scan WIFI on reprend la main pour la lecture du résultat
	else
	{
	  if (millis() >= nextWifiScanMillis)
	  {
	    nextWifiScanGetResultMillis = 0;
	    wifiSerial.listen();
	    nbAP = wifiScanApGetResult(&apList, wifly); // Le scan prend 3s par défaut
#ifdef DEBUG_TO_CONSOLE
	    if (nbAP > 0)
	    {
		  consoleSerial.print("WIFI - AP trouvées : ");
		  consoleSerial.println(nbAP);
		  for (int ap=0; ap < nbAP; ap++)
		  {
		    consoleSerial.print("WIFI - SSID : ");
		    consoleSerial.println(apList[ap].ssid);
		    consoleSerial.print("WIFI - MAC : ");
		    consoleSerial.println(apList[ap].mac);
		    consoleSerial.print("WIFI - RSSI : ");
		    consoleSerial.println(apList[ap].rssi);	
	      }
	    }
	    else consoleSerial.println("WIFI - Aucune AP trouvée");
#endif
      }
	  if ((nbAP > 0) && apList) free(apList);
	}
  }
}

void movment()
{
  sei();
  int16_t y;
  lis.getYValue(&y);
  cli();
#ifdef DEBUG_TO_CONSOLE
  consoleSerial.print("ACCEL (INTR) - Axe Y : ");
  consoleSerial.print(y);
  consoleSerial.println(" milli Gs");
#endif
}

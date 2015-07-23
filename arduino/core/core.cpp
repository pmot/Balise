// Core program

// #define DEBUG_ACCEL_TO_CONSOLE
#define DEBUG_TO_CONSOLE
// #define DEV_MODE

#include <avr/pgmspace.h>
#include <Arduino.h>
#include <stdlib.h>
#include <SoftwareSerial.h>
#include <TinyGPS.h>
#include <Wire.h>

#include "core.h"

// Console
SoftwareSerial consoleSerial(CONSOLE_RX, CONSOLE_TX);

// GSM
#include "GSMM95.h"
GSMM95 myGSM(GSM_PWRK, &consoleSerial);

// GPS
#include "gps.h"

// WIFI
#include <WiFly.h>
#include "wifi_scan_ap.h"

SoftwareSerial wifiSerial(WIFI_RX, WIFI_TX);
WiFly wifly(&wifiSerial);
bool wifiScanEnabled = true;
apEntry tabSSIDScan[NB_SSID_SCAN];

// ACCEL
#include <LIS331.h>
#include "accelerometer.h"
LIS331 lis;

// GPS
SoftwareSerial gpsSerial(GPS_RX, GPS_TX);
TinyGPS gps;

// the setup function runs once when you press reset or power the board
void setup() {
	// initialize digital pin 13 as an output.
	pinMode(13, OUTPUT);
	// allumer la LED le temps de l'initialisation
	digitalWrite(13, HIGH);

	// Initialisation des lignes serial, i2c
	consoleSerial.begin(9600);
	consoleSerial.println(F("INIT : Begin"));
#ifdef DEV_MODE
	consoleSerial.println(F("DEV MODE, vous avez 10s pour lancer un téléversement"));
	delay(10000);
#endif
	gpsSerial.begin(9600);
	Wire.begin();

	// Initialisation du Wifi
	wifiSerial.begin(9600);
	wifly.reset();

	// Accéléromètre
	accelerometerSetup(lis);

	// GSM
	myGSM.Init(pinCode);
	
	// extinction de la LED à la fin de l'initialisation
	// on pourrait la faire clignoter en cas d'erreur durant cette phase
	digitalWrite(13, LOW);
	delay(3000);

	// setup wifiScan
	if (wifiScanEnabled=wifiScanSetup(wifly))
	{
		consoleSerial.println(F("WIFI SCAN SETUP : OK"));
	}
	else
	{
		consoleSerial.println(F("WIFI SCAN SETUP : NOT OK"));
	}
	consoleSerial.println(F("INIT : Done"));
}

// the loop function runs over and over again forever
void loop() {
	char dataToSend[180] =	"";		// Au pif
		
	uint8_t nbAP = 0;				// Nombre d'AP WIFI
	unsigned long nextSendToGround;	// timestamp du prochain envoi des données au sol
	unsigned long nextWifiScan;		// timestamp du prochain scan WIFI
	unsigned long nextWifiScanRes;	// timestamp de la lecture du scan WIFI en cours
	unsigned long nextGPSRead;		// timestamp de la prochaine lecture des coordonnées GPS
#ifdef ORD_RT
	unsigned long tickRef;			// timestamp de boucle, sert à calculer les autres timestamps
	unsigned long counter=0;		// pour recaler les tâches (à définir, peut être un int)
#endif
	int16_t y;	// Accélération en mg selon l'axe y

	struct gpsData myGpsData;
	bool gpsDataIsInvalid = true;		// GDTREM : A revoir, completement tordu
	bool newGpsDataAvailable = false;	// La méthode d'acquisition des données doit renvoyer true si valide
	gpsSetup(&myGpsData);

	nextSendToGround = millis();	// Doit avoir lieu un jour
	nextWifiScan = millis();		// Doit avoir lieu un jour
	nextWifiScanRes = 0;			// A la première itération on n'attend pas de résultat de scan
	nextGPSRead = millis();			// Doit avoir lieu un jour
	
	myGSM.Connect(gprsAPN, gprsLogin, gprsPassword);
	myGSM.SendHttpReq(server, port, (char*)urlInit);
	// myGSM.Disconnect();

	// Main loop
	while(1) {
		
		
#ifdef ORD_RT
		// Init de la référence de temps de l'élection d'une tâche.
		// Le calcul de la prochaine éléction d'une tâche est déléguée à la tâche
		// Implique : si la durée de l'ensemble des tâches est supérieure au délai
		// le plus court entre deux éléctions d'une tâche, alors une élection pour
		// cette sera reportée d'un cycle.
		tickRef = millis();
		if (counter > 1000) counter = 0;	// Faire ça plus intelligemment
		else counter++;
#endif

		// La lecture de l'accéléromètre est prioritaire
		lis.getYValue(&y);
#ifdef DEBUG_ACCEL_TO_CONSOLE
		consoleSerial.print(F("ACCEL - Axe Y : "));
		consoleSerial.print(y);
		consoleSerial.println(F(" milli Gs"));
#endif

		// Envoi des données au sol
		// Acquisition GPS
		if (itsTimeFor(nextSendToGround)) {
#ifdef ORD_RT
		  nextGPSRead = tickRef + SEND_TO_GROUND_DELAY;
#else
		  nextGPSRead = millis() + SEND_TO_GROUND_DELAY;
#endif
		  if(newGpsDataAvailable)
		  {
#ifdef DEBUG_TO_CONSOLE
			consoleSerial.println(F("GSM - Nouvelles données GPS disponibles"));
#endif
			// Envoyer les données GPS
			if(myGSM.Status())
			{
			  sprintf(dataToSend, "%s%s,%s,%s,%s,%s,%s,%s,%s,%s,%s",		// GDTREM : Meme pas honte
			    urlGpsWS,
				myGpsData.altitude,
				myGpsData.longitude,
				myGpsData.altitude,
				myGpsData.speed,
				myGpsData.satellites,
				myGpsData.hdop,
				myGpsData.fixAge,
				myGpsData.date,
				myGpsData.time,
				myGpsData.dateAge
			  );
			  if(myGSM.SendHttpReq(server, port, dataToSend))
			  {
			    // Donnnées envoyées
			    newGpsDataAvailable = false;
			  }
			}
			// Else, newGpsDataAvailable toujours vrai... (en mémoire, en attente)
		  }
		  else
		  {
			// Quelquechose à faire ?
#ifdef DEBUG_TO_CONSOLE
			consoleSerial.println(F("GSM - Pas de nouvelle donnée GPS disponible"));
			printGpsData(myGpsData, consoleSerial);
#endif
		  }
		}

		// Acquisition GPS
		if (itsTimeFor(nextGPSRead)) {
#ifdef ORD_RT
		  nextGPSRead = tickRef + GPS_READ_DELAY;
#else
		  nextGPSRead = millis() + GPS_READ_DELAY;
#endif
		  gpsSerial.listen();
		  gpsRead(gps, gpsSerial, GPS_READ_TIME); // On lit les données pendant GPS_TIME ms
		  if(gpsDataIsInvalid = setGpsData(gps, &myGpsData))
		  {
			newGpsDataAvailable |= false;		// Il y a peut être une acquisition antérieure, non envoyée
#ifdef DEBUG_TO_CONSOLE
			consoleSerial.println(F("GPS - Data not valid, last data : "));
			printGpsData(myGpsData, consoleSerial);
#endif
		  }
		  else
		  {
			newGpsDataAvailable = true;
#ifdef DEBUG_TO_CONSOLE
			consoleSerial.println(F("GPS - Data valid, new data : "));
			printGpsData(myGpsData, consoleSerial);
#endif
		  }
		}

		if (wifiScanEnabled) {
			// On n'attend pas de résultat de Scan WIFI, on lance le scan et on rend la main
			// sans attendre le résultat
			if ( nextWifiScanRes == 0 && itsTimeFor(nextWifiScan)) {
				// Programmation du prochain scan
#ifdef ORD_RT
				nextWifiScan = tickRef + WIFI_SCAN_DELAY;
#else
				nextWifiScan = millis() + WIFI_SCAN_DELAY;
#endif
				// Scan !
				if (wifiScanAp(wifly)) {
					// Si la commande scan réussi, programmation de la lecture du résultat
					// au bout WIFI_SCAN_TIME ms
#ifdef ORD_RT
					nextWifiScanRes = tickRef + WIFI_SCAN_TIME;
#else
					nextWifiScanRes = millis() + WIFI_SCAN_TIME;
#endif
				}
				// Si la commande échoue, pas de programmation de la lecture du résultat
				else
					nextWifiScanRes = 0;
			}
			else { // Sinon lecture du résultat
				if (nextWifiScanRes && itsTimeFor(nextWifiScanRes)) {

					nextWifiScanRes = 0;

					wifiSerial.listen();

					nbAP = wifiScanApGetResult(tabSSIDScan, wifly); // Le scan prend 3s par défaut
#ifdef DEBUG_TO_CONSOLE
					if (nbAP > 0) {
						consoleSerial.print(F("WIFI - AP trouvées : "));
						consoleSerial.println(nbAP);
						for (int ap=0; ap < nbAP; ap++) {
							consoleSerial.print(F("WIFI - SSID : "));
							consoleSerial.println(tabSSIDScan[ap].ssid);
							consoleSerial.print(F("WIFI - MAC : "));
							consoleSerial.println(tabSSIDScan[ap].mac);
							consoleSerial.print(F("WIFI - RSSI : "));
							consoleSerial.println(tabSSIDScan[ap].rssi);
						}
					}
					else consoleSerial.println(F("WIFI - Aucune AP trouvée"));
#endif
				}
			}
		} // wifiScanEnabled
		
	} // End while (Main loop)
}

static bool itsTimeFor(unsigned long ts) {
	return (millis() >= ts);
}

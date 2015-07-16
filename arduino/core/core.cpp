// Core program

#define DEBUG_TO_CONSOLE

#include <avr/pgmspace.h>
#include <Arduino.h>
#include <stdlib.h>
#include <SoftwareSerial.h>
#include <TinyGPS.h>
#include <Wire.h>

#include "core.h"

// GSM
#include "GSMM95.h"
GSMM95 myGSM;

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

	// GSM
	myGSM.begin();
	myGSM.init("toto");
	
	// extinction de la LED à la fin de l'initialisation
	// on pourrait la faire clignoter en cas d'erreur durant cette phase
	digitalWrite(13, LOW);
	delay(3000);

	// setup wifiScan
<<<<<<< HEAD
	if (wifiScanSetup(wifly))
		consoleSerial.println("WIFI SCAN SETUP : OK");
	else {
		consoleSerial.println("WIFI SCAN SETUP : NOT OK");
		wifiScanEnabled = false;
=======
	if (wifiScanEnabled=wifiScanSetup(wifly))
	{
		consoleSerial.println(F("WIFI SCAN SETUP : OK"));
	}
	else
	{
		consoleSerial.println(F("WIFI SCAN SETUP : NOT OK"));
>>>>>>> 30559aef0eb17bf448ef75eba8ba2b17532352a2
	}
	consoleSerial.println(F("INIT : Done"));
}

// the loop function runs over and over again forever
void loop() {
	uint8_t nbAP = 0;				// Nombre d'AP WIFI
	unsigned long nextWifiScan;		// timestamp du prochain scan WIFI
	unsigned long nextWifiScanRes;	// timestamp de la lecture du scan WIFI en cours
	unsigned long nextGPSRead;		// timestamp de la prochaine lecture des coordonnées GPS

	int16_t y;	// Accélération en mg selon l'axe y

	struct gpsData myGpsData;
	bool gpsDataIsInvalid = true;
	gpsSetup(&myGpsData);

	nextWifiScan = millis();
	nextWifiScanRes = 0;
	nextGPSRead = millis();
	
	myGSM.dataConnect(apnName, GPRS_LOGIN, GPRS_PASSWORD);
	myGSM.sendHttpReq("monserveur.fr", "balise");

	while(1) {

		// La lecture de l'accéléromètre est prioritaire
		lis.getYValue(&y);
#ifdef DEBUG_TO_CONSOLE
		/* consoleSerial.print("ACCEL - Axe Y : ");
		consoleSerial.print(y);
		consoleSerial.println(" milli Gs");*/
#endif

		// Acquisition GPS
		if (itsTimeFor(nextGPSRead)) {
			nextGPSRead = millis() + GPS_READ_DELAY;
			gpsSerial.listen();
			gpsRead(gps, gpsSerial, GPS_READ_TIME); // On lit les données pend GPS_TIME ms
			gpsDataIsInvalid = setGpsData(gps, &myGpsData);
#ifdef DEBUG_TO_CONSOLE
			if (gpsDataIsInvalid)
			{
				consoleSerial.println("GPS - Data not valid, last data : ");
			}
			else
			{
				consoleSerial.println("GPS - Data valid, new data : ");
			}
			printGpsData(myGpsData, consoleSerial);
#endif
		}

		if (wifiScanEnabled) {
			// On n'attend pas de résultat de Scan WIFI, on lance le scan et on rend la main
			// sans attendre le résultat
			if ( nextWifiScanRes == 0 && itsTimeFor(nextWifiScan)) {
				// Programmation du prochain scan
				nextWifiScan = millis() + WIFI_SCAN_DELAY;
				// Scan !
				if (wifiScanAp(wifly)) {
					// Si la commande scan réussi, programmation de la lecture du résultat
					// au bout WIFI_SCAN_TIME ms
					nextWifiScanRes = millis() + WIFI_SCAN_TIME;
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
						consoleSerial.print("WIFI - AP trouvées : ");
						consoleSerial.println(nbAP);
						for (int ap=0; ap < nbAP; ap++) {
							consoleSerial.print("WIFI - SSID : ");
							consoleSerial.println(tabSSIDScan[ap].ssid);
							consoleSerial.print("WIFI - MAC : ");
							consoleSerial.println(tabSSIDScan[ap].mac);
							consoleSerial.print("WIFI - RSSI : ");
							consoleSerial.println(tabSSIDScan[ap].rssi);
						}
					}
					else consoleSerial.println("WIFI - Aucune AP trouvée");
#endif
				}
			}
		} // wifiScanEnabled
	}
}

<<<<<<< HEAD
void movment() {
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

bool itsTimeFor(unsigned long ts) {
=======
static bool itsTimeFor(unsigned long ts) {
>>>>>>> 30559aef0eb17bf448ef75eba8ba2b17532352a2
	return (millis() >= ts);
}


// Common
#include <avr/pgmspace.h>
#include <Arduino.h>
#include <stdlib.h>
#include <SoftwareSerial.h>

#include "core.h"

// GPS
#include <TinyGPS.h>
#include "gps.h"
// I2C
#include <Wire.h>

// WIFI
#include <WiFly.h>
#include "wifi_scan_ap.h"

// ACCELEROMETRE
#include <LIS331.h>
#include "accelerometer.h"
LIS331 lis;

// Console
SoftwareSerial consoleSerial(CONSOLE_RX, CONSOLE_TX);
// GPS
SoftwareSerial gpsSerial(GPS_RX, GPS_TX);
TinyGPS gps;
struct gpsData myGpsData;
// ACCELEROMETRE
LIS331 lis;

void setup() {


	PRINT_LOG(LOG_INFO ,F("SETUP begin"));


	////////////////////////
	// LED
	////////////////////////
	pinMode(LED_PIN, OUTPUT);

	////////////////////////
	// On allume la LED le temps de l'intialisation
	////////////////////////
	digitalWrite(LED_PIN, HIGH);


	////////////////////////
	// CONSOLE
	////////////////////////
	consoleSerial.begin(9600);


	////////////////////////
	// GPS
	////////////////////////
	PRINT_LOG(LOG_INFO ,F("\tGPS begin"));
	gpsSerial.begin(9600);
	gpsSetup(&myGpsData);
	PRINT_LOG(LOG_INFO ,F("\tGPS end"));

	////////////////////////
	// Accéléromètre
	////////////////////////
	accelerometerSetup(lis);
	

	////////////////////////
	// Fin de l'initialisation
	// on éteint la LED
	////////////////////////
	digitalWrite(LED_PIN, LOW);

	PRINT_LOG(LOG_INFO ,F("SETUP end"));
}


boolean i2c_receive;

void loop () {
	boolean stop=false;
	short vitesse=1;

	
	////////////////////////////////
	// Boucle principale
	// En cas de sortie de la boucle
	// il faut prévoir un redémarrage
	////////////////////////////////
	while(!stop) {

		if(vitesse) {
			float temp1;
			gpsRead(&gps, gpsSerial, GPS_READ_TIME);
			sscanf(gps.speed,"%f.0",&temp1);
			vitesse = (short) temp1;
		}
		//
		// La vitesse est nulle
		// on surveille l'accéleromètre
		// impossible de mettre une interruption sur les PIN 4 & 5
		//
		else {
			attachInterrupt(/* SDA */ , I2CReceive(), FALLING);


			if(i2c_receive) {
				lis.getYValue(&y);
			}
		}


		
		
		if (gpsSetData(gps, &myGpsData)) {
			printGpsData(&myGpsData);
		}
		




	}
	

	//
	// redemarrage (connecter une broche sur le RST)
	//
}




void printGpsData(struct gpsData *pGpsData)
{
	consoleSerial.listen();
	PRINT_LOG(LOG_INFO, F("GPS"));
	// Latitude
	PRINT_LOG(LOG_INFO, F("** LATITUDE: "));PRINT_LOG(LOG_INFO,pGpsData->latitude);
	// Longitude
	PRINT_LOG(LOG_INFO, F("** LONGITUDE: "));PRINT_LOG(LOG_INFO,pGpsData->longitude);
	// Altitude
	PRINT_LOG(LOG_INFO, F("** ALTITUDE: "));PRINT_LOG(LOG_INFO,pGpsData->altitude);
	// Speed
	PRINT_LOG(LOG_INFO, F("** SPEED: "));PRINT_LOG(LOG_INFO,pGpsData->speed);
	// Fix Age
	PRINT_LOG(LOG_INFO, F("** Fix Age: "));PRINT_LOG(LOG_INFO,pGpsData->fixAge);
	// HDOP
	PRINT_LOG(LOG_INFO, F("** HDOP: "));PRINT_LOG(LOG_INFO,pGpsData->hdop);
	// Satellites
	PRINT_LOG(LOG_INFO, F("** Nb SATS: "));PRINT_LOG(LOG_INFO,pGpsData->satellites);
	// Date
	PRINT_LOG(LOG_INFO, F("** Date: "));PRINT_LOG(LOG_INFO,pGpsData->date);
	// Time
	PRINT_LOG(LOG_INFO, F("** Time: "));PRINT_LOG(LOG_INFO,pGpsData->time);
	// Date Age
	PRINT_LOG(LOG_INFO, F("** Date Age: "));PRINT_LOG(LOG_INFO,pGpsData->dateAge);

}



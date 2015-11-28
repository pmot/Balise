
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

// Console
SoftwareSerial consoleSerial(CONSOLE_RX, CONSOLE_TX);
// GPS
SoftwareSerial gpsSerial(GPS_RX, GPS_TX);
TinyGPS gps;
struct gpsData myGpsData;
// ACCELEROMETRE
LIS331 lis;



void setup() {


	////////////////////////
	// CONSOLE
	////////////////////////
	consoleSerial.begin(115200);
	PRINT_LOG(LOG_INFO ,F(""));
	PRINT_LOG(LOG_INFO ,F("#########################"));
	PRINT_LOG(LOG_INFO ,F("BEGIN"));


	////////////////////////
	// LED
	////////////////////////
	pinMode(LED_PIN, OUTPUT);

	////////////////////////
	// On allume la LED le temps de l'intialisation
	////////////////////////
	digitalWrite(LED_PIN, HIGH);

#ifdef GPS_ACTIF
	////////////////////////
	// GPS
	////////////////////////
	PRINT_LOG(LOG_INFO ,F("\tGPS begin"));
	gpsSerial.begin(9600);
	gpsSetup(&myGpsData);
	PRINT_LOG(LOG_INFO ,F("\tGPS end"));
#endif

#ifdef ACCEL_ACTIF
	////////////////////////
	// Accéléromètre
	////////////////////////
	PRINT_LOG(LOG_INFO ,F("\tACCEL begin"));

	Wire.begin();

	if(!accelerometerSetup(&lis)) {
		PRINT_LOG(LOG_ERROR ,F("\tACCEL init error"));
	}
	PRINT_LOG(LOG_INFO ,F("\tACCEL end"));
	attachInterrupt( digitalPinToInterrupt(ACCEL_INT), I2CReceived, FALLING);

#endif


	////////////////////////
	// Fin de l'initialisation
	// on éteint la LED
	////////////////////////
	digitalWrite(LED_PIN, LOW);

	PRINT_LOG(LOG_INFO ,F("END"));
}


bool i2c_receive=false;

void loop () {
	boolean stop=false;
	short vitesse=0;
	byte i=0;
	
	////////////////////////////////
	// Boucle principale
	// En cas de sortie de la boucle
	// il faut prévoir un redémarrage
	////////////////////////////////
	while(!stop) {

		if(vitesse) {
			//
			// Créer une fonction
			// pour libérer la pile
			//
#ifdef GPS_ACTIF
			gpsRead(&gps, gpsSerial, GPS_READ_TIME);
			vitesse = (short) gps.speed();
#endif
		}
		//
		// La vitesse est nulle
		// on surveille l'accéleromètre
		// impossible de mettre une interruption sur les PIN 4 & 5
		//
		else {
#ifdef ACCEL_ACTIF
			int16_t y;

			// attachInterrupt( 2 , I2CReceive, RISING);


			if(i2c_receive) {
				PRINT_LOG(LOG_TRACE ,F("\ti2c_receive=true"));
				//
				// Créer une fonction
				// pour libérer la pile
				//

				int16_t y;

				//while(lis.statusHasYDataAvailable()) {
				//	lis.getYValue(&y);
				//	PRINT_LOG(LOG_TRACE ,y);
				//}

				i2c_receive=false;
			}
#endif

		}
		// if (gpsSetData(gps, &myGpsData)) {
		//	printGpsData(&myGpsData);
		// }
		PRINT_LOG(LOG_TRACE ,i++)
		delay(1000);

	} // WHILE

	//
	// redemarrage (connecter une broche sur le RST)
	//
}

void  I2CReceived()
{
	PRINT_LOG(LOG_TRACE ,F("BEGIN"));
	i2c_receive=true;
	PRINT_LOG(LOG_TRACE ,F("END"));
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



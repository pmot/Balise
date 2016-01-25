
// Common
#include <avr/pgmspace.h>
#include <Arduino.h>
#include <stdlib.h>
#include <SoftwareSerial.h>


#define PRINT_LOG(y,x)  if(debug>=y) { consoleSerial.listen(); consoleSerial.print(__FUNCTION__); consoleSerial.print(F(": ")) ; consoleSerial.println(x); }

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

// GSM
#include "GSMM95.h"

#include "core.h"

// Console
SoftwareSerial consoleSerial(CONSOLE_RX, CONSOLE_TX);
// GPS
SoftwareSerial gpsSerial(GPS_RX, GPS_TX);
TinyGPS gps;

// ACCELEROMETRE
LIS331 lis;


// carte GSM
GSMM95 myGSM(GSM_PWRK, &consoleSerial);


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
	pinMode(ACCEL_INT, INPUT_PULLUP);
	attachInterrupt( digitalPinToInterrupt(ACCEL_INT), I2CReceived, RISING);

#endif

#ifdef GSM_ACTIF
	myGSM.Init(pinCode);
#endif

	////////////////////////
	// Fin de l'initialisation
	// on éteint la LED
	////////////////////////
	digitalWrite(LED_PIN, LOW);

	PRINT_LOG(LOG_INFO ,F("END"));
}


bool i2c_receive=false;
short vitesse=0;
byte memo_tempo=0;
bool first_loop=false;

void loop () {
	int16_t y;
	byte direction;
	byte tempo = (unsigned char) ((millis()/1000)%FREQUENCE_ENVOI_DEFAUT); // voir pour rendre paramétrable cette valeur
	bool envoi = tempo < memo_tempo ? true : false;
	memo_tempo = tempo;

	noInterrupts();

#ifdef GPS_ACTIF
	gpsRead(&gps, gpsSerial, GPS_READ_TIME);

	vitesse = (short) gps.speed();
#endif

	if(vitesse> LIMITE_VITESSE_ACCEL) {
		PRINT_LOG(LOG_TRACE ,F("La vitesse est suffisamment importante, on ne tient pas compte de l'accéléromètre"));

		direction=accelerometerDirection();
		if(first_loop==false) {
				PRINT_LOG(LOG_TRACE,F("Direction:"));
				PRINT_LOG(LOG_TRACE,direction);
		}
		first_loop=true;
	}
	//////////////////////////////////////////////////////////////////////////
	//
	// La vitesse est nulle
	// on surveille l'accéleromètre
	// impossible de mettre une interruption sur les PIN 4 & 5
	// Prendre le sens de circulation au démarrage avec la vitesse =0 à 4 km/h
	//
	//////////////////////////////////////////////////////////////////////////
	else {
#ifdef ACCEL_ACTIF
		if(first_loop) {
			//
			// On réinitialise le buffer mémorisant le sens de circulation au départ
			//
			accelerometerReset();
			first_loop=false;
		}

		interrupts();
		attachInterrupt( 2 , I2CReceived, RISING);


		if(i2c_receive == true) {
			PRINT_LOG(LOG_TRACE ,F("\ti2c_receive=true"));
			//
			// Créer une fonction
			// pour libérer la pile
			//

			lis.getYValue(&y);

			PRINT_LOG(LOG_TRACE ,y);

			if(y!=0) accelerometerStore(y>0 ? 1 : 0);

			i2c_receive=false;
		}
#endif
	}

	if(envoi) {
		PRINT_LOG(LOG_TRACE ,F("Envoi d'une position"));
#ifdef GSM_ACTIF

		if(!sendMessageLocalisation(&gps,direction)) {
			PRINT_LOG(LOG_ERROR ,F("Error in sendMessageLocalisation"));
		}

#endif
	}



	delay(1000);


	// if (gpsSetData(gps, &myGpsData)) {
	//	printGpsData(&myGpsData);
	// }

}

void  I2CReceived()
{
	// PRINT_LOG(LOG_TRACE ,F("BEGIN"));
	i2c_receive=true;
	// PRINT_LOG(LOG_TRACE ,F("END"));
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

byte sendMessageLocalisation(TinyGPS *myGps, byte direction) {
	char dataToSend[150];

#ifdef GSM_ACTIF
	if(myGSM.Status()) {
		PRINT_LOG(LOG_TRACE , F("GSM Status: OK"));

		if(myGSM.Connect(gprsAPN, gprsLogin, gprsPassword)) {
			PRINT_LOG(LOG_TRACE , F("GSM Connect: OK"));
			myGSM.SendHttpReq(server, port, "test");
		}
		else {
			PRINT_LOG(LOG_TRACE , F("GSM Connect: NON OK"));
		}

		//
		// on copie le début de la requete
		//
		strcpy(dataToSend,urlGpsWS);

		PRINT_LOG(LOG_TRACE ,F("Envoi d'une position"));
		 /*

		if (gpsToString(myGps, dataToSend+strlen(dataToSend))) {
			printGpsData(&myGpsData);
		}

		PRINT_LOG(LOG_TRACE ,dataToSend);

		if(!myGSM.SendHttpReq(server, port, dataToSend)) {
			return 0;
		}*/
	}
	else {
		PRINT_LOG(LOG_INFO, F("GSM Status: NON OK"));
	}
	// PRINT_LOG(LOG_INFO, myGSM.gsmBuf);

#endif
	return 1;
}


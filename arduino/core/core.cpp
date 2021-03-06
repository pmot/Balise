
// Common
#include <avr/pgmspace.h>
#include <Arduino.h>
#include <stdlib.h>
#include <SoftwareSerial.h>


#define PRINT_LOG(y,x)  if(debug>=y) { consoleSerial.listen(); consoleSerial.print(__FUNCTION__); consoleSerial.print(F(": ")) ; consoleSerial.println(x); }
#define LED_ON	digitalWrite(LED_PIN, HIGH);
#define LED_OFF	digitalWrite(LED_PIN, LOW);
#define LED_FLASH(x)	digitalWrite(LED_PIN, HIGH); delay(x); digitalWrite(LED_PIN, LOW);
#define LED_FLASH2(x)	LED_FLASH(x); delay(150); LED_FLASH(x);
#define LED_FLASH3(x)	LED_FLASH(x); delay(150); LED_FLASH(x); delay(150); LED_FLASH(x);


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
// GSM
struct gsmContext myGsmContext;
// WIFI
SoftwareSerial wifiSerial(WIFI_RX, WIFI_TX);
WiFly wifly(&wifiSerial);


void setup() {

	////////////////////////
	// CONSOLE
	////////////////////////
	consoleSerial.begin(115200);
	PRINT_LOG(LOG_INFO, F(""));
	PRINT_LOG(LOG_INFO, F("#########################"));
	PRINT_LOG(LOG_INFO, F("BEGIN"));

	////////////////////////
	// LED
	////////////////////////
	pinMode(LED_PIN, OUTPUT);

	////////////////////////
	// On allume la LED le temps de l'intialisation
	////////////////////////
	LED_ON

#ifdef GPS_ACTIF
	////////////////////////
	// GPS
	////////////////////////
	PRINT_LOG(LOG_INFO, F("\tGPS begin"));
	gpsSerial.begin(9600);
	PRINT_LOG(LOG_INFO, F("\tGPS end"));
#endif

#ifdef ACCEL_ACTIF
	////////////////////////
	// Accéléromètre
	////////////////////////
	PRINT_LOG(LOG_INFO, F("\tACCEL begin"));

	Wire.begin();

	if(!accelerometerSetup(&lis)) {
		PRINT_LOG(LOG_ERROR, F("\tACCEL init error"));
	}
	PRINT_LOG(LOG_INFO, F("\tACCEL end"));
	pinMode(ACCEL_INT, INPUT_PULLUP);
	attachInterrupt(digitalPinToInterrupt(ACCEL_INT), I2CReceived, RISING);
#endif

#ifdef GSM_ACTIF
	////////////////////////
	// 4 phases :
	// - Init du contexte GSM
	// - Hard Reset du modem
	// - Init du modem, y compris unlock de la SIM
	// - Connect GPRS
	////////////////////////
	PRINT_LOG(LOG_INFO, F("\tGSM begin"));
	gsmSetup(&myGsmContext, &consoleSerial);
	gsmHardReset(&myGsmContext, GSM_PWRK);
	gsmInit(&myGsmContext, pinCode);
	gsmGprsConnect(&myGsmContext, gprsAPN, gprsLogin, gprsPassword);
	PRINT_LOG(LOG_INFO, F("\tGSM end"));
#endif

#ifdef WIFI_ACTIF
	// Scan des SSID/MAC
	PRINT_LOG(LOG_INFO, F("WIFI begin"));
	wifiSerial.listen();
	wifiSetup(wifly);
	PRINT_LOG(LOG_INFO, F("WIFI end"));
#endif


	////////////////////////
	// Fin de l'initialisation
	// on éteint la LED
	////////////////////////
	LED_OFF

	PRINT_LOG(LOG_INFO, F("END"));
}


bool i2c_receive=false;
short vitesse=-1;
byte memo_tempo=0;
bool first_loop=false;

void loop () {
	int16_t y;
	byte direction;
	byte tempo = (unsigned char) ((millis()/1000)%FREQUENCE_ENVOI_DEFAUT); // voir pour rendre paramétrable cette valeur
	bool envoi = tempo < memo_tempo ? true : false;
	memo_tempo = tempo;

#ifdef GSM_ACTIF
	// Maintient de la connexion
	PRINT_LOG(LOG_TRACE, F("MAINTIENT CNX GPS BEGIN"));
	if (gsmNeedToConnect(&myGsmContext))
	{
		LED_FLASH2(100);
		gsmGprsConnect(&myGsmContext, gprsAPN, gprsLogin, gprsPassword);
	}
	PRINT_LOG(LOG_TRACE, F("MAINTIENT CNX GPS END"));
#endif

#ifdef GPS_ACTIF
	PRINT_LOG(LOG_TRACE, F("GPS READ BEGIN"));
	gpsRead(&gps, gpsSerial, GPS_READ_TIME);
	PRINT_LOG(LOG_TRACE, F("GPS READ END"));
	if (!gpsFix(&gps)) vitesse = -1;
	else vitesse = (short) gps.f_speed_kmph();
	PRINT_LOG(LOG_INFO, F("SPEED (km/h): "));
	PRINT_LOG(LOG_INFO, vitesse);
#endif

	if(vitesse> LIMITE_VITESSE_ACCEL) {
		PRINT_LOG(LOG_TRACE, F("La vitesse est suffisamment importante, on ne tient pas compte de l'accéléromètre"));

		direction=accelerometerDirection();
		if(first_loop == false) {
				PRINT_LOG(LOG_TRACE, F("Direction:"));
				PRINT_LOG(LOG_TRACE, direction);
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
			first_loop = false;
		}

		if(i2c_receive == true) {
			PRINT_LOG(LOG_TRACE, F("\ti2c_receive=true"));
			//
			// Créer une fonction
			// pour libérer la pile
			//

			lis.getYValue(&y);

			PRINT_LOG(LOG_TRACE, y);

			if(y!=0) accelerometerStore(y > 0 ? 1 : 0);

			i2c_receive=false;
		}
#endif
	}

	if(envoi) {
		PRINT_LOG(LOG_TRACE, F("Envoi d'une position"));
		if(!sendMessageLocalisation(&gps, direction)) {
			PRINT_LOG(LOG_ERROR, F("Erreur de sendMessageLocalisation"));
		}
	}

	delay(1000);

}

void  I2CReceived()
{
	// PRINT_LOG(LOG_TRACE ,F("BEGIN"));
	i2c_receive=true;
	// PRINT_LOG(LOG_TRACE ,F("END"));
}

byte sendMessageLocalisation(TinyGPS *pMyGps, byte direction) {
	char dataToSend[150];
	memset(dataToSend, 0, 150);

#ifdef WIFI_ACTIF
	// Scan des SSID/MAC
	PRINT_LOG(LOG_TRACE, F("WIFI SCAN"));
	wifiSerial.listen();
	wifiScan(wifly);
#endif

#ifdef GSM_ACTIF
	if(gpsToString(pMyGps, dataToSend)) {
		LED_FLASH(100);
		gsmHttpRequest(&myGsmContext, url, NUMENGIN, dataToSend);
	}
	else {
		LED_FLASH3(100);
		PRINT_LOG(LOG_INFO, F("No valid GPS data"));
	}
#endif

#ifdef WIFI_ACTIF
	// Resultats
	PRINT_LOG(LOG_TRACE, F("WIFI READ BEGIN"));
	memset(dataToSend, 0, 150);
	wifiSerial.listen();
	if (wifiGetResult(wifly, true, dataToSend) > 0) {
		PRINT_LOG(LOG_TRACE, dataToSend);
		wifiSerial.listen();
		while(wifiGetResult(wifly, true, dataToSend) > 0) {
			PRINT_LOG(LOG_TRACE, dataToSend);
			wifiSerial.listen();
		}
	}
	PRINT_LOG(LOG_TRACE, F("WIFI READ END"));
#endif


	return 1;
}


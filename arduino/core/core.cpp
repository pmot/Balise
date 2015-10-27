
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

// ACCEL
#include <LIS331.h>
#include "accelerometer.h"

// Console
SoftwareSerial consoleSerial(CONSOLE_RX, CONSOLE_TX);

SoftwareSerial gpsSerial(GPS_RX, GPS_TX);
TinyGPS gps;
struct gpsData myGpsData;

void setup() {
	// initialize digital pin 13 as an output.
	pinMode(LED_PIN, OUTPUT);
	// allumer la LED le temps de l'initialisation
	digitalWrite(LED_PIN, HIGH);

	consoleSerial.begin(9600);
	PRINT_LOG(LOG_INFO ,F("begin"));

	//
	// GPS
	//

	PRINT_LOG(LOG_INFO ,F("gps init"));

	gpsSerial.begin(9600);
	gpsSetup(&myGpsData);
	
	PRINT_LOG(LOG_INFO ,F("gps end"));
	
	digitalWrite(LED_PIN, LOW);

	PRINT_LOG(LOG_INFO ,F("end"));
}

void loop () {
	
	while(1) {
		
		gpsRead(&gps, gpsSerial, GPS_READ_TIME);
		
		if (gpsSetData(gps, &myGpsData)) {
			
			printGpsData(&myGpsData);
		}
		
	}
	
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



#include "gps.h"
#include <stdlib.h>

int gpsSetup(struct gpsData* myData)
{
	memset(myData->latitude, '\0', 10);
	memset(myData->longitude, '\0', 11);
	memset(myData->altitude, '\0', 8);
	memset(myData->speed, '\0', 7);
	memset(myData->satellites, '\0', 3);
	memset(myData->hdop, '\0', 5);
	memset(myData->date, '\0', 11);
	memset(myData->time, '\0', 9);
}

void gpsRead(TinyGPS gps, SoftwareSerial serial, unsigned long ms)
{
	serial.listen();

	unsigned long start = millis();

	do {
		while (serial.available())
			gps.encode(serial.read());
	} while (millis() - start < ms);
}

bool setGpsData(TinyGPS myGps, struct gpsData* myGpsData)
{
	float flat, flon, falt, fspeed;
	unsigned long uage, usat, uhdop;
	bool invalid = false;
	
	myGps.f_get_position(&flat, &flon, &uage);
	falt = myGps.f_altitude();
	fspeed = myGps.f_speed_kmph();
	uhdop = myGps.hdop();
	usat = myGps.satellites();
	
	invalid ^= (flat == TinyGPS::GPS_INVALID_F_ANGLE);
	invalid ^= (flon == TinyGPS::GPS_INVALID_F_ANGLE);
	invalid ^= (falt == TinyGPS::GPS_INVALID_F_ALTITUDE);
	invalid ^= (fspeed == TinyGPS::GPS_INVALID_F_SPEED);
	invalid ^= (uhdop == TinyGPS::GPS_INVALID_HDOP);
	invalid ^= (usat == TinyGPS::GPS_INVALID_SATELLITES);
	
	
	if (!invalid)
	{
		// On met à jour la structure gpsData
		// Conversion en chaînes de caractères
	}
	
	return invalid;
}

void printGpsData(struct gpsData myGpsData, SoftwareSerial mySerial)
{
	mySerial.println("GPS - start");
	mySerial.print("GPS - LATITUDE : ");
	mySerial.print("GPS - ");
	mySerial.println(myGpsData.latitude);
	// Etc...
	mySerial.println("GPS - stop");
}


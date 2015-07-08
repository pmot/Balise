#include "gps.h"
#include <stdlib.h>

int gpsSetup(struct gpsData* myData)
{
	memset(myData->latitude, '\0', 10);
	memset(myData->longitude, '\0', 11);
	memset(myData->altitude, '\0', 8);
	memset(myData->speed, '\0', 7);
	memset(myData->satellites, '\0', 4);
	memset(myData->hdop, '\0', 5);
	memset(myData->fixAge, '\0', 7);
	memset(myData->date, '\0', 11);
	memset(myData->time, '\0', 9);
	memset(myData->dateAge, '\0', 7);
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
	byte bmonth, bday, bhour, bminute, bsecond, bhundredths;
	int iyear;
	unsigned long ufixAge, usat, uhdop, udateAge;
	bool invalid = false;
	
	myGps.f_get_position(&flat, &flon, &ufixAge);
	falt = myGps.f_altitude();
	fspeed = myGps.f_speed_kmph();
	uhdop = myGps.hdop();
	usat = myGps.satellites();
	
	myGps.crack_datetime(&iyear, &bmonth, &bday,
			&bhour, &bminute, &bsecond, &bhundredths, &udateAge);

	
	invalid ^= (flat == TinyGPS::GPS_INVALID_F_ANGLE);
	invalid ^= (flon == TinyGPS::GPS_INVALID_F_ANGLE);
	invalid ^= (falt == TinyGPS::GPS_INVALID_F_ALTITUDE);
	invalid ^= (fspeed == TinyGPS::GPS_INVALID_F_SPEED);
	invalid ^= (uhdop == TinyGPS::GPS_INVALID_HDOP);
	invalid ^= (usat == TinyGPS::GPS_INVALID_SATELLITES);
	invalid ^= (ufixAge == TinyGPS::GPS_INVALID_AGE);
	invalid ^= (udateAge == TinyGPS::GPS_INVALID_AGE);

	if (!invalid)
	{
		// On remet tout à zéro
		gpsSetup(myGpsData);
		// On met à jour la structure gpsData
		// Conversion en chaînes de caractères
		dtostrf(flat, 8, 6, myGpsData->latitude);
		dtostrf(flon, 9, 6, myGpsData->longitude);
		dtostrf(falt, 6, 2, myGpsData->altitude);
		dtostrf(fspeed, 5, 2, myGpsData->speed);
		itoa(uhdop, myGpsData->hdop, 4);
		itoa(usat, myGpsData->satellites, 3);
		itoa(ufixAge, myGpsData->fixAge, 6);
		sprintf(myGpsData->date, "%02d/%02d/%02d", bmonth, bday, iyear);
		sprintf(myGpsData->time, "%02d:%02d:%02d", bhour, bminute, bsecond);
		itoa(udateAge, myGpsData->dateAge, 6);
	}
	
	return invalid;
}

void printGpsData(struct gpsData myGpsData, SoftwareSerial mySerial)
{
	mySerial.println("GPS - start");
	// Latitude
	mySerial.print("GPS - LATITUDE : ");
	mySerial.print("GPS - ");
	mySerial.println(myGpsData.latitude);
	// Longitude
	mySerial.print("GPS - LONGITUDE : ");
	mySerial.print("GPS - ");
	mySerial.println(myGpsData.longitude);
	// Altitude
	mySerial.print("GPS - ALTITUDE : ");
	mySerial.print("GPS - ");
	mySerial.println(myGpsData.altitude);
	// Speed
	mySerial.print("GPS - SPEED : ");
	mySerial.print("GPS - ");
	mySerial.println(myGpsData.speed);
	// Fix Age
	mySerial.print("GPS - Fix Age : ");
	mySerial.print("GPS - ");
	mySerial.println(myGpsData.fixAge);
	// HDOP
	mySerial.print("GPS - HDOP : ");
	mySerial.print("GPS - ");
	mySerial.println(myGpsData.hdop);
	// Satellites
	mySerial.print("GPS - SATS : ");
	mySerial.print("GPS - ");
	mySerial.println(myGpsData.satellites);
	// Date
	mySerial.print("GPS - Date : ");
	mySerial.print("GPS - ");
	mySerial.println(myGpsData.date);
	// Time
	mySerial.print("GPS - Time : ");
	mySerial.print("GPS - ");
	mySerial.println(myGpsData.time);
	// Date Age
	mySerial.print("GPS - Date Age : ");
	mySerial.print("GPS - ");
	mySerial.println(myGpsData.dateAge);

	mySerial.println("GPS - stop");
}


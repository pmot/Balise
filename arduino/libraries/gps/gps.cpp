#include "gps.h"
#include <stdlib.h>

int gpsSetup(struct gpsData* myData)
{
	memset (myData, '\0', sizeof(struct gpsData));
}

void gpsRead(TinyGPS gps, SoftwareSerial serial, unsigned long ms)
{
	serial.listen();

	unsigned long start = millis();

	do {
		while (serial.available())
			// gps.encode(serial.read());
			consoleSerial.write(serial.read());
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
	
	consoleSerial.println(usat);

	myGps.crack_datetime(&iyear, &bmonth, &bday,
			&bhour, &bminute, &bsecond, &bhundredths, &udateAge);


	consoleSerial.println(iyear);

	
	invalid |= (flat == TinyGPS::GPS_INVALID_F_ANGLE);
	invalid |= (flon == TinyGPS::GPS_INVALID_F_ANGLE);
	invalid |= (falt == TinyGPS::GPS_INVALID_F_ALTITUDE);
	invalid |= (fspeed == TinyGPS::GPS_INVALID_F_SPEED);
	invalid |= (uhdop == TinyGPS::GPS_INVALID_HDOP);
	invalid |= (usat == TinyGPS::GPS_INVALID_SATELLITES);
	invalid |= (ufixAge == TinyGPS::GPS_INVALID_AGE);
	invalid |= (udateAge == TinyGPS::GPS_INVALID_AGE);

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


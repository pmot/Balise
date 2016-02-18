#include "gps.h"
#include <stdlib.h>


void gpsRead(TinyGPS* pGps, SoftwareSerial mySerial, unsigned long ms)
{

	mySerial.listen();

	unsigned long start = millis();

	do {
		while (mySerial.available()) {
			pGps->encode(mySerial.read());
		}
	} while (millis() - start < ms);

}

bool gpsToString(TinyGPS *pMyGps, char* stringBuffer)
{
	float flat, flon, falt, fspeed;
	byte bmonth, bday, bhour, bminute, bsecond, bhundredths;
	int iyear;
	unsigned long ufixAge, usat, uhdop, udateAge;
	bool valid = true;

	pMyGps->f_get_position(&flat, &flon, &ufixAge);
	falt = pMyGps->f_altitude();
	fspeed = pMyGps->f_speed_kmph();
	uhdop = pMyGps->hdop();
	usat = pMyGps->satellites();

	pMyGps->crack_datetime(&iyear, &bmonth, &bday,
			&bhour, &bminute, &bsecond, &bhundredths, &udateAge);

	valid &= (flat != TinyGPS::GPS_INVALID_F_ANGLE);
	valid &= (flon != TinyGPS::GPS_INVALID_F_ANGLE);
	valid &= (falt != TinyGPS::GPS_INVALID_F_ALTITUDE);
	valid &= (fspeed != TinyGPS::GPS_INVALID_F_SPEED);
	valid &= (uhdop != TinyGPS::GPS_INVALID_HDOP);
	valid &= (usat != TinyGPS::GPS_INVALID_SATELLITES);
	valid &= (ufixAge != TinyGPS::GPS_INVALID_AGE);
	valid &= (udateAge != TinyGPS::GPS_INVALID_AGE);

	if (valid)
	{
		// Afficher des floats...
		unsigned long int a,b;
		a =     (flat-(int)flat) *1000000;
		b =     (flon-(int)flon) *1000000;
		
		sprintf(stringBuffer, "%d.%lu,%d.%lu,%d.%d,%d.%d,%lu,%lu,%lu,%04d-%02d-%02dT%02d:%02d:%02d.%03dZ,%lu",
			// Position, vitesse
			(int)flat,   a,
			(int)flon,   b,
			(int)falt,   int((falt-(int)falt) *100),
			(int)fspeed, int((fspeed-(int)fspeed) *100),
			// Fiabilité : nombre de satellite, hdop
			usat,
			uhdop,
			ufixAge,
			// Date au format ISO 8601
			iyear, bmonth, bday, bhour, bminute, bsecond, bhundredths,
			//
			udateAge
		);
	}

	return valid;
}

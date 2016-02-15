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
		sprintf(stringBuffer, "%d.%d,%d.%d,%d.%d,%d.%d,%u,%u,%u,%02d/%02d/%02d,%02d:%02d:%02d,%u",
		
			(int)flat, int((flat-(int)flat)*10000),
			(int)flon, int((flon-(int)flon)*10000),
			(int)falt, int((falt-(int)falt)*10000),
			(int)fspeed, int((fspeed-(int)fspeed)*10000),
			
			usat,
			uhdop,
			ufixAge,
			
			bmonth, bday, iyear, bhour, bminute, bsecond,
		
			udateAge
		);
	}

	return valid;
}

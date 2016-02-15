#include "gps.h"
#include <stdlib.h>

int gpsSetup(struct gpsData* pMyGpsData)
{
	memset (pMyGpsData, '\0', sizeof(struct gpsData));
}

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

bool gpsSetData(TinyGPS *myGps, struct gpsData* pMyGpsData)
{
	float flat, flon, falt, fspeed;
	byte bmonth, bday, bhour, bminute, bsecond, bhundredths;
	int iyear;
	unsigned long ufixAge, usat, uhdop, udateAge;
	bool valid = true;
	
	myGps->f_get_position(&flat, &flon, &ufixAge);
	falt = myGps->f_altitude();
	fspeed = myGps->f_speed_kmph();
	uhdop = myGps->hdop();
	usat = myGps->satellites();
	
	myGps->crack_datetime(&iyear, &bmonth, &bday,
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
		// On remet tout à zéro
		gpsSetup(pMyGpsData);
		// On met à jour la structure gpsData
		// Conversion en chaînes de caractères
		dtostrf(flat, 8, 6, pMyGpsData->latitude);
		dtostrf(flon, 9, 6, pMyGpsData->longitude);
		dtostrf(falt, 6, 2, pMyGpsData->altitude);
		dtostrf(fspeed, 5, 2, pMyGpsData->speed);
		itoa(uhdop, pMyGpsData->hdop, 4);
		itoa(usat, pMyGpsData->satellites, 3);
		itoa(ufixAge, pMyGpsData->fixAge, 6);
		sprintf(pMyGpsData->date, "%02d/%02d/%02d", bmonth, bday, iyear);
		sprintf(pMyGpsData->time, "%02d:%02d:%02d", bhour, bminute, bsecond);
		itoa(udateAge, pMyGpsData->dateAge, 6);
	}
	
	return valid;
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
/*
 * 		formatage des floats ne fonctionne pas... A revoir.
		sprintf(stringBuffer, "%8.6f,%9.6,%6.2f,%5.2f,%u,%u,%u,%02d/%02d/%02d,%02d:%02d:%02d,%u",
				flat,
				flon,
				falt,
				fspeed,

				usat,
				uhdop,
				ufixAge,

				bmonth, bday, iyear,
				bhour, bminute, bsecond,

				udateAge
		);*/
		sprintf(stringBuffer, "%d.%d,%d.%d,%02d/%02d/%02d,%02d:%02d:%02d", (int)flat,int((flat-(int)flat)*10000), (int)flon, int((flon-(int)flon)*10000), bmonth, bday, iyear, bhour, bminute, bsecond);
	}

	return valid;
}

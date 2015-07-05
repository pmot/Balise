#include "gps.h"

void gpsRead(TinyGPS gps, SoftwareSerial serial, unsigned long ms)
{
	serial.listen();

	unsigned long start = millis();

	do {
		while (serial.available())
			gps.encode(serial.read());
	} while (millis() - start < ms);
}

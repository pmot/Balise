#ifndef __GPS_H__
#define __GPS_H__

#include <Arduino.h>
#include <TinyGPS.h>
#include <SoftwareSerial.h>

void gpsRead(TinyGPS, SoftwareSerial, unsigned long);

#endif

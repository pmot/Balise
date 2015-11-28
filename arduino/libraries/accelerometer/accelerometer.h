#ifndef __ACCELEROMETER_H__
#define __ACCELEROMETER_H__


// 3-axis Accelerometer
// Sparkfun Electronics Triple Axis Accelerometer Breakout - LIS331
// Arduino UNO

#include <Arduino.h>
#include <Wire.h>
#include <LIS331.h>

#define SCALE 1	//.0007324;
				// approximate scale factor for full range (+/-24g)
				// scale factor: +/-24g = 48G range. 2^16 bits. 48/65536 = 0.0007324

bool accelerometerSetup(LIS331 *);

#endif

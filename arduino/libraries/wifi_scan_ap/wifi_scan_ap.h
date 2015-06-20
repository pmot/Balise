#ifndef __WIFI_SCAN_AP_H__
#define __WIFI_SCAN_AP_H__

#include <Arduino.h>
#include <WiFly.h>

struct apEntry* wifiScanAp(WiFly);
char* wifiScanReadLn(WiFly);

struct apEntry
{
	int rssi = -1000;
	char ssid[50] = "UNKNOWN";
	char mac[18] = "xx:xx:xx:xx:xx:xx";
};

#endif

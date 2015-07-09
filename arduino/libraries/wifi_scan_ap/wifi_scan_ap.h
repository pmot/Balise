#ifndef __WIFI_SCAN_AP_H__
#define __WIFI_SCAN_AP_H__

#include <Arduino.h>
#include <WiFly.h>

#define NB_SSID_SCAN 			1
#define MAX_LENGTH_SCAN_LINE 	110
#define LENGTH_SSID 			50
#define LENGTH_MAX_ADDRESS 		18

int wifiScanSetup(WiFly);
int wifiScanAp(WiFly);
int wifiScanApGetResult(struct apEntry*, WiFly);
int wifiScanReadLn(WiFly, char *);

struct apEntry
{
	int rssi;
	char ssid[LENGTH_SSID]; // = "UNKNOWN";
	char mac[LENGTH_MAX_ADDRESS];  // = "xx:xx:xx:xx:xx:xx";
};

#endif

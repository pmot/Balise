#ifndef __WIFI_SCAN_AP_H__
#define __WIFI_SCAN_AP_H__

#include <Arduino.h>
#include <WiFly.h>

#define MAX_LENGTH_SCAN_LINE 	110
#define	WIFI_RSSI_LENGTH		4	// -200
#define WIFI_SSID_LENGTH 		32	// une chaine...
#define WIFI_MAC_ADDR_LENGTH 	17  // xx:xx:xx:xx:xx:xx

const char wifiCompactModeStr[] PROGMEM = "set sys printlvl 0x4000\r";
const char wifiScanStr[]  PROGMEM = "scan\r";
const char wifiStartStr[] PROGMEM = "SCAN:";
const char wifiStopStr[]  PROGMEM = "END:";	
	
bool wifiSetup(WiFly);
bool wifiScan(WiFly);
int wifiGetResult(WiFly, bool, char*);
int wifiScanReadLn(WiFly, char *);

#endif

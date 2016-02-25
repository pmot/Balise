// Code qui permet de lister les AP visibles
#include "wifi_scan_ap.h"

bool wifiSetup(WiFly myWiFly) {
	// Scan, nouveau mode nécessite un firmware >= 2.22
	if (myWiFly.version() < 2.22)
	{
		return false;
	}
	
	if (myWiFly.sendCommand(wifiCompactModeStr))
	{
		return true;
	}
	else
	{
		return false;
	}
}

bool wifiScan(WiFly myWiFly) {
	// un scan wifi via le RN 171 met environ 2500ms par défaut:
	// - 200ms par canal
	// - 13 canaux
	if (myWiFly.sendCommand(wifiScanStr))
	{
		return true;
	}
	else
	{
		return false;
	}
}

int wifiGetResult(WiFly myWiFly, bool first, char* res) {
	char *p, *pRSSI, *pSSID, *pMAC;
	bool found = false;
	int field = 1;
	char c;
	unsigned long time = millis();
	char newLine[MAX_LENGTH_SCAN_LINE];

	p = pRSSI = pSSID = pMAC = NULL;
	
	// On cherche SCAN:
	if (first) {
		while (!found) {
			if (wifiScanReadLn(myWiFly, newLine)) {
				if (strstr(newLine, wifiStartStr)) found = true;
			}
			// SCAN: non trouvé dans les temps
			if ((millis() - time) > 500) return -1;
		}
	}
	
	// On lit une ligne
	while (wifiScanReadLn(myWiFly, newLine)) {
		if ((millis() - time) > 500) return -1;
	}
	
	// END:
	if (strstr(newLine, wifiStopStr)) return 0;
	
	// Split de la chaine
	// N°, Chan, RSSI, Security Mode, MAC, SSID
	p = newLine;
	while (((c = *p) != NULL) and (field < 6)) {
		if (c == ',') {
			field++;
			*p = '\0';
		}
		p++;
		if (field == 3) pRSSI = p;
		if (field == 5) pMAC = p;
		if (field == 6) pSSID = p;
	}
	sprintf(res, "%s,%s,%s", pRSSI, pSSID, pMAC);
	return strlen(res);
}

int wifiScanReadLn(WiFly myWiFly, char *ptrLine) {
	bool endOfLine = false;
	char c=' ';
	int i=0;

	while (!endOfLine && i<(MAX_LENGTH_SCAN_LINE-1)) 	{
		if (myWiFly.receive((uint8_t *)&c, 1, 300) > 0) {
			if (c != '\r')
				ptrLine[i++] = c;
			else
				endOfLine = true;
		}
		else endOfLine = true; // Plus rien de dispo en entrée...
	}

	ptrLine[i] = '\0';

	return i;
}

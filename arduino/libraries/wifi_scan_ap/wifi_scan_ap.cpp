// Code qui permet de lister les AP visibles
#include "wifi_scan_ap.h"

int wifiScanSetup(WiFly myWiFly) {
	// Scan, nouveau mode nécessite un firmware >= 2.22

	const static char newMode[] PROGMEM = "set sys printlvl 0x4000\r";
	if (myWiFly.version() < 2.22)
		return 0;

	if (!myWiFly.sendCommand(newMode))
		return 0;
	else
		return 1;
}

int wifiScanAp(WiFly myWiFly) {
	// un scan wifi via le RN 171 met environ 2500ms par défaut:
	// - 200ms par canal
	// - 13 canaux
	const static char scanCmd[] PROGMEM = "scan\r";
	if (!myWiFly.sendCommand(scanCmd))
		return 0;
	else
		return 1;
}

int wifiScanApGetResult(struct apEntry* ptAP, WiFly myWiFly) {

	// un scan wifi via le RN 171 met environ 2500ms par défaut:
	// - 200ms par canal
	// - 13 canaux
	// Retourne la liste des AP au format JSON :
	// Sauver de la SRAM avec PROGMEM
	const static char startPattern[] PROGMEM = "SCAN:";
	const static char stopPattern[] PROGMEM = "END:";

	int nbScanned = 0;
	int nbAdded = 0;
	bool wifiScanBegin = false;
	bool wifiScanEnd = false;
	char* p = NULL;
	unsigned long end_millis;
	char newLine[MAX_LENGTH_SCAN_LINE];

	//
	// Analyse de la sortie, 500ms max !
	//
	end_millis = millis() + 500;


	while ((!wifiScanEnd) && (millis() < end_millis)) {
		if (wifiScanReadLn(myWiFly,newLine)) {
			if (!wifiScanBegin) {
				if (strstr(newLine, startPattern)) {
					wifiScanBegin = true;
					//
					// Le nombre d'AP visibles vient après le ':'
					//
					p = strtok(newLine, " ");
					p = strtok(NULL, " ");
					nbScanned = atoi(p); // nb de SSID trouvé
					if (nbScanned == 0)
						wifiScanEnd = true;
					else {
						nbScanned = min(nbScanned,NB_SSID_SCAN); // la taille du tableau est limitée
					}
				}
			}
			else {
				if (strstr(newLine, stopPattern))
					wifiScanEnd = true;
				else {
					/////////////////////////////////////////////////////////
					// Nouvelle entrée apEntry à logger dans la liste apList
					// Parsing newLine séparation des champs par des ','
					// RSSI : champ 3
					// MAC : champ 6
					// SSID : champ 7
					/////////////////////////////////////////////////////////
					p = strtok(newLine, ",");
					p = strtok(NULL, ",");
					p = strtok(NULL, ",");
					if (p) ptAP[nbAdded].rssi = atoi(p);
					p = strtok(NULL, ",");
					p = strtok(NULL, ",");
					p = strtok(NULL, ",");
					p = strtok(NULL, ",");
					p = strtok(NULL, ",");
					if (p) {
						strncpy(ptAP[nbAdded].mac, p, LENGTH_MAX_ADDRESS-1);
						ptAP[nbAdded].mac[LENGTH_MAX_ADDRESS-1]='\0';
					}
					p = strtok(NULL, ",");
					if (p) {
						strncpy(ptAP[nbAdded].ssid, p, LENGTH_SSID-1);
						ptAP[nbAdded].ssid[LENGTH_SSID-1]='\0';
					}
					if (p) nbAdded++;

					if (nbAdded >= nbScanned) wifiScanEnd = true;

				}
			}
		}
	}

	return nbAdded;
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


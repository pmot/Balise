// Code qui permet de lister les AP visibles
#include "wifi_scan_ap.h"

int wifiScanSetup(WiFly myWiFly)
{
  // Scan, nouveau mode nécessite un firmware >= 2.22
  if (myWiFly.version() < 2.22) return 0;
  if (!myWiFly.sendCommand("set sys printlvl 0x4000\r")) return 0;
  else return 1;  
}

int wifiScanAp(WiFly myWiFly)
{
  // un scan wifi via le RN 171 met environ 2500ms par défaut:
  // - 200ms par canal
  // - 13 canaux  
  if (!myWiFly.sendCommand("scan\r")) return 0;
  else return 1;
}

int wifiScanApGetResult(struct apEntry** ptAP, WiFly myWiFly)
{
  // un scan wifi via le RN 171 met environ 2500ms par défaut:
  // - 200ms par canal
  // - 13 canaux
  // Retourne la liste des AP au format JSON :
  int nbScanned = 0;
  int nbAdded = 0;
  bool wifiScanBegin = false;
  bool wifiScanEnd = false;
  char* newLine = NULL;
  char* p = NULL;
  unsigned long end_millis;
  
  // Analyse de la sortie, 500ms max !
  end_millis = millis() + 500;
  while ((!wifiScanEnd) && (millis() < end_millis)) {
	if (newLine = wifiScanReadLn(myWiFly))
	{
		if (!wifiScanBegin)
		{
			if (strstr(newLine, "SCAN:"))
			{
				wifiScanBegin = true;
				// Le nombre d'AP visibles vient après le ':'
				p = strtok(newLine, " ");
				p = strtok(NULL, " ");
				nbScanned = atoi(p);
				if (nbScanned == 0) wifiScanEnd = true;
				// else Allouer l'escpace nécessaire
				else
				{
					*ptAP = (struct apEntry*)malloc(nbScanned*sizeof(struct apEntry));
				}
			}
		}
		else
		{
			if (strstr(newLine, "END:")) wifiScanEnd = true;
			else
			{
				// Nouvelle entrée apEntry à logger dans la liste apList
				// Parsing newLine séparation des champs par des ','
				// RSSI : champ 3
				// MAC : champ 6
				// SSID : champ 7
				p = strtok(newLine, ",");
				p = strtok(NULL, ",");
				p = strtok(NULL, ",");
				if (p) (*ptAP)[nbAdded].rssi = atoi(p);
				p = strtok(NULL, ",");
				p = strtok(NULL, ",");
				p = strtok(NULL, ",");
				p = strtok(NULL, ",");
				p = strtok(NULL, ",");
				if (p) strncpy((*ptAP)[nbAdded].mac, p, strlen(p));
				p = strtok(NULL, ",");
				if (p) strncpy((*ptAP)[nbAdded].ssid, p, strlen(p));
				if (p) nbAdded++;
			}
		}
		free(newLine); 
	}
  }
  
  if (nbAdded != nbScanned)
  {
	  // Mismatch, or truncated
  }

  return nbAdded;
}

char* wifiScanReadLn(WiFly myWiFly)
{
	bool endOfLine = false;
	char c=' ';
	int i=0;
	// Taille max d'une ligne = 100 cars (évite les realloc())
	char tabCar[101] = "";
	char* newLine = NULL;

	while (!endOfLine)
	{
	  if (myWiFly.receive((uint8_t *)&c, 1, 300) > 0)
	  {
		if (c != '\r') tabCar[i++] = c;
		else endOfLine = true;
	  }
	  else endOfLine = true; // Plus rien de dispo en entrée...
	}

	if (i>0)
	{
	  tabCar[i++] = '\0';
	  newLine = (char*)malloc(i);
	  strncpy(newLine, tabCar, i);
	  // Ceinture/bretelle/nul en C !
	  newLine[i] = '\0';
	}
	
	return newLine;
}
	

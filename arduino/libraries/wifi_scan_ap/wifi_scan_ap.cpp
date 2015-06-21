// Code qui permet de lister les AP visibles
#include "wifi_scan_ap.h"

struct apEntry* wifiScanAp(WiFly myWiFly)
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
  struct apEntry* apList;
  
  // Scan, nouveau mode nécessite un firmware >= 2.22
  if (myWiFly.version() < 2.22) return 0;
  if (!myWiFly.sendCommand("set sys printlvl 0x4000\r")) return 0;
  if (!myWiFly.sendCommand("scan\r")) return 0;

  // On attend la fin du scan, environ 3s
  delay(2500);

  // Analyse de la sortie, 500ms max !
  end_millis = millis() + 500;
  while ((!wifiScanEnd) && (millis() < end_millis)) {
	if (newLine = wifiScanReadLn(myWiFly))
	{
		if (!wifiScanBegin)
		{
			if (strstr(newLine, "SCAN:"))
			{
				// Le nombre d'AP visibles vient après le ':'
				p = strtok(newLine, ":");
				nbScanned = atoi(++p);
				if (nbScanned == 0) wifiScanEnd = true;
				// else Allouer l'escpace nécessaire
				else
				{
					apList = (struct apEntry*)malloc(nbScanned*sizeof(struct apEntry));
				}
			}
		}
		else
		{
			if (strstr(newLine, "END\r")) wifiScanEnd = true;
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
				apList[nbAdded].rssi = atoi(p);
				p = strtok(NULL, ",");
				p = strtok(NULL, ",");
				p = strtok(NULL, ",");
				strncpy(apList[nbAdded].mac, p, strlen(p));
				p = strtok(NULL, ",");
				strncpy(apList[nbAdded].ssid, p, strlen(p));
				nbAdded++;
			}
		}
		free(newLine); 
	}
  }
  
  if (nbAdded != nbScanned)
  {
	  // Mismatch, or truncated
  }
  // Retourne un tableau de struct apEntry... Le format JSON est pour toi
  return apList;
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
	

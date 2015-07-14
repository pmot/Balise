#ifndef __CORE_PARAMETERS_H__
#define __CORE_PARAMETERS_H__

// Définition des pin RX/TX des modules
#define WIFI_TX  4
#define WIFI_RX  5
#define GPS_TX   6
#define GPS_RX   7
#define CONSOLE_RX	12
#define CONSOLE_TX	11


// GSM
// PIN Number
#define PINNUMBER ""
// APN data
#define GPRS_APN       "websfr" // replace your GPRS APN
#define GPRS_LOGIN     ""    // replace with your GPRS login
#define GPRS_PASSWORD  "" // replace with your GPRS password

// Temps alloué à la lecture des données GPS sur la liaison série en ms
#define GPS_READ_TIME	1000
// Délai entre deux lectures de position GPS en ms
#define GPS_READ_DELAY	5000
// Délais d'attente de lecture du résultat du SCAN Wifi en ms (environ 3s)
#define WIFI_SCAN_TIME	1500
// Délais entre deux scan d'AP WIFI en ms
#define WIFI_SCAN_DELAY	10000

// Renvoi vrai si le ts est atteint
static bool itsTimeFor(unsigned long);


#endif
 

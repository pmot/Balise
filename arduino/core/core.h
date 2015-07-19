#ifndef __CORE_H__
#define __CORE_H__

// Définition des pin RX/TX des modules
#define GSM_TX	 0	// UART
#define GSM_RX	 1	// " "
#define WIFI_TX  4
#define WIFI_RX  5
#define GPS_TX   6
#define GPS_RX   7
#define CONSOLE_TX	11
#define CONSOLE_RX	12

// GSM
// PIN Spécifiques
#define GSM_RST		9
#define GSM_STATUS	10
// Code PIN
#define GSM_PIN			""
// APN
static const char gprsAPN[] PROGMEM = "websfr";
static const char gprsLogin[] PROGMEM = "";
static const char gprsPassword[] PROGMEM = "";
static const char pinCode[] PROGMEM = "1234";


// Temps alloué à la lecture des données GPS sur la liaison série en ms
#define GPS_READ_TIME	1000
// Délai entre deux lectures de position GPS en ms
#define GPS_READ_DELAY	5000
// Délais d'attente de lecture du résultat du SCAN Wifi en ms (environ 3s)
#define WIFI_SCAN_TIME	4000
// Délais entre deux scan d'AP WIFI en ms
#define WIFI_SCAN_DELAY	10000

// Renvoi vrai si le ts est atteint
static bool itsTimeFor(unsigned long);


#endif

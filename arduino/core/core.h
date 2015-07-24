#ifndef __CORE_H__
#define __CORE_H__

// Identifiant de la balise
#define	BALISE_ID	"AVEZE"


// Mode d'élection des tâches (exclusif)
// #define ORD_PREEMPT	// Privilégier l'exécution d'une tâche, par défaut
#define ORD_RT			// Privilégier la régularité

// Flag trace
// 0 = no trace
// 1 = ERROR
// 2 = DEBUG
// 3 = TRACE (contenu des variables)
#define LOG_ERROR 1
#define LOG_INFO 2
#define LOG_TRACE 4

const char debug PROGMEM = LOG_TRACE;

#define PRINT_LOG(y,x)  if(debug>=y) { consoleSerial.print(__FUNCTION__); consoleSerial.print(F(": ")) ; consoleSerial.println(x); }

// Définition des pin RX/TX des modules
// #define GSM_TX	 0	// UART
// #define GSM_RX	 1	// " "
#define WIFI_TX  4
#define WIFI_RX  5
#define GPS_TX   6
#define GPS_RX   7
#define CONSOLE_TX	11
#define CONSOLE_RX	12
// GSM
#define GSM_PWRK	10
//
#define LED_PIN		13
// APN
const char gprsAPN[] PROGMEM = "websfr";
const char gprsLogin[] PROGMEM = "";
const char gprsPassword[] PROGMEM = "";
const char pinCode[] PROGMEM = "1234";
// Serveur
const char server[] PROGMEM = "geolocsp.com";
const char port[] PROGMEM = "80";
const char urlInit[] PROGMEM = "GET /webservice/up/AVEZE HTTP/1.1";
const char urlGpsWS[] PROGMEM = "GET /webservice/ws?gps=";


// Temps alloué à la lecture des données GPS sur la liaison série en ms
#define GPS_READ_TIME	2000
// Délai entre deux lectures de position GPS en ms
#define GPS_READ_DELAY	5000
// Délai d'attente de lecture du résultat du SCAN Wifi en ms (environ 3s)
#define WIFI_SCAN_TIME	4000
// Délai entre deux scan d'AP WIFI en ms
#define WIFI_SCAN_DELAY	10000
// Délai entre deux transmissions au sol
#define SEND_TO_GROUND_DELAY	60000

// Renvoi vrai si le ts est atteint
// static bool itsTimeFor(unsigned long);
// Affichage des données sur le port console
// A des fins de test/debug
void printGpsData(struct gpsData *);


#endif

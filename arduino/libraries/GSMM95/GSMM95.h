#ifndef __GSMM95_H__
#define __GSMM95_H__
  
#include <Arduino.h>
#include <SoftwareSerial.h>		// Pour debugger sur la console

#define GSM_BUFSZ		32		// Taille du buffer
#define GSM_BAUDRATE	115200

// http://www.quectel.com/UploadImage/Downlad/M95_AT_Commands_Manual_V1.2.pdf
// Code retours connus, voir gsmStrings plus bas.

#define	GSMSTATE_PIN_REQ			3
#define GSMSTATE_PIN_RDY			4
#define GSMSTATE_NET_REG_HOME		5
#define GSMSTATE_NET_REG_DENIED		6
#define GSMSTATE_NET_REG_ROAMING	7
#define GSMSTATE_GPRS_ATTACHED		9
#define GSMSTATE_GPRS_IP_STACK		10
#define GSMSTATE_IP_INITIAL			11
#define GSMSTATE_IP_GPRSACT			12
// ...
#define	GSMSTATE_IPR				16
// ...
#define	GSMSTATE_OK					19
// ...
#define	GSMSTATE_UNKNOWN			9998
#define	GSMSTATE_INVALID			9999

// Automate initialisation
#define GSMINIT_STATE_START						100
#define	GSMINIT_STATE_TEST_MODEM				101
#define GSMINIT_STATE_SET_BAUDRATE				102
#define GSMINIT_STATE_DISABLE_ECHO				103
#define GSMINIT_STATE_DISABLE_URC				104
#define GSMINIT_STATE_ENABLE_CREG				105
#define GSMINIT_STATE_ENABLE_CGREG				106
#define GSMINIT_STATE_TEST_SIM_PIN				107
#define GSMINIT_STATE_ULOCK_SIM_PIN				108
#define GSMINIT_STATE_SIM_STATUS_SECOND_CHANCE	109	// Si la SIM n'a pas eu le temps de s'éveiller
#define GSMINIT_STATE_SET_CGQMIN				110
#define GSMINIT_STATE_SET_CGQREQ				111
#define GSMINIT_STATE_DONE						115
#define GSMINIT_STATE_SIM_OK					116 // Special

// Automate connexion
#define GSMCONNECT_STATE_START						200
#define	GSMCONNECT_STATE_TEST_NET_REG				201 // 0,1 -> home / 0,5 -> roaming / 0,3 -> denied !
#define GSMCONNECT_STATE_ATTACH_GPRS				202
#define GSMCONNECT_STATE_SET_QIFGCNT				203
#define GSMCONNECT_STATE_SET_PDP					204
#define GSMCONNECT_STATE_DISABLE_MULTI				205
#define GSMCONNECT_STATE_ENABLE_TRANSPARENT_MODE	206
#define GSMCONNECT_STATE_REG_APP					207
#define GSMCONNECT_STATE_ACTIVATE					208
//
#define GSMCONNECT_STATE_TEST_IP_STACK				209
#define GSMCONNECT_STATE_IP_INITIAL					220
#define GSMCONNECT_STATE_SET_DOMAIN					230
#define GSMCONNECT_STATE_DONE						240

#define MAX_GSM_STRINGS		20
#define MAX_GSM_STRING_SZ 	20

const char gsmStr0[]  PROGMEM = "SEND OK\r\n";			// 0
const char gsmStr1[]  PROGMEM = "OK\r\n\r\nCONNECT\r\n";// 1
const char gsmStr2[]  PROGMEM = "CONNECT OK\r\n";		// 2
const char gsmStr3[]  PROGMEM = "SIM PIN\r\n";			// 3
const char gsmStr4[]  PROGMEM = "READY\r\n";			// 4
const char gsmStr5[]  PROGMEM = "1,1\r\n";				// 5 - Registered - HOME
const char gsmStr6[]  PROGMEM = "1,3\r\n";				// 6 - Denied !!!
const char gsmStr7[]  PROGMEM = "1,5\r\n";				// 7 - Registered - ROAMING
const char gsmStr8[]  PROGMEM = "NO CARRIER\r\n";		// 8
const char gsmStr9[]  PROGMEM = "+CGATT: 1\r\n";		// 9
const char gsmStr10[] PROGMEM = "IP ";					// 10
const char gsmStr11[] PROGMEM = "IP INITIAL\r\n";		// 11
const char gsmStr12[] PROGMEM = "IP GPRS_ACT\r\n";		// 12
const char gsmStr13[] PROGMEM = "IP STATUS\r\n";		// 13
const char gsmStr14[] PROGMEM = "IP CLOSE\r\n";			// 14
const char gsmStr15[] PROGMEM = "ALREADY CONNECT\r\n";	// 15
const char gsmStr16[] PROGMEM = "IPR=";					// 16
const char gsmStr17[] PROGMEM = "+CPMS:";
const char gsmStr18[] PROGMEM = ":0\r\n";
const char gsmStr19[] PROGMEM = "OK\r\n";				// 19

const char* const gsmStrings[] PROGMEM = {
	gsmStr0, gsmStr1, gsmStr2, gsmStr3, gsmStr4, gsmStr5, gsmStr6,
	gsmStr7, gsmStr8, gsmStr9, gsmStr10, gsmStr11, gsmStr12, gsmStr13,
	gsmStr14, gsmStr15, gsmStr16, gsmStr17, gsmStr18, gsmStr19 };

struct gsmContext {
	  bool qiMux;		//
	  bool qiMode;		//
	  bool gprsReady;	// Connecte au GPRS, pret a envoyer des donnees IP
	  int  state;	//
	  SoftwareSerial* pConsole;
};

// Initialisation des structures de donnees
void gsmSetup(struct gsmContext*, SoftwareSerial*);
// Init hard du modem : PWR KEY
void gsmHardReset(struct gsmContext*, byte);
// Init soft du modem : PIN CODE
int  gsmInit(struct gsmContext*, const char*);
// Affiche les infos CSQ, CGREG, etc. sur la console
void gsmInfo();
// Retourne true si il y a besoin de lancer la connexion
bool gsmNeedToConnect(struct gsmContext*);
// Lancer la connexion GPRS : APN, USR, PWD
int  gsmGprsConnect(struct gsmContext*, const char*, const char*, const char*);
// Deconnexion GPRS
void gsmGprsDisconnect(struct gsmContext*);
// GET url, port, les donnees
int  gsmHttpRequest(struct gsmContext*, const char*, char*);
// Lecture retour des commandes AT : buffer, timeout
int  gsmExpect(struct gsmContext*, char*, int);

#endif

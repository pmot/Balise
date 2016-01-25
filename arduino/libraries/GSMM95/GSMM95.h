#ifndef __GSMM95_H__
#define __GSMM95_H__
  
#include <Arduino.h>
#include <SoftwareSerial.h>		// Pour debugger sur la console

#define GSM_BUFSZ		150		// Taille du buffer (circulaire ?)
#define GSM_BAUDRATE	115200

// Code retours connus, voir gsmStrings plus bas.

#define	GSMSTATE_PIN_REQ		3
#define GSMSTATE_PIN_RDY		4
#define GSMSTATE_NET_REG_2G		5
#define GSMSTATE_NET_REG_3G		6
#define GSMSTATE_GPRS_ATTACHED	8
#define GSMSTATE_IP_INITIAL		9
// ...
#define	GSMSTATE_OK				15

#define	GSMSTATE_UNKNOWN		9998
#define	GSMSTATE_INVALID		9999

// Automate initialisation
#define GSMINIT_STATE_START						100
#define	GSMINIT_STATE_MODEM_OK					101
#define GSMINIT_STATE_ECHO_DISABLED				102
#define GSMINIT_STATE_SIM_STATUS_SECOND_CHANCE	103	// Si la SIM n'a pas eu le temps de s'éveiller
#define GSMINIT_STATE_PIN_REQUIRED				104
#define GSMINIT_STATE_SIM_OK					105
#define GSMINIT_STATE_BAUDRATE_FIXED			106
#define GSMINIT_STATE_URC_DISABLED				107
#define GSMINIT_STATE_REGISTERED				108

// Automate connexion
#define GSMCONNECT_STATE_START					200
#define	GSMCONNECT_STATE_REGISTERED				201
#define GSMCONNECT_STATE_GPRS_ATTACHED			202
#define GSMCONNECT_STATE_IP_INITIAL				203
#define GSMCONNECT_STATE_APN_SET				204
#define GSMCONNECT_STATE_DOMAIN_SET				205
#define GSMCONNECT_STATE_DONE					206

#define MAX_GSM_STRINGS		16
#define MAX_GSM_STRING_SZ 	20

const char gsmStr0[] PROGMEM = "SEND OK\r\n";			// 0
const char gsmStr1[] PROGMEM = "OK\r\n\r\nCONNECT\r\n";	// 1
const char gsmStr2[] PROGMEM = "CONNECT OK\r\n";		// 2
const char gsmStr3[] PROGMEM = "SIM PIN\r\n";			// 3
const char gsmStr4[] PROGMEM = "READY\r\n";				// 4
const char gsmStr5[] PROGMEM = "0,1\r\n";				// 5
const char gsmStr6[] PROGMEM = "0,5\r\n";				// 6
const char gsmStr7[] PROGMEM = "NO CARRIER\r\n";		// 7
const char gsmStr8[] PROGMEM = "+CGATT: 1\r\n";			// 8
const char gsmStr9[] PROGMEM = "IP INITIAL\r\n";
const char gsmStr10[] PROGMEM = "IP STATUS\r\n";
const char gsmStr11[] PROGMEM = "IP CLOSE\r\n";
const char gsmStr12[] PROGMEM = "ALREADY CONNECT\r\n";
const char gsmStr13[] PROGMEM = "+CPMS:";
const char gsmStr14[] PROGMEM = ":0\r\n";
const char gsmStr15[] PROGMEM = "OK\r\n";				// 15

const char* const gsmStrings[] PROGMEM = {
	gsmStr0, gsmStr1, gsmStr2, gsmStr3, gsmStr4, gsmStr5, gsmStr6,
	gsmStr7, gsmStr8, gsmStr9, gsmStr10, gsmStr11, gsmStr12, gsmStr13,
	gsmStr14, gsmStr15 };

class GSMM95
{
    public:      
      GSMM95(byte, SoftwareSerial*); // Constructeur
	  int Init(const char*);
	  int Status();
      
      int Connect(const char*, const char*, const char*);
      int SendHttpReq(const char*, const char*, char*);
      void Disconnect();

      char gsmBuf[GSM_BUFSZ];
	  byte pwrKey;
	  int state;
	  SoftwareSerial* pconsole;
      int Expect(int);
      byte Automate(char);
};

#endif

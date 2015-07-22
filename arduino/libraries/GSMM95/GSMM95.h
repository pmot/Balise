#ifndef __GSMM95_H__
#define __GSMM95_H__
  
#include <Arduino.h>
#include <SoftwareSerial.h>		// Pour debugger sur la console

#define GSM_BUFSZ		150		// Taille du buffer (circulaire ?)
#define GSM_BAUDRATE	115200

// Code retours connus, voir gsmStrings plus bas.
#define	GSMSTATE_UNKNOWN	0
#define	GSMSTATE_INVALID	1000
#define	GSMSTATE_OK			1
#define	GSMSTATE_PIN_REQ	2
#define GSMSTATE_PIN_RDY	3
#define GSMSTATE_NET_REG_2G	4
#define GSMSTATE_NET_REG_3G	5

#define GSMSTATE_GPRS_REGISTERED	7

// Automate initialisation
#define GSMINIT_STATE_START						100
#define	GSMINIT_STATE_MODEM_OK					101
#define GSMINIT_STATE_ECHO_DISABLED				102	// 2 cas, pas code, un code, sinon, seconde chance
#define GSMINIT_STATE_SIM_STATUS_SECOND_CHANCE	103	// Si la SIM n'a pas eu le temps de s'éveiller
#define GSMINIT_STATE_PIN_REQUIRED				104
#define GSMINIT_STATE_SIM_OK					105
#define GSMINIT_STATE_BAUDRATE_FIXED			106
#define GSMINIT_STATE_URC_DISABLED				107
#define GSMINIT_STATE_REGISTERED				108

// Automate connexion
#define GSMCONNECT_STATE_START					200
#define	GSMCONNECT_STATE_REGISTERED				201
#define GSMCONNECT_STATE_GPRS_REGISTERED		202
#define GSMCONNECT_STATE_IP_INITIAL				203
#define GSMCONNECT_STATE_APN_SET				204
#define GSMCONNECT_STATE_DOMAIN_SET				205
#define GSMCONNECT_STATE_DONE	206

#define MAX_GSM_STRINGS	16
static const char gsmStrings[MAX_GSM_STRINGS][20] PROGMEM =
{
	"OK\r\n",					// 1
	"SIM PIN\r\n",				// Etc
	"READY\r\n",
	"0,1\r\n",
	"0,5\r\n",
	"NO CARRIER\r\n",
	"+CGATT: 1\r\n",
	"IP INITIAL\r\n",
	"IP STATUS\r\n",
	"IP CLOSE\r\n",
	"CONNECT OK\r\n",
	"ALREADY CONNECT\r\n",
	"SEND OK\r\n",
	"+CPMS:",
	"OK\r\n\r\nCONNECT\r\n",
	":0\r\n"
								// 0 Sinon
};

class GSMM95
{
    public:      
      GSMM95(byte, SoftwareSerial*);			// Constructeur
	  int  Init(const char*);
	  int Status();
      
      int  Connect(const char*, const char*, const char*);
      int  SendHttpReq(const char*, const char*, char*);
      void Disconnect();
    
    private:
	  char gsmBuf[GSM_BUFSZ];
	  byte pwrKey;
	  int  state;
	  SoftwareSerial* pconsole;
      int  Expect(int);
      byte Automate(char);
};

#endif

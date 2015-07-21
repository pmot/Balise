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

// Automate initialisation
#define GSMINIT_STAGE_START						0
#define	GSMINIT_STAGE_MODEM_OK					1
#define GSMINIT_STAGE_ECHO_DISABLED				2	// 2 cas, pas code, un code, sinon, seconde chance
#define GSMINIT_STAGE_SIM_STATUS_SECOND_CHANCE	3	// Si la SIM n'a pas eu le temps de s'éveiller
#define GSMINIT_STAGE_PIN_REQUIRED				4
#define GSMINIT_STAGE_SIM_OK					5
#define GSMINIT_STAGE_BAUDRATE_FIXED			6
#define GSMINIT_STAGE_URC_DISABLED				7
#define GSMINIT_STAGE_REGISTERED				8

// Automate connexion
#define GSMCONNECT_STAGE_START	0
#define	GSMCONNECT_STAGE_1		1
#define GSMCONNECT_STAGE_2		2
#define GSMCONNECT_STAGE_3		3
#define GSMCONNECT_STAGE_4		4
#define GSMCONNECT_STAGE_5		5
#define GSMCONNECT_STAGE_DONE	6

#define MAX_GSM_STRINGS	16
static const char gsmStrings[16][20] PROGMEM =
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

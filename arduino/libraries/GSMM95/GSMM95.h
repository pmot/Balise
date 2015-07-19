#ifndef __GSMM95_H__
#define __GSMM95_H__
  
#include <Arduino.h>
#include <SoftwareSerial.h>

#define GSM_BUFSZ		150		// Taille du buffer (circulaire ?)
#define GSM_BAUDRATE	115200

#define	GSMSTATE_UNKNOWN	0
#define	GSMSTATE_INVALID	1000
#define	GSMSTATE_NEXT_STAGE	1
#define	GSMSTATE_OK			1
#define	GSMSTATE_PIN_REQ	2
#define GSMSTATE_PIN_RDY	3
#define GSMSTATE_STAGE4		4
#define GSMSTATE_STAGE5		5


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

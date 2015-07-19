#ifndef __GSMM95_H__
#define __GSMM95_H__
  
#include <Arduino.h>

#define GSM_BUFSZ		150		// Taille du buffer circulaire
#define GSM_BAUDRATE	115200
#define GSM_PWRK		12		// TODO : à remplacer

static const char gsmStrings[][10] PROGMEM =
{
	"TOTO",
	"TATA",
	"TITI"
};

class GSMM95
{
    public:      
      GSMM95();			// Constructeur, baudrate
	  int  Init(const char*);
	  int Status();
      
      int  Connect(const char*, const char*, const char*);
      int  SendHttpReq(const char*, char*);
      void Disconnect();
    
    private:
	  char gsmBuf[GSM_BUFSZ];
	  
      int  Expect(int);
      byte Automate(char);
};

#endif

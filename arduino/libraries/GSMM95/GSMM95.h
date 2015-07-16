#ifndef __GSMM95_H__
#define __GSMM95_H__
  
#include <Arduino.h>

#define GSM_BUFSZ		150
// const static char ATCREG[] PROGMEM = "AT+CREG?\r";

class GSMM95
{     
    public:

      char gsmBuf[GSM_BUFSZ];
      
      //GSMM95(); // Constructeur
	  void begin();		// Redondance de méthodes d'INIT...
	  int  init(char*);
	  int  Status();
      
      int  dataConnect(const char*, char*, char*);
      int  sendHttpReq(char*, char*);
      void dataDisconnect();
    
    private:
      int  expect(int);
};

#endif

#include <GSMM95.h>


GSMM95::GSMM95(byte myPwrKey, SoftwareSerial* pSerial)
{
  GSMM95::state = 0;
  GSMM95::pconsole = pSerial;
  GSMM95::pwrKey = myPwrKey;
  GSMM95::qimux = false;
  GSMM95::qimode = false;
  GSMM95::gprsReady = false;
  pinMode(GSMM95::pwrKey, OUTPUT);
}

/*
 * Initialisation du M95:
	- Reset du modem
	- Unlock de la sim
	- Sequence d'init (qos, echo, etc.)

Retourne :
* 0 ---> Error 
* 1 ---> OK 

"gsmBuf" contient la derniere reponse du module
*/
int GSMM95::Init(const char* pinCode)
{
	unsigned long time = millis();	// On donne un temps limite l'init

	GSMM95::pconsole->print(F("######## IN : "));
	GSMM95::pconsole->println(__FUNCTION__);
    
	// Init sequence, see "M95_HardwareDesign_V1.2.pdf", page 30.
	// Reset!
	GSMM95::pconsole->println(F("\tGSM - INIT - Power on the modem"));
	digitalWrite(GSMM95::pwrKey, LOW);
	delay(500);
	digitalWrite(GSMM95::pwrKey, HIGH);
	delay(2000);
	GSMM95::pconsole->println(F("\tGSM - INIT - Done"));
	
	// Start and Autobauding sequence ("M95_AT_Commands_V1.0.pdf", page 35) 
	Serial.begin(GSM_BAUDRATE);
	for (int i=0; i < 3; i++) {
		Serial.print('A');
		delay(100);
		Serial.print('T');
		delay(100);
		Serial.print('\r');
		delay(100);
	}

    // Clear buffer !!!
    GSMM95::pconsole->println(F("\tGSM - INIT - Flushing the buffer : "));
    Serial.setTimeout(5000);
    while (Serial.available()) GSMM95::pconsole->println(Serial.read());
    GSMM95::pconsole->println(F("\tGSM - INIT - Done"));

	GSMM95::state = GSMINIT_STATE_START;

	do {

		if(GSMM95::state == GSMINIT_STATE_START) {
			GSMM95::pconsole->println(F("\tGSM - INIT - Begin Loop state"));
			GSMM95::state = GSMINIT_STATE_SET_BAUDRATE;
			delay(500);
		}

		if(GSMM95::state == GSMINIT_STATE_SET_BAUDRATE) {
			GSMM95::pconsole->println(F("\tGSM - INIT - Actual state : GSMINIT_STATE_SET_BAUDRATE"));
			Serial.print(F("AT+IPR="));        // set Baudrate
			Serial.print(GSM_BAUDRATE);
			Serial.print('\r');
			GSMM95::state = Expect(1000) == GSMSTATE_IPR ? GSMINIT_STATE_TEST_MODEM : GSMINIT_STATE_SET_BAUDRATE;
		}

		if(GSMM95::state == GSMINIT_STATE_TEST_MODEM) {
			GSMM95::pconsole->println(F("\tGSM - INIT - GSMINIT_STATE_TEST_MODEM"));
			Serial.print(F("AT\r"));     			
			GSMM95::state = Expect(1000) == GSMSTATE_OK ? GSMINIT_STATE_DISABLE_ECHO : GSMINIT_STATE_TEST_MODEM;
		}

		if(GSMM95::state == GSMINIT_STATE_DISABLE_ECHO)	{
			GSMM95::pconsole->println(F("\tGSM - INIT - GSMINIT_STATE_DISABLE_ECHO"));
			Serial.print(F("ATE0\r"));
			GSMM95::state = Expect(1000) == GSMSTATE_OK ? GSMINIT_STATE_DISABLE_URC : GSMSTATE_INVALID;
		}

		if(GSMM95::state == GSMINIT_STATE_DISABLE_URC) {
			GSMM95::pconsole->println(F("\tGSM - INIT - GSMINIT_STATE_DISABLE_URC"));
			Serial.print(F("AT+QIURC=0\r"));    // disable initial URC presentation   
			GSMM95::state = Expect(1000) == GSMSTATE_OK ? GSMINIT_STATE_DISABLE_CREG : GSMSTATE_INVALID;
		}
		
		if(GSMM95::state == GSMINIT_STATE_DISABLE_CREG)	{
			GSMM95::pconsole->println(F("\tGSM - INIT - GSMINIT_STATE_DISABLE_CREG"));
			Serial.print(F("AT+CREG=0\r"));
			GSMM95::state = Expect(1000) == GSMSTATE_OK ? GSMINIT_STATE_DISABLE_CGREG : GSMSTATE_INVALID;
		}
		
		if(GSMM95::state == GSMINIT_STATE_DISABLE_CGREG)	{
			GSMM95::pconsole->println(F("\tGSM - INIT - GSMINIT_STATE_DISABLE_CGREG"));
			Serial.print(F("AT+CGREG=0\r"));
			GSMM95::state = Expect(1000) == GSMSTATE_OK ? GSMINIT_STATE_TEST_SIM_PIN : GSMSTATE_INVALID;
		}

		if(GSMM95::state == GSMINIT_STATE_TEST_SIM_PIN) {		// after 0,5 - 10 sec., depends of the SIM card
			GSMM95::pconsole->println(F("\tGSM - INIT - GSMINIT_STATE_TEST_SIM_PIN"));
			delay(1000);
			Serial.print(F("AT+CPIN?\r"));
			switch (Expect(10000)) {		// wait for initial URC presentation "+CPIN: SIM PIN" or similar
			case GSMSTATE_PIN_REQ:
				GSMM95::state = GSMINIT_STATE_ULOCK_SIM_PIN; // get +CPIN: SIM PIN -> To unlock
				break;
			case GSMSTATE_PIN_RDY:
				GSMM95::state = GSMINIT_STATE_SIM_OK; 		 // get +CPIN: READY -> Go ahead !
				break;
			default:
				GSMM95::state = GSMINIT_STATE_SIM_STATUS_SECOND_CHANCE;	// Sinon...
				break;
			}
		}

		if(GSMM95::state == GSMINIT_STATE_SIM_STATUS_SECOND_CHANCE) {
			GSMM95::pconsole->println(F("\tGSM - INIT - GSMINIT_STATE_SIM_STATUS_SECOND_CHANCE"));
			switch (Expect(10000)) { 		// new try: wait for initial URC presentation "+CPIN: SIM PIN" or similar
			    case GSMSTATE_PIN_REQ: GSMM95::state = GSMINIT_STATE_ULOCK_SIM_PIN; break; 		// get +CPIN: SIM PIN
			    case GSMSTATE_PIN_RDY: GSMM95::state = GSMINIT_STATE_SIM_OK; break;		// get +CPIN: READY
			    default:
				  GSMM95::pconsole->print(F("\tSIM ERREUR FATALE : >"));
				  GSMM95::pconsole->print(GSMM95::gsmBuf);    //    here is the explanation
				  GSMM95::pconsole->println(F("<"));
				  return 0;
			}
		}

		if(GSMM95::state == GSMINIT_STATE_ULOCK_SIM_PIN) {
			GSMM95::pconsole->println(F("\tGSM - INIT - GSMINIT_STATE_ULOCK_SIM_PIN"));
			GSMM95::pconsole->print(F("\tGSM - INIT - Setting PIN code : "));
			GSMM95::pconsole->println(pinCode);
			Serial.print(F("AT+CPIN="));           // enter pin (SIM)
			Serial.print(pinCode);
			Serial.print('\r');
			GSMM95::state = Expect(1000) == GSMSTATE_PIN_RDY ? GSMINIT_STATE_SIM_OK : GSMSTATE_INVALID;
		}

		if(GSMM95::state == GSMINIT_STATE_SIM_OK)	{
			GSMM95::pconsole->println(F("\tGSM - INIT - SIM UNLOCKED !!!"));
			GSMM95::state = GSMINIT_STATE_SET_CGQMIN;
			delay(500);
		}		
		
		if(GSMM95::state == GSMINIT_STATE_SET_CGQMIN)	{
			GSMM95::pconsole->println(F("\tGSM - INIT - GSMINIT_STATE_SET_CGQMIN"));
			Serial.print(F("AT+CGQMIN=1\r")); // ,0,0,1,0,0\r"));
			GSMM95::state = Expect(1000) == GSMSTATE_OK ? GSMINIT_STATE_SET_CGQREQ : GSMSTATE_INVALID;
		}
		
		if(GSMM95::state == GSMINIT_STATE_SET_CGQREQ)	{
			GSMM95::pconsole->println(F("\tGSM - INIT - GSMINIT_STATE_SET_QIFGCNT"));
			Serial.print(F("AT+CGQREQ=1\r")); // ,0,0,1,0,0\r"));
			GSMM95::state = Expect(1000) == GSMSTATE_OK ? GSMINIT_STATE_DONE : GSMSTATE_INVALID;
		}

		if(GSMM95::state == GSMINIT_STATE_DONE) {
			GSMM95::pconsole->println(F("\tGSM - INIT - GSMINIT_STATE_DONE"));
			GSMM95::pconsole->println(millis() - time);
			GSMM95::pconsole->print(F("######## OUT A : "));
			GSMM95::pconsole->println(__FUNCTION__);
			return 1;							// Init completed ... let's go ahead!
		}
		
		if ((millis() - time) > 30000) {
			GSMM95::pconsole->println(F("\tGSM - INIT - GAME OVER !!!"));
			GSMM95::pconsole->print(F("######## OUT B : "));
			GSMM95::pconsole->println(__FUNCTION__);			
			return 0; // On sort au bout de 2 minutes
		}
		
		delay(200);	// Easy... mais pas trop
		
	}
	while(GSMM95::state < GSMSTATE_INVALID);

	GSMM95::pconsole->print(F("######## OUT C : "));
	GSMM95::pconsole->println(__FUNCTION__);
	
	return 0;			// ERROR ...

}

/*----------------------------------------------------------------------------------------------------------------------------------------------------
Determine current status of the mobile module e.g. of the GSM/GPRS-Networks
All current states are returned in the string "gsmBuf"

This function can easily be extended to further queries, e.g. AT+GSN (= query IMEI), 
AT+QCCID (= query CCID), AT+CIMI (= query IMSI) etc.
 
ATTENTION: Please note length of "Status_string" - adjust if necessary

Return value = 0 ---> Error occured 
Return value = 1 ---> OK
The public variable "gsmBuf" contains the last response from the mobile module
*/
int GSMM95::Status()
{
	// char Status_string[100];

	GSMM95::pconsole->print(F("######## IN : "));
	GSMM95::pconsole->println(__FUNCTION__);

	GSMM95::state = 0;

	if(GSMM95::state == 0) {
		Serial.print(F("AT+CREG?\r"));                                               // Query register state of GSM network
		Expect(1000);
		// strcpy(Status_string, GSMM95::gsmBuf);
		GSMM95::state += 1;
	}

	if(GSMM95::state == 1) {
		Serial.print(F("AT+CGREG?\r"));                                              // Query register state of GPRS network
		Expect(1000);
		// strcat(Status_string, GSMM95::gsmBuf);
		GSMM95::state += 1;
	}

	if(GSMM95::state == 2) {
		Serial.print(F("AT+CSQ\r"));                                                 // Query the RF signal strength
		Expect(1000);
		// strcat(Status_string, GSMM95::gsmBuf);
		GSMM95::state += 1;
	}

	if(GSMM95::state == 3) {
		Serial.print(F("AT+COPS?\r"));                                               // Query the current selected operator
		Expect(1000);
		// strcat(Status_string, GSMM95::gsmBuf);
		GSMM95::state += 1;
	}

	if(GSMM95::state == 4) {
		// strcpy(GSMM95::gsmBuf, Status_string);
		GSMM95::pconsole->print(F("######## OUT A : "));
		GSMM95::pconsole->println(__FUNCTION__);
		return 1;
	}

	GSMM95::pconsole->print(F("######## OUT B : "));
	GSMM95::pconsole->println(__FUNCTION__);
	return 0;                                                                     // ERROR while dialing
}

//----------------------------------------------------------------------------------------------------------------------------------------------------
//-- G P R S + T C P / I P ---------------------------------------------------------------------------------------------------------------------------
//-- see Quectel application note "GSM_TCPIP_AN_V1.1.pdf" --------------------------------------------------------------------------------------------
/*----------------------------------------------------------------------------------------------------------------------------------------------------
Initialise GPRS connection (previously the module needs to be logged into the GSM network already)
With the successful set-up of the GPRS connection the base for processing internet (TCP(IP, UDP etc.), e-mail (SMTP) 
and PING commands is given.

Parameter:
char APN[50] = APN of the SIM card provider
char USER[30] = Username for this
char PWD[50] = Password for this

ATTENTION: This SIM card data is provider-dependent and can be obtained from them. 
           For example "internet.t-mobile.de","t-mobile","whatever" for T-Mobile, Germany
           and "gprs.vodafone.de","whatever","whatever" for Vodafone, Germany
ATTENTION: The SIM card must be suitable or enabled for GPRS data transmission. Not all SIM cards
			  (as for example very inexpensive SIM cards) are automatically enabled!!!        

Return value  = 0 ---> Error occured 
Return value = 1 ---> OK
The public variable "gsmBuf" contains the last response from the mobile module
*/
int GSMM95::Connect(const char* APN, const char* USER, const char* PWD)
{
	unsigned long time = millis();
	GSMM95::state = GSMCONNECT_STATE_START;

	GSMM95::pconsole->print(F("######## IN : "));
	GSMM95::pconsole->println(__FUNCTION__);

	if (gprsReady) {
		GSMM95::pconsole->println(F("\tGSM - CONNECT - Seems to be already connected"));
		return 1;
	}
	
	do {
 
		if(GSMM95::state == GSMCONNECT_STATE_START) {
			GSMM95::pconsole->println(F("\tGSM - CONNECT - GSMCONNECT_STATE_START"));
			GSMM95::state = GSMCONNECT_STATE_TEST_NET_REG;
		}
		
		if(GSMM95::state == GSMCONNECT_STATE_TEST_NET_REG) {
			GSMM95::pconsole->println(F("\tGSM - CONNECT - GSMCONNECT_STATE_TEST_NET_REG"));	
			Serial.print(F("AT+CREG?\r"));								// Network Registration Report
			switch(Expect(1000)) {
				case GSMSTATE_NET_REG_HOME:
				case GSMSTATE_NET_REG_ROAMING:
					GSMM95::state = GSMCONNECT_STATE_ATTACH_GPRS;
					break;
				case GSMSTATE_NET_REG_DENIED:
					GSMM95::state = GSMSTATE_INVALID;
					break;
				default:
					GSMM95::state = GSMCONNECT_STATE_TEST_NET_REG;
			}
		}

		if(GSMM95::state == GSMCONNECT_STATE_ATTACH_GPRS) {
			GSMM95::pconsole->println(F("\tGSM - CONNECT - GSMCONNECT_STATE_ATTACH_GPRS"));
			Serial.print(F("AT+CGATT=1\r"));							// attach to GPRS service
			GSMM95::state = Expect(1000) == GSMSTATE_OK ? GSMCONNECT_STATE_SET_QIFGCNT : GSMCONNECT_STATE_TEST_NET_REG;
			// need +CGATT: 1
		}
		
		if(GSMM95::state == GSMCONNECT_STATE_SET_QIFGCNT)	{
			GSMM95::pconsole->println(F("\tGSM - CONNECT - GSMINIT_STATE_SET_QIFGCNT"));
			Serial.print(F("AT+QIFGCNT=0\r"));
			GSMM95::state = Expect(1000) == GSMSTATE_OK ? GSMCONNECT_STATE_SET_PDP : GSMSTATE_INVALID;
		}

		if(GSMM95::state == GSMCONNECT_STATE_SET_PDP)   {
			GSMM95::pconsole->println(F("\tGSM - CONNECT - GSMCONNECT_STATE_SET_PDP"));
			Serial.print(F("AT+QICSGP=1,\""));				    		// Select GPRS as the bearer
			Serial.print(APN);
			Serial.print(F("\",\""));
			// Serial.print(USER);
			Serial.print(F("\",\""));
			// Serial.print(PWD);
			Serial.print(F("\"\r"));
			GSMM95::state = Expect(1000) == GSMSTATE_OK ? GSMCONNECT_STATE_DISABLE_MULTI : GSMSTATE_INVALID;
		}

		if(GSMM95::state == GSMCONNECT_STATE_DISABLE_MULTI)	{
			GSMM95::pconsole->println(F("\tGSM - CONNECT - GSMCONNECT_STATE_DISABLE_MULTI"));
			Serial.print(F("AT+QIMUX=0\r"));
			// On peut plus bouger
			if (Expect(1000) == GSMSTATE_OK) {
				GSMM95::state = GSMCONNECT_STATE_ENABLE_TRANSPARENT_MODE;
				GSMM95::qimux = true;
			} else {
				if (qimux) {
					GSMM95::state = GSMCONNECT_STATE_ENABLE_TRANSPARENT_MODE;
				} else {
					GSMM95::state = GSMSTATE_INVALID;
				}
			}
		}

		if(GSMM95::state == GSMCONNECT_STATE_ENABLE_TRANSPARENT_MODE)	{
			GSMM95::pconsole->println(F("\tGSM - CONNECT - GSMCONNECT_STATE_ENABLE_TRANSPARENT_MODE"));
			Serial.print(F("AT+QIMODE=1\r"));
			// On peut plus bouger
			if (Expect(1000) == GSMSTATE_OK) {
				GSMM95::state = GSMCONNECT_STATE_REG_APP;
			} else {
				if (qimode) {
					GSMM95::state = GSMCONNECT_STATE_REG_APP;
				} else {
					GSMM95::state = GSMSTATE_INVALID;
				}
			}
		}

		if(GSMM95::state == GSMCONNECT_STATE_REG_APP)	{
			GSMM95::pconsole->println(F("\tGSM - CONNECT - GSMCONNECT_STATE_REG_APP"));
			Serial.print(F("AT+QIREGAPP\r"));
			GSMM95::state = Expect(1000) == GSMSTATE_OK ? GSMCONNECT_STATE_TEST_ACTIVATION : GSMSTATE_INVALID;
		}
		
		if(GSMM95::state == GSMCONNECT_STATE_TEST_ACTIVATION)	{
			GSMM95::pconsole->println(F("\tGSM - CONNECT - GSMCONNECT_STATE_TEST_ACTIVATION"));
			Serial.print(F("AT+QIACT\r"));
			GSMM95::state = Expect(1000) == GSMSTATE_OK ? GSMCONNECT_STATE_DONE : GSMSTATE_INVALID;
		}

		if(GSMM95::state == GSMCONNECT_STATE_DONE)  {
			GSMM95::pconsole->println(F("\tGSM - CONNECT - GSMCONNECT_STATE_DONE"));
			GSMM95::pconsole->println(millis() - time);
			GSMM95::pconsole->print(F("######## OUT A : "));
			GSMM95::pconsole->println(__FUNCTION__);
			GSMM95::gprsReady = true;
			return 1;													// GPRS connect successfully ... let's go ahead!
		}
		
		if ((millis() - time) > 120000) {
			GSMM95::pconsole->print(F("######## OUT B : GAME OVER !!!"));
			GSMM95::pconsole->println(__FUNCTION__);
			return 0; // On sort au bout de 2 minutes
		}
		delay(500);
	}
	while(GSMM95::state < GSMSTATE_INVALID);
	
	Serial.print(F("AT+QIDEACT\r"));
	
	GSMM95::pconsole->print(F("######## OUT C : "));
	GSMM95::pconsole->println(__FUNCTION__);  
	return 0;													  		// ERROR ... no GPRS connect
}

/*----------------------------------------------------------------------------------------------------------------------------------------------------
Send HTTP GET 
This corresponds to the "call" of a URL (such as the with the internet browser) with appended parameters

Parameter:
char server[50] = Address des server (= URL)
char parameter[200] = parameters to be appended

ATTENTION: Before using this function a GPRS connection must be set up. 

You may use the "antrax Test Loop Server" for testing. All HTTP GETs to the server are logged in a list,
that can be viewed on the internet (http://www.antrax.de/WebServices/responderlist.html)

Example for a correct URL for the transmission of the information "HelloWorld":

   http://www.antrax.de/WebServices/responder.php?HelloWorld
      whereby 
	   - "www.antrax.de" is the server name 
	   - "GET /WebServices/responder.php?HelloWorld HTTP/1.1" the parameter
	
On the server the URL of the PHP script "responder.php" is accepted and analysed in the subdirectory "WebServices". 
The part after the "?" corresponds to the transmitted parameters. ATTENTION: The parameters must not 
contain spaces. The source code of the PHP script "responder.php" is located in the documentation. 

Return value = 0 ---> Error occured 
Return value = 1 ---> OK
The public variable "gsmBuf" contains the last response from the mobile module
*/
int GSMM95::SendHttpReq(const char* server, const char* port, char* parameter)
{
	unsigned long time = millis();

	GSMM95::pconsole->print(F("######## IN : "));
	GSMM95::pconsole->println(__FUNCTION__); 

	GSMM95::state = 0;

/*
	do {
		
		if(GSMM95::state == 0)  {
			Serial.print(F("AT+QIOPEN=\"TCP\",\""));		    								     // Start up TCP connection
			Serial.print(server);
			Serial.print('"');
			Serial.print(port);
			Serial.print('\r');
			if(Expect(2000) == 1) { GSMM95::state += 1; } else { GSMM95::state = GSMSTATE_INVALID; }      // need CONNECT
		}

		if(GSMM95::state == 1)  {
			if(Expect(20000) == 9) { GSMM95::state += 1; } else { GSMM95::state = GSMSTATE_INVALID; }     // need CONNECT OK or ALREADY CONNECT
		}

		if(GSMM95::state == 2) {
			Serial.print(F("AT+QISEND\r"));                                              // Send data to the remote server
			if(Expect(5000) == 5) { GSMM95::state += 1; } else { GSMM95::state = GSMSTATE_INVALID; } 	  // get the prompt ">"
		}

		if(GSMM95::state == 3) {
			// for HTTP GET must include: "GET /subdirectory/name.php?test=parameter_to_transmit HTTP/1.1"
			// "GET /WebServices/responder.php?test=HelloWorld HTTP/1.1"
			Serial.print(parameter);   // Message-Text

			// Header Field Definitions in http://www.w3.org/Protocols/rfc2616/rfc2616-sec14.html
			Serial.print(F("\r\nHost: "));                                               // Header Field "Host"
			Serial.print(server);
			// Serial.print(F("\r\nUser-Agent: AVEZE"));                                   // Header Field "User-Agent" (MUST be "antrax" when use with portal "WebServices")
			Serial.print(F("\r\nConnection: close\r\n\r\n"));                            // Header Field "Connection"
			Serial.write(26);                                                         // CTRL-Z
			if(Expect(20000) == 10) { GSMM95::state += 1; } else { GSMM95::state = GSMSTATE_INVALID; }    // Congratulations ... the parameter_string was send to the server
		}

		if(GSMM95::state == 4) {
			Expect(5000);                    // wait of ack from remote server
			GSMM95::state += 1;
		}

		if(GSMM95::state == 5) {
			GSMM95::pconsole->print(F("######## OUT A : "));
			GSMM95::pconsole->println(__FUNCTION__); 
			return 1;				  // GPRS connect successfully ... let's go ahead!
		}

	}
	while(GSMM95::state < GSMSTATE_INVALID);
*/

	int l = strlen(parameter);
	Serial.print(F("AT+QHTTPURL="));
	Serial.print(l);					// longueur de l'url complete
	Serial.print(F(",30\r"));			// tmout en s
	
	// Need connect
	Expect(10000);
	
	Serial.print(parameter);
	Serial.print(F("\r"));
	
	// Need OK
	Expect(10000);
	
	Serial.print(F("AT+QHTTPGET=60\r"));
	Expect(10000);
	
	Serial.print(F("AT+QHTTPREAD=30\r"));
	Expect(10000);	
	
	GSMM95::pconsole->print(F("######## OUT A : "));
	GSMM95::pconsole->println(__FUNCTION__); 
	return 1;
}


/*----------------------------------------------------------------------------------------------------------------------------------------------------
Disconnect GPRS connection

ATTENTION: The frequent disconnection and rebuilding of GPRS connections can lead to unnecessarily high charges 
		 	  (e.g. due to "rounding up costs"). It is necessary to consider whether a GPRS connection for a longer time period 
			  (without data transmission) shall remain active! 

No return value 
The public variable "gsmBuf" contains the last response from the mobile module
*/
void GSMM95::Disconnect()
{
	GSMM95::pconsole->print(F("######## IN : "));
	GSMM95::pconsole->println(__FUNCTION__); 
	// GSMM95::state = 0;
	// do
	// {
	//  if(GSMM95::state == 0)
	//  {
      Serial.print(F("AT+QIDEACT\r"));		// Deactivate GPRS context
      // if(Expect(10000) == 1) { GSMM95::state += 1; } else { GSMM95::state = GSMSTATE_INVALID; }
      // if(Expect(10000) == 1) { break; } else { GSMM95::state = GSMSTATE_INVALID; }

	//  }
	// }
	// while(GSMM95::state < GSMSTATE_INVALID);
	GSMM95::pconsole->print(F("######## OUT : "));
	GSMM95::pconsole->println(__FUNCTION__);
}

 
//----------------------------------------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------------------------------------
/*----------------------------------------------------------------------------------------------------------------------------------------------------
Central receive routine of the serial interface

ATTENTION: This function is used by all other functions (see above) and must not be deleted

Parameter:
int TIMEOUT = Waiting time to receive the first character (in milliseconds)


Then contents of the received characters is written to the variable "gsmBuf". If no characters is received 
for 30 milliseconds, the reception is completed and the contents of "gsmBuf" is being analysed.

ATTENTION: The length of the reaction times of the mobile module depend on the condition of the mobile module, for example  
			  quality of wireless connection, provider, etc. and thus can vary. Please keep this in mind in case this routine is 
			  is changed.

Return value = 0      ---> No known response of the mobile module detected 
Rckgabewert = 1 - 18 ---> Response detected (see below)
The public variable "gsmBuf" contains the last response from the mobile module
*/
int GSMM95::Expect(int timeout)
{
  int  index = 0;
  int  inByte = 0;
  char WS[3];
  char expectBuf[MAX_GSM_STRING_SZ];

  GSMM95::pconsole->print(F("######## IN : "));
  GSMM95::pconsole->println(__FUNCTION__);

  //----- erase gsmBuf
  memset(gsmBuf, 0, GSM_BUFSZ);
  memset(WS, 0, 3);

  //----- clear Serial Line Buffer
  // while(Serial.available()) { Serial.read(); }
  
  //----- wait of the first character for "timeout" ms
  Serial.setTimeout(timeout);
  inByte = Serial.readBytes(WS, 1);
  gsmBuf[index++] = WS[0];
  
  //----- wait of further characters until a pause of 30 ms occures
  while(inByte > 0)
  {
    Serial.setTimeout(30);
    inByte = Serial.readBytes(WS, 1);
    gsmBuf[index++] = WS[0];
  }

  //----- analyse the reaction of the mobile module
  for (byte i=0; i < MAX_GSM_STRINGS; i++)
  {
	strcpy_P(expectBuf, (char*)pgm_read_word(&(gsmStrings[i])));
	if(strstr(gsmBuf, expectBuf))
	{
	  GSMM95::pconsole->print(F("\tFound : "));
	  GSMM95::pconsole->print(expectBuf);
	  GSMM95::pconsole->print(F(" in : "));
	  GSMM95::pconsole->println(GSMM95::gsmBuf);
	  GSMM95::pconsole->print(F("\tTransition state is : "));
	  GSMM95::pconsole->println(i);
	  GSMM95::pconsole->print(F("######## OUT A : "));
	  GSMM95::pconsole->println(__FUNCTION__); 
	  return i;
	}
  }

  GSMM95::pconsole->print(F("######## OUT B : "));
  GSMM95::pconsole->println(__FUNCTION__); 
  return 0;
}        

//----------------------------------------------------------------------------------------------------------------------------------------------------

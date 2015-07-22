#include <GSMM95.h>

GSMM95::GSMM95(byte myPwrKey, SoftwareSerial* pSerial)
{
  GSMM95::state = 0;
  GSMM95::pconsole = pSerial;
  GSMM95::pwrKey = myPwrKey;
  pinMode(GSMM95::pwrKey, OUTPUT);
}


/*----------------------------------------------------------------------------------------------------------------------------------------------------
Initialisation of the GSM-easy! - Shield:
- Set data rate 
- Activate shield 
- Perform init-sequence of the M95 
- register the M95 in the GSM network

Return value = 0 ---> Error occured 
Return value = 1 ---> OK
The public variable "gsmBuf" contains the last response from the mobile module
*/
int GSMM95::Init(const char* pinCode)
{
	unsigned long time = millis();	// On donne un temps limit� � l'init

	// Init sequence, see "M95_HardwareDesign_V1.2.pdf", page 30.
	// Reset!
	digitalWrite(GSMM95::pwrKey, LOW);
	delay(2100);

	digitalWrite(GSMM95::pwrKey, HIGH);
	delay(5000);

	// Start and Autobauding sequence ("M95_AT_Commands_V1.0.pdf", page 35) 
	Serial.begin(GSM_BAUDRATE);
	for (int i=0; i < 3; i++)
	{
		Serial.print('A');
		delay(500);
		Serial.print('T');
		delay(500);
		Serial.print('\r');
		delay(500);
	}

	GSMM95::state = GSMINIT_STATE_START;
	time = 0;
	do
	{
		if(GSMM95::state == GSMINIT_STATE_START)
		{
			Serial.print(F("AT\r"));     
			if(Expect(1000) == GSMSTATE_OK)
			{
				GSMM95::state = GSMINIT_STATE_MODEM_OK;
			}
			else
			{
				GSMM95::state = GSMSTATE_INVALID;
			}
		}

		if(GSMM95::state == GSMINIT_STATE_MODEM_OK)
		{
			Serial.print(F("ATE0\r"));
			if(Expect(1000) == GSMSTATE_OK)
			{
				GSMM95::state = GSMINIT_STATE_ECHO_DISABLED; 
			}
			else
			{ 
				GSMM95::state = GSMSTATE_INVALID; 
			}
		}
	
		if(GSMM95::state == GSMINIT_STATE_ECHO_DISABLED)
		{								// after 0,5 - 10 sec., depends of the SIM card
			switch (Expect(10000)) 		// wait for initial URC presentation "+CPIN: SIM PIN" or similar
			{                                                                         
				case GSMSTATE_PIN_REQ:  
					GSMM95::state = GSMINIT_STATE_PIN_REQUIRED; // get +CPIN: SIM PIN
					break; 													     
				case 3:  
					GSMM95::state = GSMINIT_STATE_SIM_OK; // get +CPIN: READY
					break;												           
				default: 
					GSMM95::state = GSMINIT_STATE_SIM_STATUS_SECOND_CHANCE;	// Sinon...
					break;
			}
		}
	
		if(GSMM95::state == GSMINIT_STATE_SIM_STATUS_SECOND_CHANCE)
		{
		  switch (Expect(10000)) 		// new try: wait for initial URC presentation "+CPIN: SIM PIN" or similar
		   {                                                                         
			 case GSMSTATE_PIN_REQ: GSMM95::state = GSMINIT_STATE_SIM_STATUS_SECOND_CHANCE; break; 		// get +CPIN: SIM PIN
			 case GSMSTATE_PIN_RDY: GSMM95::state = GSMINIT_STATE_SIM_OK; break;		// get +CPIN: READY
			 default: 
			 { 
				  GSMM95::pconsole->print(F("SIM ERREUR FATALE : >"));
				  GSMM95::pconsole->print(GSMM95::gsmBuf);    //    here is the explanation
			      GSMM95::pconsole->println(F("<"));
			      return 0;
			 }  
			}  
		}
	
		if(GSMM95::state == GSMINIT_STATE_PIN_REQUIRED)
		{
		  Serial.print(F("AT+CPIN="));           // enter pin (SIM)     
		  Serial.print(pinCode);
		  Serial.print('\r');
		  if(Expect(1000) == GSMSTATE_PIN_RDY) { GSMM95::state = GSMINIT_STATE_SIM_OK; } else { GSMM95::state = GSMSTATE_INVALID; } 
		}
	
		if(GSMM95::state == GSMINIT_STATE_SIM_OK)
		{
		  Serial.print(F("AT+IPR="));        // set Baudrate
		  Serial.print(GSM_BAUDRATE);
		  Serial.print('\r');
		  if(Expect(1000) == GSMSTATE_OK) { GSMM95::state = GSMINIT_STATE_BAUDRATE_FIXED; } else { GSMM95::state = GSMSTATE_INVALID; } 
		}
	
		if(GSMM95::state == GSMINIT_STATE_BAUDRATE_FIXED)
		{
			Serial.print(F("AT+QIURC=0\r"));    // disable initial URC presentation   
			time = 0;  
			if(Expect(1000) == 1) { GSMM95::state = GSMINIT_STATE_URC_DISABLED; } else { GSMM95::state = GSMSTATE_INVALID; } 
		}
	
		if(GSMM95::state == GSMINIT_STATE_URC_DISABLED)
		{
		  delay(2000);                                                                                              
		  Serial.print(F("AT+CREG?\r"));        // Network Registration Report
		  int ret_reg = Expect(1000);
		  if ((ret_reg == GSMSTATE_NET_REG_2G) || (ret_reg == GSMSTATE_NET_REG_3G))																	
		   { 
			 GSMM95::state = GSMINIT_STATE_REGISTERED; 	// get: Registered in home network or roaming
		   } 
		   else 
		   { 
			 delay(2000);
			  if(time++ < 30)
			  {
				GSMM95::state = GSMM95::state;	// stay in this state until timeout
			  }
			  else
			  {
				GSMM95::state = GSMSTATE_INVALID;// after 60 sek. (30 x 2000 ms) not registered	
			  } 
		  } 
		}
		  
		if(GSMM95::state == GSMINIT_STATE_REGISTERED)
		{
		  return 1;								// Registered successfully ... let's go ahead!
		}
		GSMM95::pconsole->print(F("GSM - INIT : "));
		GSMM95::pconsole->println(GSMM95::state);
		if ((millis() - time) > 120000) return 0; // On sort au bout de 2 minutes
		delay(500);		delay(500);								// Easy...
	}
	while(GSMM95::state < GSMSTATE_INVALID);

	return 0;			// ERROR ... no Registration in the network
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
  char Status_string[100];
  
  GSMM95::state = 0;
  do    
  {            
    if(GSMM95::state == 0)
    {
      Serial.print(F("AT+CREG?\r"));                                               // Query register state of GSM network
      Expect(1000);
	  strcpy(Status_string, GSMM95::gsmBuf);
	  GSMM95::state += 1; 
	}
    
    if(GSMM95::state == 1)
    {
      Serial.print(F("AT+CGREG?\r"));                                              // Query register state of GPRS network
      Expect(1000);
	  strcat(Status_string, GSMM95::gsmBuf);
	  GSMM95::state += 1; 
	}

    if(GSMM95::state == 2)
    {
      Serial.print(F("AT+CSQ\r"));                                                 // Query the RF signal strength
      Expect(1000);
	  strcat(Status_string, GSMM95::gsmBuf);
	  GSMM95::state += 1; 
	}
	 
    if(GSMM95::state == 3)
    {
      Serial.print(F("AT+COPS?\r"));                                               // Query the current selected operator
      Expect(1000);
	  strcat(Status_string, GSMM95::gsmBuf);
	  GSMM95::state += 1; 
	}

    if(GSMM95::state == 4)
    {	
		strcpy(GSMM95::gsmBuf, Status_string); 
		return 1;
    } 
  }  
  while(GSMM95::state <= 999);
  
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
  do
  {
    if(GSMM95::state == GSMCONNECT_STATE_START)
	{
      Serial.print(F("AT+CREG?\r"));		// Network Registration Report
	  int ret_reg = Expect(1000);
	  if ((ret_reg == GSMSTATE_NET_REG_2G) || (ret_reg == GSMSTATE_NET_REG_3G))
	  {
		  GSMM95::state = GSMCONNECT_STATE_REGISTERED;
	  } else
	  {
		  GSMM95::state = GSMSTATE_INVALID;
	  }
	}

    if(GSMM95::state == GSMCONNECT_STATE_REGISTERED)					// Judge network?
    {
      Serial.print(F("AT+CGATT?\r"));		// attach to GPRS service?      
      if(Expect(1000) == GSMSTATE_GPRS_ATTACHED) 				// need +CGATT: 1			
	   { 
	     GSMM95::state = GSMCONNECT_STATE_GPRS_ATTACHED; 				// get: attach
	   } 
	   else 
	   { 
	     delay(2000);
		  if(time++ < 30)																		  	   
		  {
		    GSMM95::state = GSMM95::state;	// stay in this state until timeout
		  }
		  else
		  {
		    GSMM95::state = GSMSTATE_INVALID;			// after 60 sek. (30 x 2000 ms) not attach	
		  } 
      }
	 } 

    if(GSMM95::state == GSMCONNECT_STATE_GPRS_ATTACHED)
    {
      Serial.print(F("AT+QISTAT\r"));                                              // Query current connection status
      if(Expect(1000) == 8) { GSMM95::state = GSMCONNECT_STATE_IP_INITIAL; } else { GSMM95::state = GSMSTATE_INVALID; }      // need STATE: IP INITIAL 
    }
    
    if(GSMM95::state == GSMCONNECT_STATE_IP_INITIAL)
    {
      Serial.print(F("AT+QICSGP=1,\""));				    								     // Select GPRS as the bearer
      Serial.print(APN);
	  Serial.print(F("\",\""));
	  Serial.print(USER);
	  Serial.print(F("\",\""));
	  Serial.print(PWD);
	  Serial.print(F("\"\r"));
      if(Expect(1000) == GSMSTATE_OK) { GSMM95::state = GSMCONNECT_STATE_APN_SET; } else { GSMM95::state = GSMSTATE_INVALID; }      // need OK 
    }
	 
    if(GSMM95::state == GSMCONNECT_STATE_APN_SET)
    {
      Serial.print(F("AT+QIDNSIP=1\r"));                                           // Connect via domain name (not via IP address!)
      if(Expect(1000) == GSMSTATE_OK) { GSMM95::state = GSMCONNECT_STATE_DOMAIN_SET; } else { GSMM95::state = GSMSTATE_INVALID; }      // need OK 
    }
	 
    if(GSMM95::state == GSMCONNECT_STATE_DOMAIN_SET)
    {
      Serial.print(F("AT+QISTAT\r"));                                              // Query current connection status
      if(Expect(1000) == 8) { GSMM95::state = GSMCONNECT_STATE_DONE; } else { GSMM95::state = GSMSTATE_INVALID; }      // need STATE: IP INITIAL, IP STATUS or IP CLOSE
    }

    if(GSMM95::state == GSMCONNECT_STATE_DONE)
    {
      return 1;																					  // GPRS connect successfully ... let's go ahead!
    }
    GSMM95::pconsole->print(F("GSM - CONNECT : "));
    GSMM95::pconsole->println(GSMM95::state);
	if ((millis() - time) > 120000) return 0; // On sort au bout de 2 minutes
	delay(500);
  } 
  while(GSMM95::state < GSMSTATE_INVALID);
  
  return 0;																							  // ERROR ... no GPRS connect
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
  
  GSMM95::state = 0;
  do
  {
    if(GSMM95::state == 0)
    {
      Serial.print(F("AT+QIOPEN=\"TCP\",\""));		    								     // Start up TCP connection
      Serial.print(server);
	  Serial.print('"');
	  Serial.print(port);
	  Serial.print('\r');
      if(Expect(2000) == 1) { GSMM95::state += 1; } else { GSMM95::state = GSMSTATE_INVALID; }      // need OK
    }
	 
    if(GSMM95::state == 1)
    {
      if(Expect(20000) == 9) { GSMM95::state += 1; } else { GSMM95::state = GSMSTATE_INVALID; }     // need CONNECT OK or ALREADY CONNECT
    }

    if(GSMM95::state == 2)
    {
      Serial.print(F("AT+QISEND\r"));                                              // Send data to the remote server
      if(Expect(5000) == 5) { GSMM95::state += 1; } else { GSMM95::state = GSMSTATE_INVALID; } 	  // get the prompt ">"
    }
      
    if(GSMM95::state == 3)
    {
      // for HTTP GET must include: "GET /subdirectory/name.php?test=parameter_to_transmit HTTP/1.1"
      // "GET /WebServices/responder.php?test=HelloWorld HTTP/1.1"
	  Serial.print(parameter);   // Message-Text
      
      // Header Field Definitions in http://www.w3.org/Protocols/rfc2616/rfc2616-sec14.html
      Serial.print(F("\r\nHost: "));                                               // Header Field "Host"
      Serial.print(server);                                                     
      Serial.print(F("\r\nUser-Agent: AVEZE"));                                   // Header Field "User-Agent" (MUST be "antrax" when use with portal "WebServices")
      Serial.print(F("\r\nConnection: close\r\n\r\n"));                            // Header Field "Connection"
      Serial.write(26);                                                         // CTRL-Z 
      if(Expect(20000) == 10) { GSMM95::state += 1; } else { GSMM95::state = GSMSTATE_INVALID; }    // Congratulations ... the parameter_string was send to the server
    } 

    if(GSMM95::state == 4)
    {
      Expect(5000);                    // wait of ack from remote server
	  GSMM95::state += 1;
    }
	 
    if(GSMM95::state == 5)
    {
      return 1;				  // GPRS connect successfully ... let's go ahead!
    }

  } 
  while(GSMM95::state < GSMSTATE_INVALID);
  
  return 0;					 // ERROR ... no GPRS connect
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
R�ckgabewert = 1 - 18 ---> Response detected (see below)
The public variable "gsmBuf" contains the last response from the mobile module
*/
int GSMM95::Expect(int timeout)
{
  int  index = 0;
  int  inByte = 0;
  char WS[3];

  //----- erase gsmBuf
  memset(gsmBuf, 0, GSM_BUFSZ);
  memset(WS, 0, 3);

  //----- clear Serial Line Buffer
  while(Serial.available()) { Serial.read(); }
  
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
	if(strstr(gsmBuf, gsmStrings[i]))
	{
	  return i+1;
	}
  }

  return 0;
}        
      
//----------------------------------------------------------------------------------------------------------------------------------------------------


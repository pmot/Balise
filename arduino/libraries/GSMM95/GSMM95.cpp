#include <GSMM95.h>

int state = 0;


GSMM95::GSMM95()
{
  pinMode(GSM_PWRK, OUTPUT);
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
	int time;

	Serial.begin(GSM_BAUDRATE);																// 9600 Baud data rate

	// digitalWrite(GSM_ON, HIGH);														// GSM_ON = HIGH --> switch on + Serial Line enable

	// Init sequence, see "M95_HardwareDesign_V1.2.pdf", page 30ff.
	// in any case: force a reset!
	// digitalWrite(PWRKEY, LOW);														// PWRKEY = HIGH
	// digitalWrite(EMERG, HIGH);														// EMERG_OFF = LOW
	delay(50);																		// wait for 50ms

	// digitalWrite(PWRKEY, LOW);														// PWRKEY = HIGH
	// digitalWrite(EMERG, LOW);														// EMERG_OFF = HIGH
	delay(2100);																	// wait for 2100ms

	// digitalWrite(PWRKEY, HIGH);														// PWRKEY = LOW
	delay(1100);																	// wait for 1100ms
	// digitalWrite(PWRKEY, LOW);														// PWRKEY = HIGH

	// Start and Autobauding sequence ("M95_AT_Commands_V1.0.pdf", page 35)
	delay(3000);																	// wait for 3000ms 

	state = 0;
	time = 0;
	do
	{
	if(state == 0)
	{
		Serial.print(F("AT\r"));                                                     	// send the first "AT"      
		if(Expect(1000) == 1) 
		{ 
			state += 1; 
		} 
		else 
		{ 
			state = 1000; 															// need OK 
		} 
	}

	if(state == 1)
	{
		Serial.print(F("ATE0\r"));                                                   	// disable Echo   
		if(Expect(1000) == 1) 
		{ 
			state += 1; 
		} 
		else 
		{ 
			state = 1000; 
		} 
	}

	if(state == 2)
	{																				// after 0,5 - 10 sec., depends of the SIM card
		switch (Expect(10000)) 												// wait for initial URC presentation "+CPIN: SIM PIN" or similar
		{                                                                         
			case 2:  
				state += 2; 														// get +CPIN: SIM PIN
				break; 													     
			case 3:  
				state += 3; 														// get +CPIN: READY
				break;												           
			default: 
				state += 1; 
				break;
		}
	}

	if(state == 3)
	{
	  switch (Expect(10000)) 		// new try: wait for initial URC presentation "+CPIN: SIM PIN" or similar
	   {                                                                         
		 case 2: state += 1; break; 		// get +CPIN: SIM PIN
		 case 3: state += 2; break;			// get +CPIN: READY
		  default: 
		  { 
			  Serial.print(F("Serious SIM-error: >"));
		   Serial.print(GSMM95::gsmBuf);    //    here is the explanation
		   Serial.println(F("<"));
		   while(1);				  		// ATTENTION: check your SIM!!!! Don't restart the software!!!
		 }  
		}  
	}

	if(state == 4)
	{
	  Serial.print(F("AT+CPIN="));           // enter pin (SIM)     
	  Serial.print(pinCode);
	  Serial.print('\r');
	  if(Expect(1000) == 3) { state += 1; } else { state = 1000; } 
	}

	if(state == 5)
	{
	  Serial.print(F("AT+IPR=9600\r"));        // set Baudrate
	  if(Expect(1000) == 1) { state += 1; } else { state = 1000; } 
	}

	if(state == 6)
	{
		Serial.print(F("AT+QIURC=0\r"));       // disable initial URC presentation   
		time = 0;  
		if(Expect(1000) == 1) { state += 1; } else { state = 1000; } 
	}

	if(state == 7)
	{
	  delay(2000);                                                                                              
	  Serial.print(F("AT+CREG?\r"));           // Network Registration Report      
	  if(Expect(1000) == 4) 																		
	   { 
		 state += 1; 						// get: Registered in home network or roaming
	   } 
	   else 
	   { 
		 delay(2000);
		  if(time++ < 30)
		  {
			state = state;					// stay in this state until timeout
		  }
		  else
		  {
			state = 1000;					// after 60 sek. (30 x 2000 ms) not registered	
		  } 
	  } 
	}
	  
	if(state == 8)
	{
	  return 1;								// Registered successfully ... let's go ahead!
	}
	}
	while(state <= 999);

	return 0;								// ERROR ... no Registration in the network
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
  
  state = 0;
  do    
  {            
    if(state == 0)
    {
      Serial.print(F("AT+CREG?\r"));                                               // Query register state of GSM network
      Expect(1000);
		strcpy(Status_string, gsmBuf);
		state += 1; 
	 }
    
    if(state == 1)
    {
      Serial.print(F("AT+CGREG?\r"));                                              // Query register state of GPRS network
      Expect(1000);
		strcat(Status_string, gsmBuf);
		state += 1; 
	 }

    if(state == 2)
    {
      Serial.print(F("AT+CSQ\r"));                                                 // Query the RF signal strength
      Expect(1000);
		strcat(Status_string, gsmBuf);
		state += 1; 
	 }
	 
    if(state == 3)
    {
      Serial.print(F("AT+COPS?\r"));                                               // Query the current selected operator
      Expect(1000);
		strcat(Status_string, gsmBuf);
		state += 1; 
	 }

    if(state == 4)
    {	
		strcpy(gsmBuf, Status_string); 
		return 1;
    } 
  }  
  while(state <= 999);
  
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
  int time = 0;
  
  state = 0;
  do
  {
    if(state == 0)
	 {
      Serial.print(F("AT+CREG?\r"));                                               // Network Registration Report
      if(Expect(1000) == 4) { state += 1; } else { state = 1000; }      // need 0,1 or 0,5
	 }

    if(state == 1)                                                              // Judge network?
    {
      Serial.print(F("AT+CGATT?\r"));                                              // attach to GPRS service?      
      if(Expect(1000) == 7) 														  // need +CGATT: 1			
	   { 
	     state += 1; 																			     // get: attach
	   } 
	   else 
	   { 
	     delay(2000);
		  if(time++ < 30)																		  	   
		  {
		    state = state;																	     // stay in this state until timeout
		  }
		  else
		  {
		    state = 1000;																		     // after 60 sek. (30 x 2000 ms) not attach	
		  } 
      }
	 } 

    if(state == 2)
    {
      Serial.print(F("AT+QISTAT\r"));                                              // Query current connection status
      if(Expect(1000) == 8) { state += 1; } else { state = 1000; }      // need STATE: IP INITIAL 
    }
    
    if(state == 3)
    {
      Serial.print(F("AT+QICSGP=1,\""));				    								     // Select GPRS as the bearer
      Serial.print(APN);
		Serial.print(F("\",\""));
		Serial.print(USER);
		Serial.print(F("\",\""));
		Serial.print(PWD);
		Serial.print(F("\"\r"));
      if(Expect(1000) == 1) { state += 1; } else { state = 1000; }      // need OK 
    }
	 
    if(state == 4)
    {
      Serial.print(F("AT+QIDNSIP=1\r"));                                           // Connect via domain name (not via IP address!)
      if(Expect(1000) == 1) { state += 1; } else { state = 1000; }      // need OK 
    }
	 
    if(state == 5)
    {
      Serial.print(F("AT+QISTAT\r"));                                              // Query current connection status
      if(Expect(1000) == 8) { state += 1; } else { state = 1000; }      // need STATE: IP INITIAL, IP STATUS or IP CLOSE
    }

    if(state == 6)
    {
      return 1;																					  // GPRS connect successfully ... let's go ahead!
    }

  } 
  while(state <= 999);
  
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
int GSMM95::SendHttpReq(const char* server, char parameter[200])
{
  int time = 0;
  
  state = 0;
  do
  {
    if(state == 0)
    {
      Serial.print(F("AT+QIOPEN=\"TCP\",\""));		    								     // Start up TCP connection
      Serial.print(server);
		Serial.print(F("\",80\r"));
      if(Expect(2000) == 1) { state += 1; } else { state = 1000; }      // need OK
    }
	 
    if(state == 1)
    {
      if(Expect(20000) == 9) { state += 1; } else { state = 1000; }     // need CONNECT OK or ALREADY CONNECT
    }

    if(state == 2)
    {
      Serial.print(F("AT+QISEND\r"));                                              // Send data to the remote server
      if(Expect(5000) == 5) { state += 1; } else { state = 1000; } 	  // get the prompt ">"
    }
      
    if(state == 3)
    {
      // for HTTP GET must include: "GET /subdirectory/name.php?test=parameter_to_transmit HTTP/1.1"
      // for example to use with "www.antrax.de/WebServices/responderlist.html":
      // "GET /WebServices/responder.php?test=HelloWorld HTTP/1.1"
		Serial.print(parameter);                                           // Message-Text
      
      // Header Field Definitions in http://www.w3.org/Protocols/rfc2616/rfc2616-sec14.html
      Serial.print(F("\r\nHost: "));                                               // Header Field "Host"
      Serial.print(server);                                                     
      Serial.print(F("\r\nUser-Agent: Balise"));                                   // Header Field "User-Agent" (MUST be "antrax" when use with portal "WebServices")
      Serial.print(F("\r\nConnection: close\r\n\r\n"));                            // Header Field "Connection"
      Serial.write(26);                                                         // CTRL-Z 
      if(Expect(20000) == 10) { state += 1; } else { state = 1000; }    // Congratulations ... the parameter_string was send to the server
    } 

    if(state == 4)
    {
      Expect(5000);                                                     // wait of ack from remote server
		state += 1;
    }
	 
    if(state == 5)
    {
      return 1;																					  // GPRS connect successfully ... let's go ahead!
    }

  } 
  while(state <= 999);
  
  return 0;																							  // ERROR ... no GPRS connect
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
  state = 0;
  do
  {
    if(state == 0)
    {
      Serial.print(F("AT+QIDEACT\r"));                                             // Deactivate GPRS context
      if(Expect(10000) == 1) { state += 1; } else { state = 1000; }     // ein OK wäre schön ...
    }
  }
  while(state <= 999);  
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
Rückgabewert = 1 - 18 ---> Response detected (see below)
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
  if(strstr(gsmBuf, "OK\r\n"))					{ return 1; }
  if(strstr(gsmBuf, "SIM PIN\r\n"))	      		{ return 2; }
  if(strstr(gsmBuf, "READY\r\n"))		    	{ return 3; }
  if(strstr(gsmBuf, "0,1\r\n"))					{ return 4; }
  if(strstr(gsmBuf, "0,5\r\n"))					{ return 4; }
  if(strstr(gsmBuf, "NO CARRIER\r\n"))			{ return 6; }
  if(strstr(gsmBuf, "+CGATT: 1\r\n"))			{ return 7; }
  if(strstr(gsmBuf, "IP INITIAL\r\n"))			{ return 8; }
  if(strstr(gsmBuf, "IP STATUS\r\n"))			{ return 8; }
  if(strstr(gsmBuf, "IP CLOSE\r\n"))			{ return 8; }
  if(strstr(gsmBuf, "CONNECT OK\r\n"))			{ return 9; }
  if(strstr(gsmBuf, "ALREADY CONNECT\r\n"))		{ return 9; }
  if(strstr(gsmBuf, "SEND OK\r\n"))				{ return 10; }
  if(strstr(gsmBuf, "+CPMS:"))				   	{ return 13; }
  if(strstr(gsmBuf, "OK\r\n\r\nCONNECT\r\n"))	{ return 14; }
  if(strstr(gsmBuf, ":0\r\n"))		    	    { return 17; }

 
  
  return 0;
}        
      
//----------------------------------------------------------------------------------------------------------------------------------------------------


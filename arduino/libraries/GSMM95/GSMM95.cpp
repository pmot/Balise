#include <GSMM95.h>


void gsmSetup(struct gsmContext* pGsmContext, SoftwareSerial* pSerial)
{
  pGsmContext->state = GSMINIT_STATE_START;
  pGsmContext->pConsole = pSerial;
  pGsmContext->qiMux = false;
  pGsmContext->qiMode = false;
  pGsmContext->gprsReady = false;
}

void gsmHardReset(struct gsmContext* pGsmContext, byte pwrKey)
{

	pGsmContext->pConsole->print(F("######## IN : "));
	pGsmContext->pConsole->println(__FUNCTION__);

	// On repart en init
	pGsmContext->state = GSMINIT_STATE_START;
	pGsmContext->qiMux = false;
	pGsmContext->qiMode = false;
	pGsmContext->gprsReady = false;
	
	// Init sequence, see "M95_HardwareDesign_V1.2.pdf", page 30.
	pinMode(pwrKey, OUTPUT);
	// Reset!
	pGsmContext->pConsole->println(F("\tGSM - INIT - Power on the modem"));
	digitalWrite(pwrKey, LOW);
	delay(500);
	digitalWrite(pwrKey, HIGH);
	delay(5000);
	pGsmContext->pConsole->println(F("\tGSM - INIT - Done"));
	
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
	
	delay(5000);
	
    // Clear buffer !!!
    pGsmContext->pConsole->println(F("\tGSM - PowerOnModem - Flushing the buffer : "));
    Serial.setTimeout(5000);
    while (Serial.available()) pGsmContext->pConsole->println(Serial.read());
    pGsmContext->pConsole->println(F("\tGSM - PowerOnModem - Done"));

	pGsmContext->pConsole->print(F("######## OUT : "));
	pGsmContext->pConsole->println(__FUNCTION__);
}  

/*
 * Initialisation du M95:
	- Unlock de la sim
	- Sequence d'init (qos, echo, etc.)

Retourne :
* 0 ---> Error 
* 1 ---> OK 

"gsmBuf" contient la derniere reponse du module
*/
int gsmInit(struct gsmContext* pGsmContext, const char* pinCode)
{
	char gsmBuf[GSM_BUFSZ];
	unsigned long time = millis();	// On donne un temps limite l'init
	
	pGsmContext->pConsole->print(F("######## IN : "));
	pGsmContext->pConsole->println(__FUNCTION__);

	pGsmContext->state = GSMINIT_STATE_START;

	do {

		if(pGsmContext->state == GSMINIT_STATE_START) {
			pGsmContext->pConsole->println(F("\tGSM - INIT - Begin Loop state"));
			pGsmContext->state = GSMINIT_STATE_SET_BAUDRATE;
			delay(500);
		}

		if(pGsmContext->state == GSMINIT_STATE_SET_BAUDRATE) {
			pGsmContext->pConsole->println(F("\tGSM - INIT - Actual state : GSMINIT_STATE_SET_BAUDRATE"));
			Serial.print(F("AT+IPR="));        // set Baudrate
			Serial.print(GSM_BAUDRATE);
			Serial.print('\r');
			pGsmContext->state = gsmExpect(pGsmContext, gsmBuf, 1000) == GSMSTATE_IPR ? GSMINIT_STATE_TEST_MODEM : GSMINIT_STATE_SET_BAUDRATE;
		}

		if(pGsmContext->state == GSMINIT_STATE_TEST_MODEM) {
			pGsmContext->pConsole->println(F("\tGSM - INIT - GSMINIT_STATE_TEST_MODEM"));
			Serial.print(F("AT\r"));     			
			pGsmContext->state = gsmExpect(pGsmContext, gsmBuf, 1000) == GSMSTATE_OK ? GSMINIT_STATE_DISABLE_ECHO : GSMINIT_STATE_TEST_MODEM;
		}

		if(pGsmContext->state == GSMINIT_STATE_DISABLE_ECHO)	{
			pGsmContext->pConsole->println(F("\tGSM - INIT - GSMINIT_STATE_DISABLE_ECHO"));
			Serial.print(F("ATE0\r"));
			pGsmContext->state = gsmExpect(pGsmContext, gsmBuf, 1000) == GSMSTATE_OK ? GSMINIT_STATE_DISABLE_URC : GSMSTATE_INVALID;
		}

		if(pGsmContext->state == GSMINIT_STATE_DISABLE_URC) {
			pGsmContext->pConsole->println(F("\tGSM - INIT - GSMINIT_STATE_DISABLE_URC"));
			Serial.print(F("AT+QIURC=0\r"));    // disable initial URC presentation   
			pGsmContext->state = gsmExpect(pGsmContext, gsmBuf, 1000) == GSMSTATE_OK ? GSMINIT_STATE_ENABLE_CREG : GSMSTATE_INVALID;
		}

		if(pGsmContext->state == GSMINIT_STATE_ENABLE_CREG)	{
			pGsmContext->pConsole->println(F("\tGSM - INIT - GSMINIT_STATE_ENABLE_CREG"));
			Serial.print(F("AT+CREG=1\r"));
			pGsmContext->state = gsmExpect(pGsmContext, gsmBuf, 1000) == GSMSTATE_OK ? GSMINIT_STATE_ENABLE_CGREG : GSMSTATE_INVALID;
		}

		if(pGsmContext->state == GSMINIT_STATE_ENABLE_CGREG)	{
			pGsmContext->pConsole->println(F("\tGSM - INIT - GSMINIT_STATE_ENABLE_CGREG"));
			Serial.print(F("AT+CGREG=1\r"));
			pGsmContext->state = gsmExpect(pGsmContext, gsmBuf, 1000) == GSMSTATE_OK ? GSMINIT_STATE_TEST_SIM_PIN : GSMSTATE_INVALID;
		}

		if(pGsmContext->state == GSMINIT_STATE_TEST_SIM_PIN) {		// after 0,5 - 10 sec., depends of the SIM card
			pGsmContext->pConsole->println(F("\tGSM - INIT - GSMINIT_STATE_TEST_SIM_PIN"));
			delay(1000);
			Serial.print(F("AT+CPIN?\r"));
			switch (gsmExpect(pGsmContext, gsmBuf, 10000)) {		// wait for initial URC presentation "+CPIN: SIM PIN" or similar
			case GSMSTATE_PIN_REQ:
				pGsmContext->state = GSMINIT_STATE_ULOCK_SIM_PIN; // get +CPIN: SIM PIN -> To unlock
				break;
			case GSMSTATE_PIN_RDY:
				pGsmContext->state = GSMINIT_STATE_SIM_OK; 		 // get +CPIN: READY -> Go ahead !
				break;
			default:
				pGsmContext->state = GSMINIT_STATE_SIM_STATUS_SECOND_CHANCE;	// Sinon...
				break;
			}
		}

		if(pGsmContext->state == GSMINIT_STATE_SIM_STATUS_SECOND_CHANCE) {
			pGsmContext->pConsole->println(F("\tGSM - INIT - GSMINIT_STATE_SIM_STATUS_SECOND_CHANCE"));
			switch (gsmExpect(pGsmContext, gsmBuf, 10000)) { 		// new try: wait for initial URC presentation "+CPIN: SIM PIN" or similar
			    case GSMSTATE_PIN_REQ: pGsmContext->state = GSMINIT_STATE_ULOCK_SIM_PIN; break; 		// get +CPIN: SIM PIN
			    case GSMSTATE_PIN_RDY: pGsmContext->state = GSMINIT_STATE_SIM_OK; break;		// get +CPIN: READY
			    default:
				  pGsmContext->pConsole->print(F("\tSIM ERREUR FATALE : >"));
				  pGsmContext->pConsole->print(gsmBuf);    //    here is the explanation
				  pGsmContext->pConsole->println(F("<"));
				  return 0;
			}
		}

		if(pGsmContext->state == GSMINIT_STATE_ULOCK_SIM_PIN) {
			pGsmContext->pConsole->println(F("\tGSM - INIT - GSMINIT_STATE_ULOCK_SIM_PIN"));
			pGsmContext->pConsole->print(F("\tGSM - INIT - Setting PIN code : "));
			pGsmContext->pConsole->println(pinCode);
			Serial.print(F("AT+CPIN="));           // enter pin (SIM)
			Serial.print(pinCode);
			Serial.print('\r');
			pGsmContext->state = gsmExpect(pGsmContext, gsmBuf, 1000) == GSMSTATE_PIN_RDY ? GSMINIT_STATE_SIM_OK : GSMSTATE_INVALID;
		}

		if(pGsmContext->state == GSMINIT_STATE_SIM_OK)	{
			pGsmContext->pConsole->println(F("\tGSM - INIT - SIM UNLOCKED !!!"));
			pGsmContext->state = GSMINIT_STATE_SET_CGQMIN;
			delay(500);
		}		
		
		if(pGsmContext->state == GSMINIT_STATE_SET_CGQMIN)	{
			pGsmContext->pConsole->println(F("\tGSM - INIT - GSMINIT_STATE_SET_CGQMIN"));
			Serial.print(F("AT+CGQMIN=1\r")); // ,0,0,1,0,0\r"));
			pGsmContext->state = gsmExpect(pGsmContext, gsmBuf, 1000) == GSMSTATE_OK ? GSMINIT_STATE_SET_CGQREQ : GSMSTATE_INVALID;
		}
		
		if(pGsmContext->state == GSMINIT_STATE_SET_CGQREQ)	{
			pGsmContext->pConsole->println(F("\tGSM - INIT - GSMINIT_STATE_SET_QIFGCNT"));
			Serial.print(F("AT+CGQREQ=1\r")); // ,0,0,1,0,0\r"));
			pGsmContext->state = gsmExpect(pGsmContext, gsmBuf, 1000) == GSMSTATE_OK ? GSMINIT_STATE_DONE : GSMSTATE_INVALID;
		}

		if(pGsmContext->state == GSMINIT_STATE_DONE) {
			pGsmContext->pConsole->println(F("\tGSM - INIT - GSMINIT_STATE_DONE"));
			pGsmContext->pConsole->println(millis() - time);
			pGsmContext->pConsole->print(F("######## OUT A : "));
			pGsmContext->pConsole->println(__FUNCTION__);
			return 1;							// Init completed ... let's go ahead!
		}
		
		if ((millis() - time) > 30000) {
			pGsmContext->pConsole->println(F("\tGSM - INIT - GAME OVER !!!"));
			pGsmContext->pConsole->print(F("######## OUT B : "));
			pGsmContext->pConsole->println(__FUNCTION__);			
			return 0; // On sort au bout de 2 minutes
		}
		
		delay(200);	// Easy... mais pas trop
		
	}
	while(pGsmContext->state < GSMSTATE_INVALID);

	pGsmContext->pConsole->print(F("######## OUT C : "));
	pGsmContext->pConsole->println(__FUNCTION__);
	
	return 0;			// ERROR ...

}

/*
 * 
 * Affiche diverses infos interessantes.

*/
void gsmInfo(struct gsmContext* pGsmContext)
{
	char gsmBuf[GSM_BUFSZ];
	char Status_string[100];

	pGsmContext->pConsole->print(F("######## IN : "));
	pGsmContext->pConsole->println(__FUNCTION__);

	Serial.print(F("AT+CREG?\r"));                                  // Query register state of GSM network
	gsmExpect(pGsmContext, gsmBuf, 1000);
	strcpy(Status_string, gsmBuf);

	Serial.print(F("AT+CGREG?\r"));                                 // Query register state of GPRS network
	gsmExpect(pGsmContext, gsmBuf, 1000);
	strcat(Status_string, gsmBuf);

	Serial.print(F("AT+CSQ\r"));                                    // Query the RF signal strength
	gsmExpect(pGsmContext, gsmBuf, 1000);
	strcat(Status_string, gsmBuf);

	Serial.print(F("AT+COPS?\r"));                                  // Query the current selected operator
	gsmExpect(pGsmContext, gsmBuf, 1000);
	strcat(Status_string, gsmBuf);
	
	pGsmContext->pConsole->print(F("######## OUT : "));
	pGsmContext->pConsole->println(__FUNCTION__);
}

/*
 * 
 * Status de la connexion
 * Scrute les changements du status de connexion
 * Necessite CREG et CGREG actif

*/
bool gsmNeedToConnect(struct gsmContext* pGsmContext)
{
  unsigned long time = millis();
  char c;
  bool done = false;
  int i=0;
  bool bNeedToConnect = false;
  pGsmContext->pConsole->print(F("######## IN : "));
  pGsmContext->pConsole->println(__FUNCTION__);


  // Capture des eventuels evenements CREG et CGREG
  // Detecter les deconnexions pour relancer un connect
  while (Serial.available() and !done) {
	// Risque de boucle... cet automate atttend uniquement les évènements CREG et CGREG
	// Les URC (pb hard, temp, alim) sont desactivées mais si on les active, se
	// réserver la possibilité de sortir car pas prises en charge par
	// le code qui suit.
	if ((millis() - time) > 30000) return true;
	c = Serial.read();
	pGsmContext->pConsole->print(c);
	switch (c) {
		// Le C de CREG ou CGREG ?
		case 'C' :
			if (i == 0) i++;
			else i = 0;
			break;
		// Le G de CGREG ?
		case 'G' :
			if ((i == 1) or (i == 3) or (i == 4)) i++;
			else i = 0;
			break;
		// Le R de CREG ou CGREG
		case 'R' :
			if ((i == 1) or (i == 2)) i++;
			else i = 0;
			break;
		// Le E de CREG ou CGREG
		case 'E' :
			if ((i == 2) or (i == 3)) i++;
			else i = 0;
			break;				
		// Et la suite
		case ':' :
			if ((i == 4) or (i == 5)) i++;
			else i = 0;
			break;
		case ' ' :
			if ((i == 5) or (i == 6)) i++;
			else i = 0;
			break;
		// 1 : on a activé les notifs CREG et CGREG dans l'init
		// donc normal de trouver cette valeur après un espace et :
		// Cf. commandes AT
		case '1' :
			if ((i == 6) or (i == 7)) {
				i++;
				done = true;
			}
			else i = 0;
			break;
		// Les notifs CREG et CGREG non trouvées, on reçoit quoi la ?
		// Les URC sont désactivées normalement, ou bien vérifie et corrige le code...
		default : i=0;
	}
	if ((millis() - time) > 500)
	  return true;
  }

  // Ici on a trouvé un creg ou cgreg
  // creg ou cgreg : 1, code
  // code : On peut affiner les erreurs ou succès
  // 1 : attaché : home
  // 5 : attaché : roaming
  // 2 : on cherche
  // etc.... le pire est 3, denied
  if (Serial.available()) {
	// on lit la ',' pour trouver le code
	c = Serial.read();
	pGsmContext->pConsole->print(c);
	if (Serial.available()) {
		// On lit le code
		c = Serial.read();
		pGsmContext->pConsole->print(c);
		pGsmContext->pConsole->print(c);
		switch(c) {
			case '1' :		// Home, c'est l'opérateur du contrat
			// On pourrait réagir spécifiquement sur le roaming...
			// désactiver suivant les arrangements entre opérateurs...
			// Ici on y va...
			case '5' :		// Roaming
				bNeedToConnect = true;	// On renvoie un go pour connexion
				pGsmContext->gprsReady = false;		// On a eu une deconnexion...
				break;
			// On pourrait reagir specifiquement sur un register denied
			default :		// Non accroche au reseau pas pret pour un connect
				bNeedToConnect = false;
				pGsmContext->gprsReady = false;
		}
	}
  }

  pGsmContext->pConsole->print(F("######## OUT B : "));
  pGsmContext->pConsole->println(__FUNCTION__);

  return bNeedToConnect;
}

/*
 * Activation de la data, connexion GPRS
 * 
Parametres :
char APN
char USER
char PWD
*/
int gsmGprsConnect(struct gsmContext* pGsmContext, const char* APN, const char* USER, const char* PWD)
{
	char gsmBuf[GSM_BUFSZ];
	unsigned long time = millis();
	
	// Pas bien car on perd l'etat...
	// Faire en sorte de repartir au bon endroit pour ne pas avoir a tout reprendre !!!
	pGsmContext->state = GSMCONNECT_STATE_START;

	pGsmContext->pConsole->print(F("######## IN : "));
	pGsmContext->pConsole->println(__FUNCTION__);

	if (pGsmContext->gprsReady) {
		pGsmContext->pConsole->println(F("\tGSM - CONNECT - Already connected"));
		return 1;
	}
	
	do {
 
		if(pGsmContext->state == GSMCONNECT_STATE_START) {
			pGsmContext->pConsole->println(F("\tGSM - CONNECT - GSMCONNECT_STATE_START"));
			pGsmContext->state = GSMCONNECT_STATE_TEST_NET_REG;
		}
		
		if(pGsmContext->state == GSMCONNECT_STATE_TEST_NET_REG) {
			pGsmContext->pConsole->println(F("\tGSM - CONNECT - GSMCONNECT_STATE_TEST_NET_REG"));	
			Serial.print(F("AT+CREG?\r"));								// Network Registration Report
			switch(gsmExpect(pGsmContext, gsmBuf, 10000)) {
				case GSMSTATE_NET_REG_HOME:
				case GSMSTATE_NET_REG_ROAMING:
					pGsmContext->pConsole->println(F("\tGSM - CONNECT - Registered"));
					pGsmContext->state = GSMCONNECT_STATE_ATTACH_GPRS;
					break;
				case GSMSTATE_NET_REG_DENIED:
					pGsmContext->pConsole->println(F("\tGSM - CONNECT - Denied"));
					pGsmContext->state = GSMSTATE_INVALID;
					break;
				default:
					pGsmContext->pConsole->println(F("\tGSM - CONNECT - Register in progress"));
					pGsmContext->state = GSMCONNECT_STATE_TEST_NET_REG;
			}
		}

		// Quelquechose à voir avec le bearer.
		// A vérifier :
		// on doit se trainer des forfaits...
		// Par exemple : l'aiguillage de la carte sim
		// sans data, avec data, sms ou pas, mms...
		if(pGsmContext->state == GSMCONNECT_STATE_ATTACH_GPRS) {
			pGsmContext->pConsole->println(F("\tGSM - CONNECT - GSMCONNECT_STATE_ATTACH_GPRS"));
			Serial.print(F("AT+CGATT=1\r"));							// attach to GPRS service
			pGsmContext->state = gsmExpect(pGsmContext, gsmBuf, 1000) == GSMSTATE_OK ? GSMCONNECT_STATE_SET_QIFGCNT : GSMCONNECT_STATE_TEST_NET_REG;
			// need +CGATT: 1
			// on a une carte data open ou pas ???
		}
		
		if(pGsmContext->state == GSMCONNECT_STATE_SET_QIFGCNT)	{
			pGsmContext->pConsole->println(F("\tGSM - CONNECT - GSMINIT_STATE_SET_QIFGCNT"));
			Serial.print(F("AT+QIFGCNT=0\r"));
			pGsmContext->state = gsmExpect(pGsmContext, gsmBuf, 1000) == GSMSTATE_OK ? GSMCONNECT_STATE_SET_PDP : GSMSTATE_INVALID;
		}

		if(pGsmContext->state == GSMCONNECT_STATE_SET_PDP)   {
			pGsmContext->pConsole->println(F("\tGSM - CONNECT - GSMCONNECT_STATE_SET_PDP"));
			Serial.print(F("AT+QICSGP=1,\""));				    		// Select GPRS as the bearer
			Serial.print(APN);
			Serial.print(F("\",\""));
			// Serial.print(USER);
			Serial.print(F("\",\""));
			// Serial.print(PWD);
			Serial.print(F("\"\r"));
			pGsmContext->state = gsmExpect(pGsmContext, gsmBuf, 1000) == GSMSTATE_OK ? GSMCONNECT_STATE_DISABLE_MULTI : GSMSTATE_INVALID;
		}

		// Les deux sections suivantes montrent une lacune sur la compréhension du fonctionnement du M95
		// on ne peut changer le comportement du modem sans un hard reset

		// Utilisé une fois après un hard reset, fait partie du context
		// Cf. ref Quectel M95
		if(pGsmContext->state == GSMCONNECT_STATE_DISABLE_MULTI)	{
			pGsmContext->pConsole->println(F("\tGSM - CONNECT - GSMCONNECT_STATE_DISABLE_MULTI"));
			// GDTREM : TODO, vérifier la valeur qiMux du context GSM
			// Un qiMux sur un qiMux génère une "ERROR"
			Serial.print(F("AT+QIMUX=0\r"));
			// On peut plus bouger si pas reset hard du modem
			if (gsmExpect(pGsmContext, gsmBuf, 1000) == GSMSTATE_OK) {
				pGsmContext->state = GSMCONNECT_STATE_ENABLE_TRANSPARENT_MODE;
				pGsmContext->qiMux = true;
			// Double vérification... si on a une erreur sur un qiMux...
			} else {
				if (pGsmContext->qiMux) {
					pGsmContext->state = GSMCONNECT_STATE_ENABLE_TRANSPARENT_MODE;
				} else {
					pGsmContext->state = GSMSTATE_INVALID;
				}
			}
		}

		// Utilisé une fois après un hard reset, fait partie du context
		// Cf. ref Quectel M95
		if(pGsmContext->state == GSMCONNECT_STATE_ENABLE_TRANSPARENT_MODE)	{
			pGsmContext->pConsole->println(F("\tGSM - CONNECT - GSMCONNECT_STATE_ENABLE_TRANSPARENT_MODE"));
			// GDTREM : TODO, vérifier la valeur qiMode du context GSM
			// Un qiMode sur un qiMode génère une "ERROR"
			Serial.print(F("AT+QIMODE=1\r"));
			// On peut plus bouger qiMode, a moins de faire un hard reset
			if (gsmExpect(pGsmContext, gsmBuf, 1000) == GSMSTATE_OK) {
				pGsmContext->state = GSMCONNECT_STATE_REG_APP;
				pGsmContext->qiMode = true;
			// Double vérification... sin on a une erreur sur un qiMode...
			} else {
				if (pGsmContext->qiMode) {
					pGsmContext->state = GSMCONNECT_STATE_REG_APP;
				} else {
					pGsmContext->state = GSMSTATE_INVALID;
				}
			}
		}

		// 
		if(pGsmContext->state == GSMCONNECT_STATE_REG_APP)	{
			pGsmContext->pConsole->println(F("\tGSM - CONNECT - GSMCONNECT_STATE_REG_APP"));
			Serial.print(F("AT+QIREGAPP\r"));
			pGsmContext->state = gsmExpect(pGsmContext, gsmBuf, 1000) == GSMSTATE_OK ? GSMCONNECT_STATE_ACTIVATE : GSMSTATE_INVALID;
		}
		
		if(pGsmContext->state == GSMCONNECT_STATE_ACTIVATE)	{
			pGsmContext->pConsole->println(F("\tGSM - CONNECT - GSMCONNECT_STATE_TEST_ACTIVATION"));
			Serial.print(F("AT+QIACT\r"));
			pGsmContext->state = gsmExpect(pGsmContext, gsmBuf, 1000) == GSMSTATE_OK ? GSMCONNECT_STATE_TEST_IP_STACK : GSMSTATE_INVALID;
		}

		if(pGsmContext->state == GSMCONNECT_STATE_TEST_IP_STACK)	{
			pGsmContext->pConsole->println(F("\tGSM - CONNECT - GSMCONNECT_TEST_IP_STACK"));
			Serial.print(F("AT+QISTATE\r"));
			pGsmContext->state = gsmExpect(pGsmContext, gsmBuf, 1000) == GSMSTATE_IP_GPRSACT ? GSMCONNECT_STATE_TEST_IP_STACK : GSMSTATE_INVALID;
		}

		if(pGsmContext->state == GSMCONNECT_STATE_DONE)  {
			pGsmContext->pConsole->println(F("\tGSM - CONNECT - GSMCONNECT_STATE_DONE"));
			pGsmContext->pConsole->println(millis() - time);
			pGsmContext->pConsole->print(F("######## OUT A : "));
			pGsmContext->pConsole->println(__FUNCTION__);
			pGsmContext->gprsReady = true;
			return 1;													// GPRS connect successfully ... let's go ahead!
		}
		
		if ((millis() - time) > 30000) {
			pGsmContext->pConsole->print(F("######## OUT B : GAME OVER !!!"));
			pGsmContext->pConsole->println(__FUNCTION__);
			return 0; // On sort au bout de 30s
		}
		delay(500);
	}
	while(pGsmContext->state < GSMSTATE_INVALID);
	
	pGsmContext->gprsReady = false;
	pGsmContext->pConsole->print(F("######## OUT C : "));
	pGsmContext->pConsole->println(__FUNCTION__);  
	return 0;													  		// ERROR ... no GPRS connect
}

/*--------------------
 * Send HTTP GET 
 * http://www.antrax.de/downloads/gsm-easy!/quectel-application%20notes/gsm_tcpip_an_v1.1.pdf
 * http://www.antrax.de/downloads/gsm-easy!/quectel-application%20notes/gsm_http_atc_v1.00.pdf
*/
int gsmHttpRequest(struct gsmContext* pGsmContext, const char* url, char* data)
{
	char gsmBuf[GSM_BUFSZ];
	unsigned long time = millis();

	pGsmContext->pConsole->print(F("######## IN : "));
	pGsmContext->pConsole->println(__FUNCTION__); 

	pGsmContext->pConsole->print(F("Sending request : "));
	pGsmContext->pConsole->print(url);
	pGsmContext->pConsole->println(data);

	int l = strlen(url) + strlen(data);
	pGsmContext->pConsole->println(F("\tGSM - SEND - AT+QHTTPURL"));
	Serial.print(F("AT+QHTTPURL="));
	Serial.print(l);					// longueur de l'url complete
	Serial.print(F(",30\r"));			// tmout en s
	// Need CONNECT
	gsmExpect(pGsmContext, gsmBuf, 10000);
	delay(1000);

	Serial.print(url);
	Serial.print(data);
	Serial.print(F("\r"));
	// Need OK
	gsmExpect(pGsmContext, gsmBuf, 10000);
	delay(1000);

	pGsmContext->pConsole->println(F("\tGSM - SEND - AT+QHTTPGET"));
	Serial.print(F("AT+QHTTPGET=60\r"));
	gsmExpect(pGsmContext, gsmBuf, 10000);
	delay(1000);

	pGsmContext->pConsole->println(F("\tGSM - SEND - AT+QHTTPREAD"));
	Serial.print(F("AT+QHTTPREAD=30\r"));
	gsmExpect(pGsmContext, gsmBuf, 10000);

	pGsmContext->pConsole->print(F("######## OUT : "));
	pGsmContext->pConsole->println(__FUNCTION__); 
	return 1;
}


/*
 * Disconnect GPRS connection

*/
void gsmDisconnect(struct gsmContext* pGsmContext)
{
	pGsmContext->pConsole->print(F("######## IN : "));
	pGsmContext->pConsole->println(__FUNCTION__); 

    Serial.print(F("AT+QIDEACT\r"));		// Deactivate GPRS context
    pGsmContext->gprsReady = false;

	pGsmContext->pConsole->print(F("######## OUT : "));
	pGsmContext->pConsole->println(__FUNCTION__);
}

/*
 *  
Central receive routine of the serial interface

*/
int gsmExpect(struct gsmContext* pGsmContext, char* gsmBuf, int timeout)
{
  int  index = 0;
  int  inByte = 0;
  char WS[3];
  char expectBuf[MAX_GSM_STRING_SZ];

  pGsmContext->pConsole->print(F("######## IN : "));
  pGsmContext->pConsole->println(__FUNCTION__);

  memset(gsmBuf, 0, GSM_BUFSZ);
  memset(WS, 0, 3);
  
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
	  // pGsmContext->pConsole->print(F("\tFound : "));
	  // pGsmContext->pConsole->println(expectBuf);
	  // pGsmContext->pConsole->print(F("\tNext State : "));
	  // pGsmContext->pConsole->println(i);
	  pGsmContext->pConsole->print(F("######## OUT A : "));
	  pGsmContext->pConsole->println(__FUNCTION__); 
	  return i;
	}
  }

  pGsmContext->pConsole->print(F("######## OUT B : "));
  pGsmContext->pConsole->println(__FUNCTION__); 
  return 0;
}

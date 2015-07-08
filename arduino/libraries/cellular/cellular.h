#ifndef __CELLULAR_H__
#define __CELLULAR_H__

#define CELL_SERIAL_BAUDRATE	115200

enum protocols{ CCEU_HTTP, CCEU_SIP, CCEU_RAW_UDP, GEOLOC_HTTP };

#include <Arduino.h>

// Initialise le port COM et envoie un AT 
bool cellSetup(HardwareSerial);
int cellMakeData(char*, enum protocols); // DATA, Protocole
int cellPost(char*, char*, char*);  // Host, URL, DATA
int cellSend(char*, int, enum protocols); // Host, port, protocole
int cellExpect(char*, int);	// Attend une chaîne en retour
#endif

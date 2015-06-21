#ifndef __CELLULAR_H__
#define __CELLULAR_H__

enum protocols{ CCEU_HTTP, CCEU_SIP, CCEU_RAW_UDP};

#include <Arduino.h>

int makeData(char*, enum protocols); // DATA, Protocole
int post(char*, char*, char*);  // Host, URL, DATA
int send(char*, int, enum protocols); // Host, port, proto (CCEU_SIP ou CCEU_RAW_UDP)

#endif

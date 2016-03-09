/*
 * 
         +-\/-+
A0 PB5  1|    |8  Vcc
A3 PB3  2|    |7  PB2 D0 A1
A2 PB4  3|    |6  PB1 D1
   GND  4|    |5  PB0 D2
         +----+

Programmation :
Tiny        -> Arduino UNO (Echec avec un MEGA2560)
PB5 - RESET -> Pin 10
PB0 - MOSI  -> Pin 11
PB1 - MISO  -> Pin 12
PB2 - SCK   -> Pin 13

Soit :
13    12    11      10
Vert  Jaune Orange  Bleu

En fonctionnement :
Serie : 
#define SERIAL_RX_PIN 3       Physical Pin 2 for an ATtinyX5
#define SERIAL_TX_PIN 4       Physical Pin 3 for an ATtinyX5
Tiny        -> Arduino...
PB3 - RX    ->
PB4 - TX    ->
I2C :
ATTiny      -> Arduino UNO  / MEGA
PB0 - SDA   -> A4           / 20
PB2 - SCL   -> A5           / 21

Durée d'un scan : 13 canaux * 200 ms -> 2600 ms entre le scan et le résultat :

SCAN:Found 6
01,01,-85,04,3104,1c,00,c0:83:0a:41:99:e1,2WIRE819
02,01,-40,04,1104,1c,00,14:5b:d1:ec:6e:00,McGroves
03,02,-73,04,3104,28,c0,00:26:f2:fa:de:f5,JANOV2
04,06,-70,04,3104,28,40,28:c6:8e:16:89:b8,NETGEAR50
05,11,-84,04,1104,28,40,f8:7b:8c:0b:18:e9,JANOV
06,11,-82,04,1104,1c,40,58:6d:8f:6c:83:38,Gpatrick
END:

*/

#define SERIAL_RX_PIN     3
#define SERIAL_TX_PIN     4
#define I2C_SLAVE_ADDRESS 0x4
#define MAX_AP            20

#include <TinyWireS.h>
#include <SoftwareSerial.h>

// #define DEBUG

#define GAME_OVER                       5000
#define DEFAULT_WAIT_RESPONSE_TIME      1000        // 1000ms
#define DEFAULT_BAUDRATE                9600
#define MAX_BUF_SIZE                    110          // 1 octet RSSI + 6 octets MAC + 4 octets hash : 11 octets par AP -> 10 AP

// Diminuer la taille du buffer RX, sur MAC :
// /Applications/Arduino.app/Contents/Java/hardware/arduino/avr/libraries/SoftwareSerial/SoftwareSerial.h :
// - #define _SS_MAX_RX_BUFF 64
// + #define _SS_MAX_RX_BUFF 8  // On gagne quelques octets en rognant sur le buffer

static const char strCMD[] PROGMEM = "$$$"; // envoyer $$$  pour passer en mode commande
static const char strNJN[] PROGMEM = "set w j 0\r";
static const char strOPT[] PROGMEM = "set sys printlvl 0x4000\r";
static const char strSCN[] PROGMEM = "scan\r";

// SoftwareSerial wifiSerial(SERIAL_RX_PIN, SERIAL_TX_PIN);
SoftwareSerial wifiSerial(8, 9);
bool scan = true;
unsigned long int top = 0;
bool locked = false;
short n = 0;
char apList[MAX_BUF_SIZE];

void setup()
{
#ifdef DEBUG
  Serial.begin(9600);
#endif
  // Initialisation du module wifi
  wifiSerial.begin(DEFAULT_BAUDRATE);
  wifiSerial.setTimeout(DEFAULT_WAIT_RESPONSE_TIME);
  wifiSend(strCMD); wifiClear(500);
  wifiSend(strNJN); wifiClear(500);
  wifiSend(strOPT); wifiClear(500);
  // Init I2C
  TinyWireS.begin(I2C_SLAVE_ADDRESS);
  TinyWireS.onRequest(requestEvent);
}

void loop()
{

  if (scan)
  {
#ifdef DEBUG
    Serial.println("Scan");
#endif
    wifiSend(strSCN);
    scan = false;
    top = millis();
    wifiClear(1000);
  }
  else
  {
#ifdef DEBUG
    Serial.println("Res");
#endif
    if ((n = getAPs()) >=0) {
      if ((n > 0) and !locked) {
#ifdef DEBUG
        Serial.print(n);
        Serial.println(" AP visibles");
#endif
        locked = true;
        populate(n);
        locked = false;
      }
      scan = true;
    }
    else {
      if ((millis() - top) > GAME_OVER) {
        wifiClear(1000);
        scan = true;
      }
    }
  }
}

// https://github.com/rambo/TinyWire/blob/master/TinyWireS/examples/TinyWireS_Stress_Master/TinyWireS_Stress_Master.ino
void requestEvent()
{
  if (locked) return;
  locked = true;
  TinyWireS.send(20);
  locked = false;
}

void wifiClear(unsigned long int timeout)
{
    unsigned long int t=0;
    t=millis();
#ifdef DEBUG
    Serial.println("Clean");
#endif
    while ((millis() - t) < timeout) {
      while (wifiSerial.available()) {
        wifiSerial.read();
      }
    }
}


// Récrire cette fonction qui est trop moche
short getAPs() {
  bool found = false;
  int nb = -1;
  int i = 0;
  unsigned long int t=millis();
  char WS[3];
  char buf[3];
  short cars = 0;

  // On cherche le pattern "SCAN:Found "
  while (((millis() - t) < 2000) and (cars < 50) and !found) {
    if (wifiSerial.available()) {
      if (wifiSerial.readBytes(WS, 1) == 1) {
        cars++;
        switch (WS[0]) {
          case 'S': i == 0 ? i++ : i = 0; break;
          case 'C': i == 1 ? i++ : i = 0; break;
          case 'A': i == 2 ? i++ : i = 0; break;
          case 'N': i == 3 ? i++ : i = 0; break;
          case ':': i == 4 ? i++ : i = 0; break;
          case 'F': i == 5 ? i++ : i = 0; break;
          case 'o': i == 6 ? i++ : i = 0; break;
          case 'u': i == 7 ? i++ : i = 0; break;
          case 'n': i == 8 ? i++ : i = 0; break;
          case 'd': i == 9 ? i++ : i = 0; break;
          case ' ': i == 10 ? i++ : i = 0; found = true; break;
          default : i = 0;
        }
      }
    }
  }

  if (found) {
    t = millis();
    found = false;
    cars = 0;
    i = 0;
    while (((millis() - t) < 2000) and (cars < 50) and !found and (i<3)) {
      if (wifiSerial.available()) {
        if (wifiSerial.readBytes(WS, 1) == 1) {
          cars++;
          if((WS[0] >= 48) and (WS[0] <= 57)) buf[i++] = WS[0];
          else {
            if(WS[0] == '\r') {
              wifiSerial.readBytes(WS, 1); // On lit le LF aussi
              buf[i] = '\0';
              found = true;
            }
            else { // Pb !!!
              found = true;
              i = 0;
            }
          }
        } // Trop de brackets tuent les brackets… faut changer de brackets...
      }
    }
  }

  if (found and (i > 0)) nb = atoi(buf);
  
  return nb;
  
}

int populate(int n) {
  // 06,11,-82,04,1104,1c,40,58:6d:8f:6c:83:38,Gpatrick ('teution, peut être vide !)
  // On lit les RSSI (8 bits), MAC et SSID, on fait un hash du MAC,SSID (32 bits) -> 5 octets
  // On stocke une série de RSSI/HASH : 8 bits 32 bits 8 bits etc...
  // SSID peut etre vide.
  byte cnt = 0;
  byte i=0;
  unsigned long int hash = 5381;
  unsigned long int t=millis();  // timeout
  char WS[2]; // lire le buffer software serial, pourquoi 3 cars ?

  memset(apList, '\0', MAX_BUF_SIZE);
  wifiSerial.setTimeout(2000);
  while(((millis() - t) < 1000) and (cnt < n) and (cnt < MAX_AP)) {
    wifiSerial.parseInt();
    wifiSerial.parseInt();
#ifdef DEBUG
    Serial.print(wifiSerial.parseInt());
#else
    wifiSerial.parseInt();
#endif
    i=0;
    while(i<15) { i+=wifiSerial.readBytes(WS, 1); }
    while(WS[0] != '\r') {
#ifdef DEBUG
      if(WS[0] != ':') Serial.write(WS[0]);
#endif
      if ((WS[0] != ',') and (WS[0] != ',')) hash = ((hash << 5) + hash) + WS[0];
      wifiSerial.readBytes(WS, 1);
    }
    wifiSerial.readBytes(WS, 1); // On lit le LF
#ifdef DEBUG
    Serial.print(",");
    Serial.println(hash);
#endif
    hash = 5381;
    cnt++;
  }
  
  return 0;
}

void wifiSend(const char* cmd){
  short i=0;
  while(char c=pgm_read_byte(cmd+i++)) {
#ifdef DEBUG
    Serial.write(c);
#endif
    wifiSerial.write(c);
  }
}

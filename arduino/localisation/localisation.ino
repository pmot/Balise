
#include <Arduino.h>
#include <SoftwareSerial.h>
#include <TinyGPS.h>
#include <WiFly.h>

#include "Automate.h"
#include "parameters.h"

SoftwareSerial gpsSerial(GPS_RX, GPS_TX);
TinyGPS gps;
SoftwareSerial wifiSerial(WIFI_RX, WIFI_TX);
WiFly wifly(&wifiSerial);


Automate beginWifiScan("Suites\r", "END:\r");
Automate endWifiScan("END:\r", "");

void setup() {

  Serial.begin(9600);
  delay(1000);
  Serial.println("INIT");

  wifiSerial.begin(9600);
  gpsSerial.begin(9600);

  // Initialisation de la carte Wifi
  delay(1000);
  wifly.reset();
  delay(3000);

  Serial.println("INIT DONE");

}

void loop() {
  float flat, flon;
  unsigned long age, date, time, chars = 0;
  Serial.println("\n========= BEGIN");
  
  // Prise de position GPS
  gpsSerial.listen();
  gpsRead(1000);
  // Print : HDOP, etc.
  gps.f_get_position(&flat, &flon, &age);
  print_date(gps);
  Serial.println();
  print_float(flat, TinyGPS::GPS_INVALID_F_ANGLE, 10, 6);
  Serial.println();
  print_float(flon, TinyGPS::GPS_INVALID_F_ANGLE, 11, 6);
  Serial.println();
  print_float(gps.f_speed_kmph(), TinyGPS::GPS_INVALID_F_SPEED, 6, 2);
  Serial.println();
  Serial.println("EndGPS");
  // Scan des SSID
  wifiSerial.listen();
  wifiScanSSID();
  // Print : SSID, MAC, RSSI
  Serial.println();
  Serial.println("EndWifi");
  Serial.println("\n========= END");
  
}

static void gpsRead(unsigned long ms)
{
  unsigned long start = millis();
  do 
  {
    while (gpsSerial.available())
      gps.encode(gpsSerial.read());
  } while (millis() - start < ms);
}

static void print_float(float val, float invalid, int len, int prec)
{
  if (val == invalid)
  {
    while (len-- > 1)
      Serial.print('*');
    Serial.print(' ');
  }
  else
  {
    Serial.print(val, prec);
    int vi = abs((int)val);
    int flen = prec + (val < 0.0 ? 2 : 1); // . and -
    flen += vi >= 1000 ? 4 : vi >= 100 ? 3 : vi >= 10 ? 2 : 1;
    for (int i=flen; i<len; ++i)
      Serial.print(' ');
  }
  gpsRead(0);
}

static void print_int(unsigned long val, unsigned long invalid, int len)
{
  char sz[32];
  if (val == invalid)
    strcpy(sz, "*******");
  else
    sprintf(sz, "%ld", val);
  sz[len] = 0;
  for (int i=strlen(sz); i<len; ++i)
    sz[i] = ' ';
  if (len > 0) 
    sz[len-1] = ' ';
  Serial.print(sz);
  gpsRead(0);
}

static void print_date(TinyGPS &gps)
{
  int year;
  byte month, day, hour, minute, second, hundredths;
  unsigned long age;
  gps.crack_datetime(&year, &month, &day, &hour, &minute, &second, &hundredths, &age);
  if (age == TinyGPS::GPS_INVALID_AGE)
    Serial.print("********** ******** ");
  else
  {
    char sz[32];
    sprintf(sz, "%02d/%02d/%02d %02d:%02d:%02d ",
        month, day, year, hour, minute, second);
    Serial.print(sz);
  }
  print_int(age, TinyGPS::GPS_INVALID_AGE, 5);
  gpsRead(0);
}

static void print_str(const char *str, int len)
{
  int slen = strlen(str);
  for (int i=0; i<len; ++i)
    Serial.print(i<slen ? str[i] : ' ');
  gpsRead(0);
}

static void wifiScanSSID()
{

  // un scan wifi via le RN 171 met environ 2500ms par défaut:
  // - 200ms par canal
  // - 13 canaux
  
  char c;
  bool wifiScanBegin = false;
  bool wifiScanDone = false;
  
  // Scan !
  wifly.sendCommand("scan\r");

  while (!wifiScanBegin) {
    if (wifly.receive((uint8_t *)&c, 1, 300) > 0)
      wifiScanBegin = beginWifiScan.iterate(c);
    // gpsRead(0);
  }

  while (!wifiScanDone) {
    if (wifly.receive((uint8_t *)&c, 1, 300) > 0) {
      wifiScanDone = endWifiScan.iterate(c);
      if (!wifiScanDone) Serial.print((char)c);
      // Ecrire une methode qui collecte la liste des SSIDS
      // en Json ?
      // wifi.encode(c);
    }
    // Print wifi SSIDs
    // wifi.printSSIDs();
    // gpsRead(0);
  }
}




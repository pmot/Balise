
#include <Arduino.h>
#include <SoftwareSerial.h>
#include "WiFly.h"

SoftwareSerial wifiSerial(5, 4);
WiFly wifly(&wifiSerial);

int i = 0;
char c;

void setup() {
  wifiSerial.begin(9600);

  Serial.begin(9600);
  Serial.println("--------- SCAN --------");  // put your setup code here, to run once:
  
  delay(1000);
  wifly.reset();
  delay(3000);
  Serial.println("INIT DONE");

}

void loop() {
  i = 0;
  bool scanencours = true;


  wifly.sendCommand("scan\r");
  while (scanencours) {
    if(wifly.receive((uint8_t *)&c, 1, 300) > 0)
        if((c>0) && (c<128)) Serial.print((char)c);
    scanencours = automate();
  }

  Serial.println("Next scan in 2s");
  delay(2000);
  
}

bool automate() {
  // blah
  bool ret = true;
  switch(i) {
    case 0 :
      if (c == 'E') i++;
      break;
    case 1 :
      if (c == 'N') i++;
      else i = 0;
      break;
    case 2 :
      if (c == 'D') i++;
      else i = 0;
      break;
    case 3 :
      if (c == ':') ret = false;
      else i = 0;
      break; 
  }
  
  return ret;  
}

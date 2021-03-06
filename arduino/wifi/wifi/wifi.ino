#include <Arduino.h>
#include <SoftwareSerial.h>
#include "WiFly.h"

#define SSID      "monssid"
#define KEY       "macle"
#define AUTH      WIFLY_AUTH_WPA1     // or WIFLY_AUTH_WPA1, WIFLY_AUTH_WPA2_PSK, WIFLY_AUTH_WEP, WIFLY_AUTH_OPEN

// Pins' connection
// Arduino       WiFly
//  2    <---->    TX
//  3    <---->    RX
SoftwareSerial uart(5, 4);
WiFly wifly(&uart);
//WiFly wifly(&Serial1);     // for leonardo, use hardware serial - Serial1

void setup() {
  uart.begin(9600);

  Serial.begin(9600);
  Serial.println("--------- WIFLY TEST --------");
  
  // wait for initilization of wifly
  delay(3000);
  
  uart.begin(9600);     // WiFly UART Baud Rate: 9600
  
  wifly.reset();
  /*
  Serial.println("Join " SSID );
  if (wifly.join(SSID, KEY, AUTH)) {
    Serial.println("OK");
  } else {
    Serial.println("Failed");
  }
  */
  // get WiFly params
  wifly.sendCommand("get everything\r");
  char c;
  while (wifly.receive((uint8_t *)&c, 1, 300) > 0) {
    Serial.print((char)c);
  }
  
  if (wifly.commandMode()) {
    Serial.println("Enter command mode. Send \"exit\"(with \\r) to exit command mode");
  }
}

void loop() {
  while (wifly.available()) {
    Serial.write(wifly.read());
  }
  
  while (Serial.available()) {
    wifly.write(Serial.read());
  }
}

/*
GSM :
PWRKEY -> D8
RX -> TX
TX -> RX

Cable USB :
Noir -> Grnd
Vert -> 10
Jaune -> 11
*/

#include <SoftwareSerial.h>

SoftwareSerial mySerial(10, 11); // RX, TX
char c;


void setup() {
  
  
  Serial.begin(9600);
  Serial.println("Yep!");

  digitalWrite(8, HIGH);
  delay(1000);
  digitalWrite(8, LOW);
  delay(1000);
  digitalWrite(8, HIGH);
  delay(2100);
  
  mySerial.begin(9600);
  
  Serial.println("Yip!");
  
  for (int i=0; i < 3; i++)
  {
  mySerial.print("A");
  delay(500);
  mySerial.print("T");
  delay(500);
  mySerial.print("\r");
  delay(500);
  }
  
  mySerial.print("AT+IPR=9600\r");

  while(mySerial.available())
  {
    c = mySerial.read();
  }  
  Serial.println("Init fin");
}

void loop() {

  char c;
  
  if(Serial.available())
  {
    c = Serial.read();
    mySerial.print(c);
  }
  while(mySerial.available())
  {
    c = mySerial.read();
    Serial.print(c);
  }

}

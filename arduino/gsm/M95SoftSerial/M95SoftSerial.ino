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
  delay(10000);
  mySerial.begin(9600);
  mySerial.println("Yep!");


  digitalWrite(8, HIGH);
  delay(1000);
  digitalWrite(8, LOW);
  delay(1000);
  digitalWrite(8, HIGH);
  delay(2100);
  
  mySerial.println("Yip!");

  Serial3.begin(115200);
  
  for (int i=0; i < 3; i++)
  {
  Serial3.print("A");
  delay(500);
  Serial3.print("T");
  delay(500);
  Serial3.print("\r");
  delay(500);
  }
  
  Serial3.print("AT+IPR=115200\r");

  while(Serial3.available())
  {
    c = Serial3.read();
  }  
  
  mySerial.println("Init fin");
  
}

void loop() {

  mySerial.print("AT+CPIN?\r\n");
  Serial3.print("AT+CPIN?\r");
  delay(200);
  while(Serial3.available())
  {
    c = Serial3.read();
    // mySerial.print(c);
    mySerial.print(c);
  }
  delay(2000);

}

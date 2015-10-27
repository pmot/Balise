#include <SoftwareSerial.h>

#define CONSOLE_TX	11
#define CONSOLE_RX	12

SoftwareSerial consoleSerial(CONSOLE_RX, CONSOLE_TX); // RX, TX
char c;


void setup() {
  
  delay(5000);
  
  Serial.begin(9600);
  
  consoleSerial.listen();
  consoleSerial.begin(9600);
  consoleSerial.println("Hello");
  
  for (int i=0; i < 3; i++)
  {
    Serial.print("A");
    delay(500);
    Serial.print("T");
    delay(500);
    Serial.print("\r");
    delay(500);
  }
  
  Serial.print("AT+IPR=9600\r");

  while(Serial.available())
  {
    consoleSerial.write(Serial.read());
  }  
  consoleSerial.println("Init fin");
}

void loop() {

  char c;
  
  if(Serial.available())
  {
    c = Serial.read();
    consoleSerial.print(c);
  }
  while(consoleSerial.available())
  {
    c = consoleSerial.read();
    Serial.print(c);
  }

}

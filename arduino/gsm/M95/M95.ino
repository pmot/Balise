#include <SoftwareSerial.h>

#define CONSOLE_TX	11
#define CONSOLE_RX	12

#define GSM_PWRK	10


SoftwareSerial consoleSerial(CONSOLE_RX, CONSOLE_TX); // RX, TX
char c;


void setup() {
  
  delay(5000);
  
  Serial.begin(9600);

  digitalWrite(GSM_PWRK, HIGH);
  delay(5000);
  digitalWrite(GSM_PWRK, LOW);
  delay(2000);
  digitalWrite(GSM_PWRK, HIGH);
  delay(5000);  

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
    // if (c=='.') {
    //  delay(1000);
    //  Serial.println((char)26);
    //  Serial.println();
    // }
  }

}

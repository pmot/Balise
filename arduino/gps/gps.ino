
#include <SoftwareSerial.h>

SoftwareSerial mySerial(7, 6); // RX, TX

void setup()  
{
  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo only
  }

  // set the data rate for the SoftwareSerial port
  mySerial.begin(9600);
  // mySerial.println("Hello, world?");
  
  Serial.println("FIN INIT");

}

void loop() // run over and over
{
  if (mySerial.available())
    Serial.write(mySerial.read());
  if (Serial.available())
    mySerial.write(Serial.read());
}

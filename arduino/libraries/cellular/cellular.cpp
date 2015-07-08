#include "cellular.h"

bool cellSetup(HardwareSerial mySerial)
{
	mySerial.begin(CELL_SERIAL_BAUDRATE);
	return true;
	// Deal with autobauding...
	mySerial.print("A");
	delay(1000);
	mySerial.print("T");
	delay(1000);
	mySerial.print("\r");
}

#include "accelerometer.h"

// Read the accelerometer data and put values into global variables
void readVal()
{
byte xAddressByteL = 0x28; // Low Byte of X value (the first data register)
byte readBit = B10000000; // bit 0 (MSB) HIGH means read register
byte incrementBit = B01000000; // bit 1 HIGH means keep incrementing registers
// this allows us to keep reading the data registers by pushing an empty byte
byte dataByte = xAddressByteL | readBit | incrementBit;
byte b0 = 0x0; // an empty byte, to increment to subsequent registers

byte yL ;// = SPI.transfer(b0); // get the low byte of Y data
byte yH ;// = SPI.transfer(b0); // get the high byte of Y data

// shift the high byte left 8 bits and merge the high and low
int yVal = (yL | (yH << 8));

// scale the values into G's
// yAcc = yVal * SCALE;
}

/*
  // lis.setPowerStatus(LR_POWER_NORM);
  // lis.setYEnable(true);
  // Registres
  // lis.writeReg(0x30, 0x08);
  // lis.writeReg(0x32, 0x01);
  // lis.writeReg(0x33, 0x06);
  // Interruption
*/

void accelerometerSetup(LIS331 lis)
{
// Set up the accelerometer
// write to Control register 1: address 20h
byte addressByte = 0x20;
/* Bits:
PM2 PM1 PM0 DR1 DR0 Zen Yen Xen
PM2PM1PM0: Power mode (001 = Normal Mode)
DR1DR0: Data rate (00=50Hz, 01=100Hz, 10=400Hz, 11=1000Hz)
Zen, Yen, Xen: Z enable, Y enable, X enable
*/
byte ctrlRegByte = 0x37; // 00111111 : normal mode, 1000Hz, xyz enabled
// Send the data for Control Register 1
// TODO : recalculer ctrRegByte pour activer uniquement y
lis.writeReg(addressByte, ctrlRegByte);

// write to Control Register 2: address 21h
addressByte = 0x21;
// This register configures high pass filter
ctrlRegByte = 0x00; // High pass filter off

// Send the data for Control Register 2
// SPI.transfer(addressByte);
// SPI.transfer(ctrlRegByte);

// Control Register 3 configures Interrupts
addressByte = 0x22;
/* Bits:
IHL PP_OD LIR2 I2_CFG1 I2_CFG0 LIR1 I1_CFG1 I1_CFG0
IHL: interrupt high low (0=active high)
PP_OD: push-pull/open drain selection (0=push-pull)
LIR2: latch interrupt request on INT2_SRC register (0=interrupt request not latched)
I2_CFG1, I2_CFG0: data signal on INT 2 pad control bits
LIR1: latch interrupt request on INT1_SRC register (0=interrupt request not latched)
I1_CFG1, I1_CFG0: data signal on INT 1 pad control bits

Data signal on pad
I1(2)_CFG1      I1(2)_CFG0            INT 1(2) pad
   0                0              interrupt 1(2) source
   0                1              interrupt 1 source or interrupt 2 source
   1                0              data ready
   1                1              boot running

*/
ctrlRegByte = 0x00; // 00000000 : default
// SPI.transfer(addressByte);
// SPI.transfer(ctrlRegByte);

// write to Control Register 4: address 23h
addressByte = 0x23;
/* Bits:
BDU BLE FS1 FS0 STsign 0 ST SIM
BDU: Block data update (0=continuous update)
BLE: Big/little endian data (0=accel data LSB at LOW address)
FS1FS0: Full-scale selection (00 = +/-6G, 01 = +/-12G, 11 = +/-24G)
STsign: selft-test sign (default 0=plus)
ST: self-test enable (default 0=disabled)
SIM: SPI mode selection(default 0=4 wire interface, 1=3 wire interface)
*/
ctrlRegByte = 0x30; // 00110000 : 24G (full scale)


// Send the data for Control Register 4
// SPI.transfer(addressByte);
// SPI.transfer(ctrlRegByte);

// Control Register for interrupt configuration (INT1_CFG)
addressByte = 0x30;
ctrlRegByte = 0x2A; // 00101010 : interrupt request enabled on high Z, high Y, high X events

// SPI.transfer(addressByte);
// SPI.transfer(ctrlRegByte);

// Control Register for interrupt threshold for high events (INT1_THS)
addressByte = 0x32;
ctrlRegByte = 0x0F; // 0 0000000 : value TBD
// SPI.transfer(addressByte);
// SPI.transfer(ctrlRegByte);

// Control Register for interrupt duration for high events (INT1_DURATION)
addressByte = 0x33;
ctrlRegByte = 0x02; // 0 0000010 : value TBD
// SPI.transfer(addressByte);
// SPI.transfer(ctrlRegByte);

// Control Register for interrupt configuration (INT2_CFG)
addressByte = 0x34;
ctrlRegByte = 0x15; // 00010101 : interrupt request enabled on low Z, low Y, low X events

// SPI.transfer(addressByte);
// SPI.transfer(ctrlRegByte);

// Control Register for interrupt threshold (INT2_THS)
addressByte = 0x36;
ctrlRegByte = 0x00; // 00000000 : value TBD
// SPI.transfer(addressByte);
// SPI.transfer(ctrlRegByte);
}

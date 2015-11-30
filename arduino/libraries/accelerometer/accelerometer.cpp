#include "accelerometer.h"

bool accelerometerSetup(LIS331 *lis) {

	/////////////////////////////////////////////////////////////////
	/* Bits:
			PM2 PM1 PM0 DR1 DR0 Zen Yen Xen
			PM2PM1PM0: Power mode (001 = Normal Mode)
			DR1DR0: Data rate (00=50Hz, 01=100Hz, 10=400Hz, 11=1000Hz)
			Zen, Yen, Xen: Z enable, Y enable, X enable
	 */
	byte ctrlRegByte = 0b00111010; // 00111010 : normal mode, 1000Hz, y enabled, xz disabled
	// Send the data for Control Register 1
	if(!lis->writeReg(LR_CTRL_REG1, ctrlRegByte)) {
		return false;
	}

	/////////////////////////////////////////////////////////////////
	// This register configures high pass filter
	ctrlRegByte = 0b00000000; // High pass filter off 0b00000000 - 0b00110100

	// Send the data for Control Register 2
	if(!lis->writeReg(LR_CTRL_REG2, ctrlRegByte)) {
		return false;
	}

	// Filter value
	ctrlRegByte = 0b00000010; //
	if(!lis->writeReg( LR_REFERENCE, ctrlRegByte)) {
			return false;
	}

	/////////////////////////////////////////////////////////////////
	// Control Register 3 configures Interrupts
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
	ctrlRegByte = 0b00000000; // 00000000 : default
	if(!lis->writeReg(LR_CTRL_REG3, ctrlRegByte)) {
		return false;
	}

	/////////////////////////////////////////////////////////////////
	// write to Control Register 4: address 23h
	/* Bits:
		BDU BLE FS1 FS0 STsign 0 ST SIM
		BDU: Block data update (0=continuous update)
		BLE: Big/little endian data (0=accel data LSB at LOW address)
		FS1FS0: Full-scale selection (00 = +/-6G, 01 = +/-12G, 11 = +/-24G)
		STsign: selft-test sign (default 0=plus)
		ST: self-test enable (default 0=disabled)
		SIM: SPI mode selection(default 0=4 wire interface, 1=3 wire interface)
	 */
	ctrlRegByte = 0b00000000; // 00110000 : 24G (full scale)
	if(!lis->writeReg(LR_CTRL_REG4, ctrlRegByte)) {
		return false;
	}


	/////////////////////////////////////////////////////////////////
	// Control Register for interrupt configuration (INT1_CFG)
	ctrlRegByte = 0b00001000; // 00001000 : interrupt request enabled  high Y
	if(!lis->writeReg(LR_INT1_CFG, ctrlRegByte)) {
		return false;
	}

	/////////////////////////////////////////////////////////////////
	// Control Register for interrupt configuration (INT1_SRC)
	ctrlRegByte = 0b00001000; // 00001000 : iY high event has occurred
	if(!lis->writeReg(LR_INT1_SOURCE, ctrlRegByte)) {
		return false;
	}

	/////////////////////////////////////////////////////////////////
	// Control Register for interrupt threshold for high events (INT1_THS)
	ctrlRegByte = 0b00000011; // 0 0000000 : value TBD
	if(!lis->writeReg(LR_INT1_THS, ctrlRegByte)) { // configurer le seuil d'interruption
		return false;
	}
	/////////////////////////////////////////////////////////////////
	// Control Register for interrupt duration for high events (INT1_DURATION)
	ctrlRegByte = 0b0000111; // 0 0000010 : value TBD
	if(!lis->writeReg(LR_INT1_DURATION, ctrlRegByte)) {
		return false;
	}

	/////////////////////////////////////////////////////////////////
	// Control Register for interrupt configuration (INT2_CFG)
	ctrlRegByte = 0b00000000; // 00010101 : interrupt request enabled on low Z, low Y, low X events
	if(!lis->writeReg(LR_INT2_CFG, ctrlRegByte)) {
		return false;
	}

	/////////////////////////////////////////////////////////////////
	// Control Register for interrupt threshold (INT2_THS)
	ctrlRegByte = 0x00; // 00000000 : value TBD
	if(!lis->writeReg(LR_INT2_THS, ctrlRegByte)) {
		return false;
	}

	return true;
}

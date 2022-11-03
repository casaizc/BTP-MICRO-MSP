/*
 * FM24C16B.h
 *
 * Created: 18/02/2020 12:15:45 p. m.
 * Author : SEBASTIAN ARBOLEDA D
 * e-mail: sebasarboleda143@gmail.com
 */ 


#ifndef FM24C16B_H_
#define FM24C16B_H_

#include <avr/io.h>
#include <stdlib.h>
#include <string.h>
// IDs
//Manufacturers codes
#define FUJITSU_MANUFACT_ID 0x00A
#define CYPRESS_MANUFACT_ID 0x004
#define MANUALMODE_MANUFACT_ID 0xF00
#define MANUALMODE_PRODUCT_ID 0xF00
#define MANUALMODE_DENSITY_ID 0xF00

// The density codes gives the memory's adressing scheme
#define DENSITY_MB85RC04V 0x00		// 4K
#define DENSITY_MB85RC64TA 0x03		// 64K
#define DENSITY_MB85RC256V 0x05		// 256K
#define DENSITY_MB85RC512T 0x06		// 512K
#define DENSITY_MB85RC1MT 0x07		// 1M

#define DENSITY_CY15B128J 0x01		// 128K - FM24V01A also
#define DENSITY_CY15B256J 0x02		// 256K - FM24V02A also
#define DENSITY_FM24V05 0x03		// 512K
#define DENSITY_FM24V10 0x04		// 1024K

// Devices MB85RC16, MB85RC16V, MB85RC64A, MB85RC64V and MB85RC128A do not support Device ID reading
// 			FM24W256,FM24CL64B, FM24C64B, FM24C16B, FM24C04B, FM24CL04B

#define MAXADDRESS_04 512
#define MAXADDRESS_16 2048
#define MAXADDRESS_64 8192
#define MAXADDRESS_128 16384
#define MAXADDRESS_256 32768
#define MAXADDRESS_512 65535
#define MAXADDRESS_1024 65535 // 1M devices are in fact managed as 2 512 devices from lib point of view > create 2 instances of the object with each a differnt address


//Special commands
#define MASTER_CODE	0xF8 //0xF8 fujitsu MB85RC64TA ; 0xA0 MB85RC64A
#define SLEEP_MODE	0x86 //Cypress codes, not used here
#define HIGH_SPEED	0x08 //Cypress codes, not used here

// Managing Write protect pin
#define MANAGE_WP true //false if WP pin remains not connected

// Error management
#define ERROR_0 0 // Success
#define ERROR_1 1 // Data too long to fit the transmission buffer on Arduino
#define ERROR_2 2 // received NACK on transmit of address
#define ERROR_3 3 // received NACK on transmit of data
#define ERROR_4 4 // Serial seems not available
#define ERROR_5 5 // Not referenced device ID
#define ERROR_6 6 // Unused
#define ERROR_7 7 // Fram chip unidentified
#define ERROR_8 8 // Number of bytes asked to read null
#define ERROR_9 9 // Bit position out of range
#define ERROR_10 10 // Not permitted opération
#define ERROR_11 11 // Memory address out of range

class FRAM_MB85RC_I2C{

public:	
	void init();
	uint8_t checkDevice(void);
	uint8_t getDeviceIDs(void);
	void sleep(void);
	void wakeUp(void);
	uint8_t writeFRAM(uint8_t addr5, uint8_t addr8, uint32_t data);
	uint32_t readFRAM(uint8_t addr5, uint8_t addr8);
	
private:
	uint8_t		_framInitialised;
	uint16_t	manufacturer;
	uint16_t	productid;
	uint16_t	densitycode;
	uint16_t	density;
	uint16_t	maxaddress;
		
};

extern FRAM_MB85RC_I2C fram;

#endif /* FM24C16B_H_ */
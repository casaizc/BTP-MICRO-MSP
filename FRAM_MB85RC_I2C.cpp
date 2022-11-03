/*
 * FM24C16B.cpp
 *
 * Created: 18/02/2020 12:15:45 p. m.
 * Author : SEBASTIAN ARBOLEDA D
 * e-mail: sebasarboleda143@gmail.com
 */ 

/************************************************************************/
/*								FRAM	
readFRAM(ADDR(4)+PAGE+R/W(A2,A1,A0), SEGMENT)							*/
/*  Bits 7–4 are the device type and should be set to 1010b             */
/*  Bits 3–1 are the page select, It specifies the 256-byte				*/
/*  block of memory that is targeted for the current operation.         */
/************************************************************************/
	
#include "FRAM_MB85RC_I2C.h"
#include "i2cmaster.h"
#define FRAM_ADDR 0b10100000

FRAM_MB85RC_I2C fram;

void FRAM_MB85RC_I2C::init(){
	i2cmaster.i2c_init();
}

uint8_t FRAM_MB85RC_I2C::checkDevice(void){
	
	uint8_t result;
	result = getDeviceIDs();
	
	if ((result == ERROR_0) && ((manufacturer == FUJITSU_MANUFACT_ID) || (manufacturer == CYPRESS_MANUFACT_ID) || (manufacturer == MANUALMODE_MANUFACT_ID)) && (maxaddress != 0)) {
		_framInitialised = 1;
		return ERROR_0;
	}
	else {
		return ERROR_7;
		_framInitialised = 0;
	}
	
}


	
uint8_t FRAM_MB85RC_I2C::getDeviceIDs(void){
	//uint8_t localbuffer[3] = { 0, 0, 0 };
	uint8_t result=0;
	
	/* Get device IDs sequence 	*/
	/* 1/ Send 0xF8 to the I2C bus as a write instruction. bit 0: 0 => 0xF8 >> 1 */
	/* Send 0xF8 to 12C bus. Bit shift to right as beginTransmission() requires a 7bit. beginTransmission() 0 for write => 0xF8 */
	/* Send device address as 8 bits. Bit shift to left as we are using a simple write()                                        */
	/* Send 0xF9 to I2C bus. By requesting 3 bytes to read, requestFrom() add a 1 bit at the end of a 7 bits address => 0xF9    */
	/* See p.10 of http://www.fujitsu.com/downloads/MICRO/fsa/pdf/products/memory/fram/MB85RC-DS501-00017-3v0-E.pdf             */
	
	uint8_t error = i2cmaster.i2c_start_wait(MASTER_CODE);
	uint8_t localbuffer[3] = { 0, 0, 0 };
		
	if(!error){
		i2cmaster.i2c_write(FRAM_ADDR);
		i2cmaster.i2c_rep_start(MASTER_CODE | I2C_READ);
		localbuffer[0] = (uint8_t) i2cmaster.i2c_readAck();
		localbuffer[1] = (uint8_t) i2cmaster.i2c_readAck();
		localbuffer[2] = (uint8_t) i2cmaster.i2c_readNak();
		i2cmaster.i2c_stop();
	
	
		
		/* Shift values to separate IDs */
		manufacturer	= (localbuffer[0] << 4) | (localbuffer[1] >> 4);
		densitycode		= (uint16_t)(localbuffer[1] & 0x0F);
		productid		= ((localbuffer[1] & 0x0F) << 8) + localbuffer[2];
		
		if (manufacturer == FUJITSU_MANUFACT_ID) {
			if(densitycode == DENSITY_MB85RC64TA) {
				density = 64;
				maxaddress = MAXADDRESS_64;
			}else{
				density = 0; 
				maxaddress = 0; 
				result = ERROR_7;
			}
		}
		else {
			density = 0; /* means error */
			maxaddress = 0; /* means error */
			result = ERROR_7; /*device unidentified, comminication ok*/
		}

		return result;
	}else{
		return ERROR_7;
	}
}

void FRAM_MB85RC_I2C::sleep(void)
{	
	/****************************************************************************************************/							
	/*Sleep Mode																						*/
	/*a) The master sends start condition followed by F8H.												*/
	/*b) After ACK response from slave, the master sends the device address word.						*/
	/*In this device address word, Read/Write code are Don't care.										*/
	/*c) After ACK response from slave, the master re-sends the start condition followed by 86H.		*/
	/*d) The slave moves to Sleep mode after ACK response to the master.								*/
	/****************************************************************************************************/

	i2cmaster.i2c_start_wait(MASTER_CODE);
	i2cmaster.i2c_write(FRAM_ADDR);
	i2cmaster.i2c_rep_start(SLEEP_MODE);
	i2cmaster.i2c_stop();
	
	DDRC	|= (1<<PORTC4) | (1<<PORTC5); //SCL, SDA = VDD -> A0, A1, A2 = OPEN -> Under Stop Condition
	PORTC	|= (1<<PORTC4) | (1<<PORTC5);
}

void FRAM_MB85RC_I2C::wakeUp(void){
																															 
	/*********************************************************************************************************************/			
	/* Exit from Sleep mode 																							 */
	/*	a) The master sends start condition followed by device address word.											 */
	/*	In this device address word, Read/Write code are Don't care.													 */
	/*	b) At the rising edge of 9th clock from start condition, an internal regulator starts to operate its recovery	 */
	/*	sequence.																										 */
	/*	c) After the recovery time (tREC) passed, standby mode enabled.													 */
	/*	After returning to Standby mode, reading and writing are enabled by sending each command starts with start		 */
	/*	condition.																										 */
	/*********************************************************************************************************************/

	i2cmaster.i2c_start_NoWait(FRAM_ADDR);
}

uint32_t FRAM_MB85RC_I2C::readFRAM(uint8_t addr5, uint8_t addr8)
{	
	i2cmaster.i2c_start_wait(FRAM_ADDR | I2C_WRITE);
	i2cmaster.i2c_write(addr5);
	i2cmaster.i2c_write(addr8);
	
	i2cmaster.i2c_rep_start(FRAM_ADDR | I2C_READ);
	uint8_t fourthByte = i2cmaster.i2c_readAck();
	uint8_t thirdByte = i2cmaster.i2c_readAck();
	uint8_t secondByte = i2cmaster.i2c_readAck();
	uint8_t firstByte = i2cmaster.i2c_readNak();
	
	i2cmaster.i2c_stop();
	
	uint32_t data = ( ((uint32_t)fourthByte << 24) | ((uint32_t)thirdByte << 16) | ((uint32_t)secondByte << 8) | (uint32_t)firstByte);
	return data;
}

uint8_t FRAM_MB85RC_I2C::writeFRAM(uint8_t addr5, uint8_t addr8, uint32_t data)
{
	i2cmaster.i2c_start_wait(FRAM_ADDR | I2C_WRITE);
	i2cmaster.i2c_write(addr5);
	i2cmaster.i2c_write(addr8);
	
	i2cmaster.i2c_write((uint8_t)( (data >> 24) & 0xFF ) );
	i2cmaster.i2c_write((uint8_t)( (data >> 16) & 0xFF ) );
	i2cmaster.i2c_write((uint8_t)( (data >> 8)	& 0xFF ) );
	i2cmaster.i2c_write((uint8_t)( (data)		& 0xFF ) );
	i2cmaster.i2c_stop();
	
	return 0;
}
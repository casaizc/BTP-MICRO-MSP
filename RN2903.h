/*
 * RN2903.h
 *
 * Created: 18/02/2020 12:15:45 p. m.
 * Author : SEBASTIAN ARBOLEDA D
 * e-mail: sebasarboleda143@gmail.com
 */ 


#ifndef RN2903_H_
#define RN2903_H_

#define _ST_JOINED		0x00000001
#define _ST_IDLE		0x00000000
#define _ST_SILENT		0x00000040
#define _ST_REQREJOIN	0x00010000

#include <avr/io.h>
#include <util/delay.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <avr/sfr_defs.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>


struct Response{
	uint8_t valid_tx;
	int port;
	char data[4];
};
	
class RN2903{
	
	public:	
	
	void init();
	//void close();
	
	/************************************************************************/
	/*                        USART PERIPHERAL                              */
	/************************************************************************/
	void transmitByte(unsigned char);
	uint8_t readByte(void);
	void writeCmd(const char TXBuffer[], uint8_t numOfLines);
	void writeCmd_F(const char TXBuffer[], uint8_t numOfLines);	/*FLASH METHODS*/
	
	/************************************************************************/
	/*                      RADIO SETUP FUNCTIONS                           */
	/************************************************************************/
	uint8_t configOTAA(const char deveui[], const char appkey[], uint8_t lowCh, uint8_t highCh, uint8_t confCh);
	uint8_t configABP(const char deveui[], const char devaddr[], const char nwkskey[], const char appskey[], const char appkey[], uint8_t lowCh, uint8_t highCh, uint8_t confCh);
	void sleep(void);
	void wakeUp(void);
	void reset();
	uint32_t radioStatus();
	uint16_t getVdd();
	
	/************************************************************************/
	/*                       UPSTREAM FUNCTIONS                             */
	/************************************************************************/
	Response response;
	uint8_t sendtoLoRaServer_v1_response(unsigned long var, uint8_t conf = 1);
	uint8_t sendtoLoRaServer_v1(unsigned long var, uint8_t conf = 1);
	uint8_t sendtoLoRaServer_v2(uint32_t var1, uint32_t var2, uint32_t var3);	
};
extern RN2903 rn2903;
#endif /* RN2903_H_ */
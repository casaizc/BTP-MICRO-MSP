/*
 * RAK3172.h
 *
 * Created: 19/08/2021 02:37:18 p.m.
 *  Author: JULIAN_RODRIGUEZ
 */ 


#ifndef RAK3172_H_
#define RAK3172_H_

#include <avr/io.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <util/delay.h>

//BAUDRATE 2MHZ: 38.4k -> UBRRn:3 ..... 57.6k bps -> UBRRn:2
//BAUDRATE 4MHZ: 4800 -> UBRRn:103 ..... 38.4k -> UBRRn:12 ..... 57.6k bps -> UBRRn:8
//BAUDRATE 8MHZ: 38.4k -> UBRRn:25 ..... 57.6k bps -> UBRRn:16 .... 115.2k -> UBRRn:8
#define _BAUDRATE_115200 8 
#define _BAUDRATE_38400 25
#define _BAUDRATE_9600 103



class RAK3172{
	public:
	volatile char res[128];
	void init(uint8_t baudRate);
	void writeCmd(const char TXBuffer[], uint8_t numOfLines);
	uint8_t readByte(void);
	uint8_t readByte_15seg(void);
	
	void resetHw();
	void resetSw();
		
	uint8_t configRack3172(const char deveui[], const char devaddr[], const char nwkskey[], const char appskey[], const char appkey[], uint8_t lowCh, uint8_t highCh, uint8_t confCh);
	uint8_t sendtoLoRaServer_v1(unsigned long var, uint16_t batVolt = 0);
	uint8_t sendtoLoRaServer_v1_response(unsigned long var, uint16_t batVolt,uint8_t numLines);
	
};
extern RAK3172 rak3172;



#endif /* RAK3172_H_ */
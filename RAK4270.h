/*
 * RAK4270.h
 *
 * Created: 19/08/2021 02:37:18 p.m.
 *  Author: JULIAN_RODRIGUEZ
 */ 


#ifndef RAK4270_H_
#define RAK4270_H_

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

struct Response //at+recv=9,-27,3,2:0022
{
	char atRes[10];
	char port[3];
	int rssi[4];
	char snr[3];
	char dataLength[2];
	char data[10];
	uint8_t valid_tx;
};

class RAK4270{
	public:
	
	Response response;
	
	void init(uint8_t baudRate);
	void configBaudRate(void);
	void writeCmd(const char TXBuffer[], uint8_t numOfLines);
	uint8_t readByte(void);
	
	void resetHw();
	void resetSw();
	
	//uint8_t configOTAA(const char deveui[], const char appkey[], uint8_t lowCh, uint8_t highCh, uint8_t confCh);
	//uint8_t configABP(const char deveui[], const char devaddr[], const char nwkskey[], const char appskey[], const char appkey[], uint8_t lowCh, uint8_t highCh, uint8_t confCh);
	void sleep(void);
	void wakeUp(void);
	//uint32_t radioStatus();
	//uint16_t getVdd();
	
	uint8_t configRack4270(const char deveui[], const char devaddr[], const char nwkskey[], const char appskey[], const char appkey[], uint8_t lowCh, uint8_t highCh, uint8_t confCh);
	
	uint8_t sendtoLoRaServer_v1(unsigned long var, uint16_t batVolt = 0);
	uint8_t sendtoLoRaServer_v1_response(unsigned long var, uint16_t batVolt,uint8_t numLines);
	//uint8_t sendtoLoRaServer_v1_response(unsigned long var, uint16_t batVolt);
	
};
extern RAK4270 rak4270;



#endif /* RAK4270_H_ */
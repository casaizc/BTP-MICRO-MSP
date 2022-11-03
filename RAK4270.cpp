/*
* RAK4270.cpp
*
* Created: 19/08/2021 02:37:45 p.m.
*  Author: JULIAN_RODRIGUEZ
*/

#include "RAK4270.h"


RAK4270 rak4270;

void RAK4270::init(uint8_t baudRate){
	//USART0
	UCSR0C &= ~(1<<UMSEL00) & ~(1<<UMSEL01); //Set the USART mode.  In this case it is asynchronous
	UCSR0C &= ~(1<<UPM00) & ~(1<<UPM01); //no parity
	UCSR0C &= ~(1<<USBS0); //one stop bit
	UCSR0C |= (1<<UCSZ00) | (1<<UCSZ01); //8-bit
	UCSR0B &= ~(1<<UCSZ02);				 //8-bit
	UCSR0B &= ~(1<<RXCIE0) & ~(1<<TXCIE0) & ~(1<<UDRIE0); //Disable interrupts

	UBRR0H = (unsigned char)(baudRate >> 8);
	UBRR0L = (unsigned char)baudRate;
	UCSR0A |= (1<<U2X0);
	
	UCSR0B &= ~(1<<TXEN0) & ~(1<<RXEN0);	//tx/rx disable
	
}/* RAK4270_Init */

void RAK4270::writeCmd(const char TXBuffer[], uint8_t numOfLines)
{
	
	UCSR0B &= ~(1<<TXEN0) & ~(1<<RXEN0);  //tx/rx disable
	UCSR0B |= ((1<<TXEN0) |  (1<<RXEN0)); //tx/rx enable

	//WRITE CMD
	char c;
	uint8_t i = 0;

	while ((c = TXBuffer[i++]) != 0)
	{
		/* loop until the TX register is empty */
		while(!(UCSR0A & (1<<UDRE0))); //If UDRE0 is one, the buffer is empty, and therefore ready to be written.
		UDR0 = c;
	}
	
	while(!(UCSR0A & (1<<UDRE0))); //If UDRE0 is one, the buffer is empty, and therefore ready to be written.
	UDR0 = 13;
	while(!(UCSR0A & (1<<UDRE0))); //If UDRE0 is one, the buffer is empty, and therefore ready to be written.
	UDR0 = 10;
	
	while(!(UCSR0A & (1<<TXC0)));
	
	//READ CMD
	char z;
	
	for(uint8_t i = 0; i<numOfLines;i++){
		while((z = RAK4270::readByte()) != '\n'){
		}
	}
}

uint8_t RAK4270::readByte(void){
	// Check to see if something was received
	while (! (UCSR0A & (1<<RXC0)) );
	return (uint8_t) UDR0;
}

/************************************************************************/
/*                      RADIO SETUP FUNCTIONS                           */
/************************************************************************/
void RAK4270::configBaudRate(void){
	RAK4270::writeCmd("at+set_config=device:restart", 0);
	_delay_ms(100);
	RAK4270::writeCmd("at+set_config=device:uart:2:38400", 0);
	_delay_ms(100);
}

void RAK4270::wakeUp(void){
	/************************************************************************/
	/* About the UART pin. Pin 4 (TX1), pin 5 (RX1) are reserved for UART1. */
	/* Pin 2 (TX2), pin 1 (RX2) are reserved for UART2. During sleep, pin 5 */
	/* (RX1) and pin 1 (RX2) are configured as external interrupt mode, 	*/
	/* internal pulldown resistor, rising edge trigger wake-up.				*/
	/************************************************************************/
	UCSR0B &= ~(1<<TXEN0) & ~(1<<RXEN0);	//tx/rx disable
	
	//WAKE UP CONDITION:
	DDRD |= (1<<PORTD1);					//TX PIN as an OUTPUT
	
	PORTD &= ~(1<<PORTD1);					//PD1 write an 0
	_delay_ms(4);							//wait for ~4mS: TimeBreakCondition = 13/4800 = 2.7mS
	PORTD |= (1<<PORTD1);					//PD1 write an 1
	_delay_ms(500);
	
	//UCSR0B |= ((1<<TXEN0) | (1<<RXEN0));	//tx/rx enable
	RAK4270::writeCmd("at+set_config=device:sleep:0", 1);
	//RAK4270::writeCmd("at+set_config=device:restart", 0);
}

void RAK4270::resetHw(void){
	//RESET THE MODULE:
	DDRB	|= (1<<PORTB1); //reset pin for LoRa module
		
	PORTB &= ~(1<<PORTB1);
	_delay_ms(250);
	_delay_ms(250);
	PORTB |= (1<<PORTB1);
	for (uint8_t times = 0; times < 8; times++)
	{
		_delay_ms(250);
	}
}
	
void RAK4270::resetSw(void){
	/************************************************************************/
	/*                        reset Command                                 */
	/************************************************************************/
	
	RAK4270::writeCmd("at+set_config=device:restart", 0);
	
	_delay_ms(100);
	
	RAK4270::writeCmd("at+version", 1);
	
}

uint8_t RAK4270::configRack4270(const char deveui[], const char devaddr[], const char nwkskey[], const char appskey[], const char appkey[], uint8_t lowCh, uint8_t highCh, uint8_t confCh){
	
	char cmdBuffer[150];

	/************************************************************************/
	/*                     Credenciales LORAWAN                             */
	/************************************************************************/	
	sprintf(cmdBuffer,"at+set_config=lora:dev_eui:%s", deveui);
	RAK4270::writeCmd(cmdBuffer, 1);
	
	sprintf(cmdBuffer,"at+set_config=lora:dev_addr:%s", devaddr);
	RAK4270::writeCmd(cmdBuffer, 1);
	
	//ABP
	sprintf(cmdBuffer,"at+set_config=lora:apps_key:%s", appskey);
	RAK4270::writeCmd(cmdBuffer, 1);
	sprintf(cmdBuffer,"at+set_config=lora:nwks_key:%s", nwkskey);
	RAK4270::writeCmd(cmdBuffer, 1);
	
	/************************************************************************/
	/*                    Configuracion Inicial                             */
	/************************************************************************/
	
	RAK4270::writeCmd("at+set_config=lora:region:AU915", 1);
	RAK4270::writeCmd("at+set_config=lora:join_mode:1", 1);
	RAK4270::writeCmd("at+set_config=lora:class:0", 1);
	RAK4270::writeCmd("at+set_config=lora:confirm:1", 1);
	RAK4270::writeCmd("at+set_config=lora:dr:3", 1);
	RAK4270::writeCmd("at+set_config=lora:tx_power:0", 1);	//MAX TX POWER
	RAK4270::writeCmd("at+set_config=lora:adr:0", 1);
	RAK4270::writeCmd("at+set_config=lora:dutycycle_enable:0", 1);
	RAK4270::writeCmd("at+set_config=lora:send_repeat_cnt:7", 1);
	
	
	/************************************************************************/
	/*                   Configuracion de Canales                           */
	/***********************************************************************/
	
	for(uint8_t i = 0 ; i < 72 ; i++){
		sprintf(cmdBuffer,"at+set_config=lora:ch_mask:%i:0",i);
		RAK4270::writeCmd(cmdBuffer,1);
	}
	
	for(uint8_t i = lowCh ; i <= highCh ; i++){
		sprintf(cmdBuffer,"at+set_config=lora:ch_mask:%i:1",i);
		RAK4270::writeCmd(cmdBuffer,1);
	}
	sprintf(cmdBuffer,"at+set_config=lora:ch_mask:%i:1", confCh);
	RAK4270::writeCmd(cmdBuffer, 1);
	return 1;
}

void RAK4270::sleep(void){
	RAK4270::writeCmd("at+set_config=device:sleep:1", 1);
	UCSR0B &= ~(1<<TXEN0) & ~(1<<RXEN0);  //tx/rx disable
	DDRD |= (1<<DDD1);
	PORTD &= ~(1<<PORTD1);
}

/************************************************************************/
/*                       UPSTREAM FUNCTIONS                             */
/************************************************************************/
/*
uint8_t RAK4270::sendtoLoRaServer_v1_response(unsigned long var, uint16_t batVolt)//USART1
{
	char charV	[100];
	char hexV	[100];
	char DataBuffer	[150];

	char cTemp;
	
	for(uint8_t j = 0; j<100;j++){
		charV[j] = '\0';
	}
	
	sprintf(charV,"%lu,%i",var,batVolt);
	
	sprintf(DataBuffer,"at+send=lora:1:");

	
	uint8_t iCharLiter = 0;
	while((cTemp = charV[iCharLiter++]) != '\0' ){
		sprintf(hexV, "%X", cTemp);
		strcat(DataBuffer,hexV);
	}
	
	UCSR0B &= ~(1<<TXEN0) & ~(1<<RXEN0);  //tx/rx disable
	UCSR0B |= ((1<<TXEN0) |  (1<<RXEN0)); //tx/rx enable

	//WRITE CMD
	char c;
	uint8_t i = 0;

	while ((c = DataBuffer[i++]) != 0)
	{
		// loop until the TX register is empty
		while(!(UCSR0A & (1<<UDRE0))); //If UDRE0 is one, the buffer is empty, and therefore ready to be written.
		UDR0 = c;
	}
	
	while(!(UCSR0A & (1<<UDRE0))); //If UDRE0 is one, the buffer is empty, and therefore ready to be written.
	UDR0 = 13;
	while(!(UCSR0A & (1<<UDRE0))); //If UDRE0 is one, the buffer is empty, and therefore ready to be written.
	UDR0 = 10;
	
	while(!(UCSR0A & (1<<TXC0)));
	
	//leer comando
	char z;
	char res[64];
	for(uint8_t i = 0; i<64;i++){
		res[i] = '\0';
	}
	uint16_t pos = 0;
	
	for(uint8_t i = 0; i<2;i++){
		
		while((z = RAK4270::readByte()) != '\n'){
			res[pos] = z;
			pos++;
		}
		
		if(res[0] != 'O' && i == 0){
			response.valid_tx = 0;
			i = 2;
		}
		pos = 0;
	}
	
	if(res[0] == 'a' && res[1] == 't' && res[2] == '+' && res[3] == 'r' && res[4] == 'e' && res[5] == 'c' && res[6] == 'v'){
		

		//  0              1		2		3		4			5
		int parametro = 0; // at+recv      =   9    ,  -27   ,  3   ,   2     :     0022
		int indiceAT = 0;
		int indicePort = 0;
		int indiceRssi = 0;
		int indiceSnr = 0;
		int indicelenght = 0;
		int indiceData = 0;
		
		for(uint8_t i = 0; i<64;i++){
			
			if (res[i] == '=' || res[i] == ',' || res[i] == ':'  || res[i] == '\0' )
			{
				parametro++;
				continue;
			}
			if (parametro == 0)
			{
				response.atRes[indiceAT] = res[i];
				indiceAT++;
			}
			if (parametro == 1)
			{
				response.port[indicePort] = res[i];
				indicePort++;
			}
			if (parametro == 2)
			{
				response.rssi[indiceRssi] = res[i];
				indiceRssi++;
			}
			if (parametro == 3)
			{
				response.snr[indiceSnr] = res[i];
				indiceSnr++;
			}
			if (parametro == 4)
			{
				response.dataLength[indicelenght] = res[i];
				indicelenght++;
			}
			if (parametro == 5)
			{
				response.data[indiceData] = res[i];
				indiceData++;
			}
		}
		response.valid_tx = 1;
		}else{
		response.valid_tx = 0;
	}
	
	return response.valid_tx;
}
*/
uint8_t RAK4270::sendtoLoRaServer_v1_response(unsigned long var, uint16_t batVolt,uint8_t numLines)//USART1
{
	char charV	[100];
	char hexV	[100];
	char DataBuffer	[150];

	char cTemp;
	
	for(uint8_t j = 0; j<100;j++){
		charV[j] = '\0';
	}
	
	sprintf(charV,"%lu,%i",var,batVolt);
	
	sprintf(DataBuffer,"at+send=lora:1:");

	
	uint8_t iCharLiter = 0;
	while((cTemp = charV[iCharLiter++]) != '\0' ){
		sprintf(hexV, "%X", cTemp);
		strcat(DataBuffer,hexV);
	}
	
	UCSR0B &= ~(1<<TXEN0) & ~(1<<RXEN0);  //tx/rx disable
	UCSR0B |= ((1<<TXEN0) |  (1<<RXEN0)); //tx/rx enable

	//WRITE CMD
	char c;
	uint8_t i = 0;

	while ((c = DataBuffer[i++]) != 0)
	{
		// loop until the TX register is empty
		while(!(UCSR0A & (1<<UDRE0))); //If UDRE0 is one, the buffer is empty, and therefore ready to be written.
		UDR0 = c;
	}
	
	while(!(UCSR0A & (1<<UDRE0))); //If UDRE0 is one, the buffer is empty, and therefore ready to be written.
	UDR0 = 13;
	while(!(UCSR0A & (1<<UDRE0))); //If UDRE0 is one, the buffer is empty, and therefore ready to be written.
	UDR0 = 10;
	
	while(!(UCSR0A & (1<<TXC0)));
	
	//leer comando
	char z;
	char res[128];
	for(uint8_t i = 0; i<128;i++){
		res[i] = '\0';
	}
	uint16_t pos = 0;
	
	for(uint8_t i = 0; i<numLines;i++){
		
		while((z = RAK4270::readByte()) != '\n'){
			res[pos] = z;
			pos++;
		}
		
		if(res[0] != 'O' && i == 0){
			return 0;
		}
	}
	/*
	if(res[0] == 'a' && res[1] == 't' && res[2] == '+' && res[3] == 'r' && res[4] == 'e' && res[5] == 'c' && res[6] == 'v'){
		int parametro = 0; // at+recv      =   9    ,  -27   ,  3   ,   2     :     0022
		int indiceAT = 0;
		int indicePort = 0;
		int indiceRssi = 0;
		int indiceSnr = 0;
		int indicelenght = 0;
		int indiceData = 0;
		
		for(uint8_t i = 0; i<64;i++){
			
			if (res[i] == '=' || res[i] == ',' || res[i] == ':'  || res[i] == '\0' )
			{
				parametro++;
				continue;
			}
			if (parametro == 0)
			{
				response.atRes[indiceAT] = res[i];
				indiceAT++;
			}
			if (parametro == 1)
			{
				response.port[indicePort] = res[i];
				indicePort++;
			}
			if (parametro == 2)
			{
				response.rssi[indiceRssi] = res[i];
				indiceRssi++;
			}
			if (parametro == 3)
			{
				response.snr[indiceSnr] = res[i];
				indiceSnr++;
			}
			if (parametro == 4)
			{
				response.dataLength[indicelenght] = res[i];
				indicelenght++;
			}
			if (parametro == 5)
			{
				response.data[indiceData] = res[i];
				indiceData++;
			}
		}
		response.valid_tx = 1;
		}else{
		response.valid_tx = 0;
	}
	*/
	
	if(numLines == 1){
		if(res[0] == 'O' && res[1] == 'K'){
			return 1;		
		}else{
			return 0;
		}
	} else if(numLines == 2){
		if(res[0] == 'O' && res[1] == 'K' && res[3] == 'a' && res[4] == 't' && res[5] == '+' && res[6] == 'r' && res[7] == 'e' && res[8] == 'c' && res[9] == 'v'){
			return 1;
		}else{
			return 0;
		}
	}else{
		return 0;
	}
	
	
}

uint8_t RAK4270::sendtoLoRaServer_v1(unsigned long var, uint16_t batVolt)//USART1
{
	char charV	[100];
	char hexV	[100];
	char DataBuffer	[150];

	char cTemp;
	
	for(uint8_t j = 0; j<100;j++){
		charV[j] = '\0';
	}
	
	sprintf(charV,"%lu,%i",var,batVolt);
	
	sprintf(DataBuffer,"at+send=lora:1:");

	
	uint8_t iCharLiter = 0;
	while((cTemp = charV[iCharLiter++]) != '\0' ){
		sprintf(hexV, "%X", cTemp);
		strcat(DataBuffer,hexV);
	}
	
	UCSR0B &= ~(1<<TXEN0) & ~(1<<RXEN0);  //tx/rx disable
	UCSR0B |= ((1<<TXEN0) |  (1<<RXEN0)); //tx/rx enable

	//WRITE CMD
	char c;
	uint8_t i = 0;

	while ((c = DataBuffer[i++]) != 0)
	{
		// loop until the TX register is empty
		while(!(UCSR0A & (1<<UDRE0))); //If UDRE0 is one, the buffer is empty, and therefore ready to be written.
		UDR0 = c;
	}
	
	while(!(UCSR0A & (1<<UDRE0))); //If UDRE0 is one, the buffer is empty, and therefore ready to be written.
	UDR0 = 13;
	while(!(UCSR0A & (1<<UDRE0))); //If UDRE0 is one, the buffer is empty, and therefore ready to be written.
	UDR0 = 10;
	
	while(!(UCSR0A & (1<<TXC0)));
	
	//leer comando
	/*char z;
	char res[64];
	uint16_t pos = 0;
	
	for(uint8_t i = 0; i<2;i++){
		
		while((z = RAK4270::readByte()) != '\n'){
			res[pos] = z;
			pos++;
		}*/
		
		/*if(res[0] != 'O'){
			
		}*/
	//}
	
	/*if(res[0] == 'o' && res[1] == 'k' && res[3] == 'm' && res[4] == 'a' && res[5] == 'c' && res[6] == '_' && res[7] == 't' && res[8] == 'x' && res[9] == '_' && res[10] == 'o' && res[11] == 'k'){
		return 1;
	}
	else{
		return 1;
	}*/
	return 0;
}
	
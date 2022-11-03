/*
* RAK3172.cpp
*
* Created: 19/08/2021 02:37:45 p.m.
*  Author: JULIAN_RODRIGUEZ
*/

#include "RAK3172.h"


RAK3172 rak3172;

void RAK3172::init(uint8_t baudRate){
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
	
}/* RAK3172_Init */

void RAK3172::writeCmd(const char TXBuffer[], uint8_t numOfLines)
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
	numOfLines++;
		
	for(uint8_t i = 0; i<numOfLines;i++){
		while((z = RAK3172::readByte()) != '\n'){
		}
	}	
}

uint8_t RAK3172::readByte(void){
	// Check to see if something was received
	uint32_t counter = 0xEC0B3;//timeout 2 seg
	while (!(UCSR0A & (1<<RXC0)) && counter !=0 ){
		counter--;
	}
	if (counter == 0)
	{
		return 10;
	}
	return (uint8_t) UDR0;	
}

uint8_t RAK3172::readByte_15seg(void){
	// Check to see if something was received
	uint32_t counter = 0x6EA540; // timeout 15 seg
	while (!(UCSR0A & (1<<RXC0)) && counter !=0 ){
		counter--;
	}
	if (counter == 0)
	{
		return 10;
	}
	return (uint8_t) UDR0;
}

/************************************************************************/
/*                      RADIO SETUP FUNCTIONS                           */
/************************************************************************/

void RAK3172::resetHw(void){
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
	
void RAK3172::resetSw(void){
	/************************************************************************/
	/*                        reset Command                                 */
	/************************************************************************/
	
	RAK3172::writeCmd("ATZ", 10);//This command is used to trigger an MCU reset.
	RAK3172::writeCmd("AT+VER=?", 1);
	
}

uint8_t RAK3172::configRack3172(const char deveui[], const char devaddr[], const char nwkskey[], const char appskey[], const char appkey[], uint8_t lowCh, uint8_t highCh, uint8_t confCh){
	
	char cmdBuffer[150];

	/************************************************************************/
	/*                     Credenciales LORAWAN                             */
	/************************************************************************/	
		
	sprintf(cmdBuffer,"AT+DEVEUI=%s", deveui);
	RAK3172::writeCmd(cmdBuffer, 1);
	
	sprintf(cmdBuffer,"AT+DEVADDR=%s", devaddr);
	RAK3172::writeCmd(cmdBuffer, 1);
	
	//ABP
	sprintf(cmdBuffer,"AT+APPSKEY=%s", appskey);
	RAK3172::writeCmd(cmdBuffer, 1);
	sprintf(cmdBuffer,"AT+NWKSKEY=%s", nwkskey);
	RAK3172::writeCmd(cmdBuffer, 1);
	
	/************************************************************************/
	/*                    Configuracion Inicial                             */
	/************************************************************************/
	
	RAK3172::writeCmd("AT+BAND=6", 1);//(0:EU433, 1:CN470, 2:RU864, 3:IN865, 4:EU868, 5:US915, 6:AU915, 7:KR920, 8:AS923)
	RAK3172::writeCmd("AT+NJM=0", 1);//(0:ABP, 1:OTAA)
	RAK3172::writeCmd("AT+CLASS=A", 1);
	RAK3172::writeCmd("AT+CFM=1", 1);//(0:unconfirmed, 1:confirm)
	RAK3172::writeCmd("AT+ADR=0", 1);//0:disable, 1:enable)
	RAK3172::writeCmd("AT+DR=3", 1);//(0-7 corresponding to DR_X)
	RAK3172::writeCmd("AT+TXP=0", 1);	//MaxEIRP - 2*TXPower
	RAK3172::writeCmd("AT+DCS=0", 1);//(0:disable, 1:enable)
	RAK3172::writeCmd("AT+RETY=7", 1);//(0-7 rtx)
	RAK3172::writeCmd("AT+PNM=1", 1);//public network mode (0=off, 1=on)

	
	
	
	/************************************************************************/
	/*                   Configuracion de Canales                           */
	/***********************************************************************/
	RAK3172::writeCmd("AT+MASK=?", 2);//(0-7 enabled channels)
	RAK3172::writeCmd("AT+MASK=0001", 1);//(0-7 enabled channels)
	RAK3172::resetSw();
	/*
	for(uint8_t i = 0 ; i < 72 ; i++){
		sprintf(cmdBuffer,"at+set_config=lora:ch_mask:%i:0",i);
		RAK3172::writeCmd(cmdBuffer,1);
	}
	
	for(uint8_t i = lowCh ; i <= highCh ; i++){
		sprintf(cmdBuffer,"at+set_config=lora:ch_mask:%i:1",i);
		RAK3172::writeCmd(cmdBuffer,1);
	}
	sprintf(cmdBuffer,"at+set_config=lora:ch_mask:%i:1", confCh);
	RAK3172::writeCmd(cmdBuffer, 1);*/
	return 1;
}

/************************************************************************/
/*                       UPSTREAM FUNCTIONS                             */
/************************************************************************/

uint8_t RAK3172::sendtoLoRaServer_v1_response(unsigned long var, uint16_t batVolt,uint8_t numLines)//USART1
{
	char charV	[100];
	char hexV	[100];
	char DataBuffer	[150];

	char cTemp;
	
	for(uint8_t j = 0; j<100;j++){
		charV[j] = '\0';
	}
	
	sprintf(charV,"%lu,%i",var,batVolt);
	
	sprintf(DataBuffer,"AT+SEND=1:");

	
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
	
	//leer comando
	char z;
	char res[128];
	for(uint8_t i = 0; i<128;i++){
		res[i] = '\0';
	}
	uint16_t pos = 0;


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
		
	for(uint8_t i = 0; i<numLines;i++){
		
		while((z = RAK3172::readByte_15seg()) != '\n'){
			if(z != 13){
				res[pos] = z;
				pos++;
			}		
		}

		if(res[0] != 'O' && i == 1){
			return 0;
		}
	}
	//RAK3172::writeCmd(res, 1);
	
	if(numLines == 2){
		if(res[0] == 'O' && res[1] == 'K'){
			for (uint8_t i = 0; i<20; i++)
			{
				_delay_ms(100);
			}
			return 1;		
		}else{
			return 0;
		}
	} else if(numLines == 3){
		if(res[0] == 'O' && res[1] == 'K' && res[2] == '+' && res[3] == 'E' && res[4] == 'V' && res[5] == 'T' && res[6] == ':' && res[7] == 'S' && res[8] == 'E' && res[9] == 'N' && res[10] == 'D' && res[11] == ' ' && res[12] == 'C' && res[13] == 'O' && res[14] == 'N' && res[15] == 'F' && res[16] == 'I' && res[17] == 'R' && res[18] == 'M' && res[19] == 'E' && res[20] == 'D' && res[21] == ' ' && res[22] == 'O' && res[23] == 'K'){
			return 1;
		}else if(res[0] == 'O' && res[1] == 'K' && res[2] == '+' && res[3] == 'E' && res[4] == 'V' && res[5] == 'T' && res[6] == ':' && res[7] == 'R' && res[8] == 'X'){
			return 1;
		}else{
			return 0;
		}
	}else{
		return 0;
	}
	
	
}

uint8_t RAK3172::sendtoLoRaServer_v1(unsigned long var, uint16_t batVolt)//USART1
{
	char charV	[100];
	char hexV	[100];
	char DataBuffer	[150];

	char cTemp;
	
	for(uint8_t j = 0; j<100;j++){
		charV[j] = '\0';
	}
	
	sprintf(charV,"%lu,%i",var,batVolt);
	
	sprintf(DataBuffer,"AT+SEND=1:");

	
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
		
		while((z = RAK3172::readByte()) != '\n'){
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
	
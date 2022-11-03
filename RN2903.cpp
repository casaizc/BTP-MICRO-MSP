/*
 * RN2903.cpp
 *
 * Created: 18/02/2020 12:15:45 p. m.
 * Author : SEBASTIAN ARBOLEDA D
 * e-mail: sebasarboleda143@gmail.com
 */ 

#include "RN2903.h"
//BAUDRATE 2MHZ: 38.4k -> UBRRn:3 ..... 57.6k bps -> UBRRn:2
//BAUDRATE 4MHZ: 4800 -> UBRRn:103 ..... 38.4k -> UBRRn:12 ..... 57.6k bps -> UBRRn:8 
//BAUDRATE 8MHZ: 38.4k -> UBRRn:25 ..... 57.6k bps -> UBRRn:16 
#define _BAUDRATE 25 //38400	
RN2903 rn2903;

void RN2903::init(){	
	//USART0
	UCSR0C &= ~(1<<UMSEL00) & ~(1<<UMSEL01);				//Set the USART mode.  In this case it is asynchronous
	UCSR0C &= ~(1<<UPM00) & ~(1<<UPM01);					//no parity
	UCSR0C &= ~(1<<USBS0);									//one stop bit
	UCSR0C |= (1<<UCSZ00) | (1<<UCSZ01);					//8-bit
	UCSR0B &= ~(1<<UCSZ02);									//8-bit
	UCSR0B &= ~(1<<RXCIE0) & ~(1<<TXCIE0) & ~(1<<UDRIE0);	//Disable interrupts
	
															
	UBRR0H = (unsigned char)(_BAUDRATE >> 8);				//BAUDRATE			
	UBRR0L = (unsigned char)_BAUDRATE;					
	UCSR0A |= (1<<U2X0);
	
	UCSR0B |= ((1<<TXEN0) | (1<<RXEN0));					//tx/rx enable
	
	//AUTOBAUDRATE
	_delay_ms(1000);
	UCSR0B &= ~(1<<TXEN0) & ~(1<<RXEN0);					//tx/rx disable
	
	//BREAK CONDITION:
	DDRD |= (1<<PORTD1);									//TX PIN as an OUTPUT
	PORTD &= ~(1<<PORTD1);									//PD1 write an 0
	_delay_ms(4);											//wait for ~4mS: TimeBreakCondition = 13/4800 = 2.7mS
	PORTD |= (1<<PORTD1);									//PD1 write an 1
	
	UCSR0B |= (1<<TXEN0) | (1<<RXEN0);						//tx/rx enable
	
	//AUTOBAUDRATE:
	while(!(UCSR0A & (1<<UDRE0)));							//If UDRE0 is one, the buffer is empty, and therefore ready to be written.
	UDR0 = 0x55;											// Sends U character
	_delay_ms(4);
}/* RN2903_Init */

/************************************************************************/
/*                        USART PERIPHERAL                              */
/************************************************************************/
uint8_t RN2903::readByte(void){
	// Check to see if something was received
	while (! (UCSR0A & (1<<RXC0)) );
	return (uint8_t) UDR0;
}

void RN2903::transmitByte(unsigned char data ){
	
	while ( !( UCSR0A & (1<<UDRE0)) );	/* Wait for empty transmit buffer */
	UDR0 = data;						/* Put data into buffer, sends the data */
}

void RN2903::writeCmd(const char TXBuffer[], uint8_t numOfLines){
	
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
		while((z = RN2903::readByte()) != '\n'){
		}
	}
}

//Function to transmit a string in Flash
void RN2903::writeCmd_F(const char TXBuffer[], uint8_t numOfLines){
	UCSR0B &= ~(1<<TXEN0) & ~(1<<RXEN0);  //tx/rx disable
	UCSR0B |= ((1<<TXEN0) |  (1<<RXEN0)); //tx/rx enable
	
	for (uint16_t k = 0; k < strlen_P(TXBuffer); k++) {
		transmitByte(pgm_read_byte_near(TXBuffer+k));
	}
	
	while(!(UCSR0A & (1<<UDRE0))); //If UDRE0 is one, the buffer is empty, and therefore ready to be written.
	UDR0 = 13;
	while(!(UCSR0A & (1<<UDRE0))); //If UDRE0 is one, the buffer is empty, and therefore ready to be written.
	UDR0 = 10;
	
	while(!(UCSR0A & (1<<TXC0)));
	
	//READ CMD
	char z;
	
	for(uint8_t i = 0; i<numOfLines;i++){
		while((z = RN2903::readByte()) != '\n'){
		}
	}
}

/************************************************************************/
/*                      RADIO SETUP FUNCTIONS                           */
/************************************************************************/
void RN2903::wakeUp(){
	//USART0
	UCSR0C &= ~(1<<UMSEL00) & ~(1<<UMSEL01);	//Set the USART mode.  In this case it is asynchronous
	UCSR0C &= ~(1<<UPM00) & ~(1<<UPM01);		//no parity
	UCSR0C &= ~(1<<USBS0);						//one stop bit
	UCSR0C |= (1<<UCSZ00) | (1<<UCSZ01);		//8-bit
	UCSR0B &= ~(1<<UCSZ02);						//8-bit
	UCSR0B &= ~(1<<RXCIE0) & ~(1<<TXCIE0) & ~(1<<UDRIE0); //Disable interrupts

	UBRR0H = (unsigned char)(_BAUDRATE >> 8);
	UBRR0L = (unsigned char)_BAUDRATE;
	UCSR0A |= (1<<U2X0);
	
	UCSR0B |= ((1<<TXEN0) | (1<<RXEN0));		//tx/rx enable
	
	//AUTOBAUDRATE
	_delay_ms(1000);
	UCSR0B &= ~(1<<TXEN0) & ~(1<<RXEN0);	//tx/rx disable
	
	//BREAK CONDITION:
	DDRD |= (1<<PORTD1);					//TX PIN as an OUTPUT
	PORTD &= ~(1<<PORTD1);					//PD1 write an 0
	_delay_ms(4);							//wait for ~4mS: TimeBreakCondition = 13/4800 = 2.7mS
	PORTD |= (1<<PORTD1);					//PD1 write an 1
	
	UCSR0B |= (1<<TXEN0) | (1<<RXEN0);		//tx/rx enable
	
	//AUTO BAUDRATE:
	while(!(UCSR0A & (1<<UDRE0)));			//If UDRE0 is one, the buffer is empty, and therefore ready to be written.
	UDR0 = 0x55;							// Sends U character
	
	char z;
	while((z = RN2903::readByte()) != '\n'){
	}
	_delay_ms(10);
}

void RN2903::reset(){	
	//RESET THE MODULE:
	DDRB	|= (1<<PORTB1); //reset pin for LoRa module
	
	PORTB &= ~(1<<PORTB1);
	_delay_ms(250);
	_delay_ms(250);
	PORTB |= (1<<PORTB1);
	_delay_ms(20);
}

uint32_t RN2903::radioStatus(){
	
	UCSR0B &= ~(1<<TXEN0) & ~(1<<RXEN0);  //tx/rx disable
	UCSR0B |= ((1<<TXEN0) |  (1<<RXEN0)); //tx/rx enable

	//WRITE CMD PROCESS
	const char TXBuffer[] = "mac get status";
	char c;
	uint8_t iString = 0;

	while ((c = TXBuffer[iString++]) != 0)
	{
		/* loop until the TX register is empty */
		while(!(UCSR0A & (1<<UDRE0))){} //If UDRE0 is one, the buffer is empty, and therefore ready to be written.
		UDR0 = c;
	}
	
	while(!(UCSR0A & (1<<UDRE0))){} //If UDRE0 is one, the buffer is empty, and therefore ready to be written.
	UDR0 = 13;
	while(!(UCSR0A & (1<<UDRE0))){} //If UDRE0 is one, the buffer is empty, and therefore ready to be written.
	UDR0 = 10;
	
	while(!(UCSR0A & (1<<TXC0))){}
	
	//leer comando
	char z;
	char res[33];
	uint16_t pos = 0;
	
	while((z = RN2903::readByte()) != '\n'){
		res[pos] = z;
		pos++;
	}
	
	//STATUS CODE CONVERTION.	
	uint32_t statusCode = 0;
	
	for(uint8_t i=0;i<=7;i++){
		switch(res[i])
		{
			case '0': statusCode += 0 << (28-(4*i));	break;
			case '1': statusCode += 1 << (28-(4*i));	break;
			case '2': statusCode += 2 << (28-(4*i));	break;
			case '3': statusCode += 3 << (28-(4*i));	break;
			case '4': statusCode += 4 << (28-(4*i));	break;
			case '5': statusCode += 5 << (28-(4*i));	break;
			case '6': statusCode += 6 << (28-(4*i));	break;
			case '7': statusCode += 7 << (28-(4*i));	break;
			case '8': statusCode += 8 << (28-(4*i));	break;
			case '9': statusCode += 9 << (28-(4*i));	break;
			case 'a': statusCode += 10 << (28-(4*i));	break;
			case 'b': statusCode += 11 << (28-(4*i));	break;
			case 'c': statusCode += 12 << (28-(4*i));	break;
			case 'd': statusCode += 13 << (28-(4*i));	break;
			case 'e': statusCode += 14 << (28-(4*i));	break;
			case 'f': statusCode += 15 << (28-(4*i));	break;
		}
	}
	return statusCode;
}

uint8_t RN2903::configOTAA(const char deveui[], const char appkey[], uint8_t lowCh, uint8_t highCh, uint8_t confCh){
	
	char cmdBuffer	[150];
	
	//KEYS
	RN2903::writeCmd("mac reset",1);
	RN2903::writeCmd("mac set devaddr 00000000",1);							//devaddr
	RN2903::writeCmd("mac set appeui 0000000000000000",1);							//appeui	
	sprintf(cmdBuffer,"mac set deveui %s", deveui);							//deveui
	RN2903::writeCmd(cmdBuffer,1);	  
	RN2903::writeCmd("mac set nwkskey 00000000000000000000000000000000",1);	//nwkskey
	RN2903::writeCmd("mac set appskey 00000000000000000000000000000000",1);	//appskey
	sprintf(cmdBuffer,"mac set appkey %s", appkey);							//appkey
	RN2903::writeCmd(cmdBuffer,1);
	
	//MAC SET UP
	RN2903::writeCmd("mac set adr off",1);
	RN2903::writeCmd("mac set ar off",1);
	RN2903::writeCmd("mac set retx 1",1);
	RN2903::writeCmd("mac set sync 12",1);
	RN2903::writeCmd("mac set pwridx 5",1);
	
	
	//CHANNELS
	for(uint8_t i = 0 ; i < 72 ; i++){
		sprintf(cmdBuffer,"mac set ch status %i off",i);
		RN2903::writeCmd(cmdBuffer,1);
	}
	
	for(uint8_t i = lowCh ; i <= highCh ; i++){
		sprintf(cmdBuffer,"mac set ch status %i on",i);
		RN2903::writeCmd(cmdBuffer,1);
	}
	
	//rcv window ch
	sprintf(cmdBuffer,"mac set ch status %i on", confCh);
	RN2903::writeCmd(cmdBuffer,1);
	
	RN2903::writeCmd("mac save",1);
	
	return 1;
}

uint8_t RN2903::configABP(const char deveui[], const char devaddr[], const char nwkskey[], const char appskey[], const char appkey[], uint8_t lowCh, uint8_t highCh, uint8_t confCh){
	
	char cmdBuffer	[150];

/*-------------------------KEYS-------------------------------*/
	RN2903::writeCmd("mac reset",1);
	//devaddr
	sprintf(cmdBuffer,"mac set devaddr %s", devaddr);
	RN2903::writeCmd(cmdBuffer,1);						
	//appeui
	RN2903::writeCmd("mac set appeui 0000000000000000",1);							
	//deveui
	sprintf(cmdBuffer,"mac set deveui %s", deveui);							
	RN2903::writeCmd(cmdBuffer,1);
	//nwkskey
	sprintf(cmdBuffer,"mac set nwkskey %s", nwkskey);
	RN2903::writeCmd(cmdBuffer,1);	
	//appskey
	sprintf(cmdBuffer,"mac set appskey %s", appskey);
	RN2903::writeCmd(cmdBuffer,1);
	//appkey
	sprintf(cmdBuffer,"mac set appkey %s", appkey);							
	RN2903::writeCmd(cmdBuffer,1);
	
	//MAC SET UP: Set up is performed before every TX
	//RN2903::writeCmd("mac set adr off",1);
	//RN2903::writeCmd("mac set ar off",1);
	//RN2903::writeCmd("mac set retx 1",1);
	//RN2903::writeCmd("mac set sync 12",1);
	//RN2903::writeCmd("mac set pwridx 5",1);
	//RN2903::writeCmd("mac set dr 0",1);
	
	//CHANNELS
	for(uint8_t i = 0 ; i < 72 ; i++){
		sprintf(cmdBuffer,"mac set ch status %i off",i);
		RN2903::writeCmd(cmdBuffer,1);
	}
	
	for(uint8_t i = lowCh ; i <= highCh ; i++){
		sprintf(cmdBuffer,"mac set ch status %i on",i);
		RN2903::writeCmd(cmdBuffer,1);
	}
	
	//rcv window ch
	sprintf(cmdBuffer,"mac set ch status %i on", confCh);
	RN2903::writeCmd(cmdBuffer,1);
	
	RN2903::writeCmd("mac save",1);
	
	return 1;
}

void RN2903::sleep(){
	_delay_ms(50);
	RN2903::writeCmd("sys sleep 4294967293",0);
	_delay_ms(50);
}

uint16_t RN2903::getVdd(){
	
	UCSR0B &= ~(1<<TXEN0) & ~(1<<RXEN0);  //tx/rx disable
	UCSR0B |= ((1<<TXEN0) |  (1<<RXEN0)); //tx/rx enable

	//escribir comando
	//Lora_writeCmd("sys get vdd",1);
	
	const char TXBuffer[] = "sys get vdd";
	char c;
	uint8_t iString = 0;

	while ((c = TXBuffer[iString++]) != 0)
	{
		/* loop until the TX register is empty */
		while(!(UCSR0A & (1<<UDRE0))){} //If UDRE0 is one, the buffer is empty, and therefore ready to be written.
		UDR0 = c;
	}
	
	while(!(UCSR0A & (1<<UDRE0))){} //If UDRE0 is one, the buffer is empty, and therefore ready to be written.
	UDR0 = 13;
	while(!(UCSR0A & (1<<UDRE0))){} //If UDRE0 is one, the buffer is empty, and therefore ready to be written.
	UDR0 = 10;
	
	while(!(UCSR0A & (1<<TXC0))){}
	
	//leer comando
	char z;
	char vdd[10];
	uint16_t pos = 0;
	
	while((z = RN2903::readByte()) != '\n'){
		vdd[pos] = z;
		pos++;
	}
	uint16_t intVdd = atoi(vdd);
	
	return intVdd;
}

/************************************************************************/
/*                       UPSTREAM FUNCTIONS                             */
/************************************************************************/
uint8_t RN2903::sendtoLoRaServer_v1(unsigned long var, uint8_t conf){
	char charV	[150];
	char hexV	[150];
	char DataBuffer	[150];

	char cTemp;
	
	for(uint8_t j = 0; j<150;j++){
		charV[j] = '\0';
	}
	
	sprintf(charV,"%lu,%i",var,RN2903::getVdd());
	
	if (conf){
		sprintf(DataBuffer,"mac tx cnf 1 ");
	} else {
		sprintf(DataBuffer,"mac tx uncnf 1 ");
	}
	
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

	while ((c = DataBuffer[i++]) != 0){
		/* loop until the TX register is empty */
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
	char res[150];
	uint16_t pos = 0;
	
	for(uint8_t i = 0; i<2;i++){
		
		while((z = RN2903::readByte()) != '\n'){
			res[pos] = z;
			pos++;
		}
		
		if(res[0] != 'o'){
			return 0;
		}
	}
	
	if(res[0] == 'o' && res[1] == 'k' && res[3] == 'm' && res[4] == 'a' && res[5] == 'c' && res[6] == '_' && res[7] == 't' && res[8] == 'x' && res[9] == '_' && res[10] == 'o' && res[11] == 'k'){
		return 1;
	}
	else{
		return 0;
	}
}

uint8_t RN2903::sendtoLoRaServer_v2(uint32_t var1, uint32_t var2, uint32_t var3){
	UCSR0B &= ~(1<<TXEN0) & ~(1<<RXEN0);  //tx/rx disable
	UCSR0B |= ((1<<TXEN0) |  (1<<RXEN0)); //tx/rx enable

	//WRITE CMD
	
	char dataFrame[180];
	sprintf(dataFrame,"mac tx cnf 1 %07lX%07lX%05lX%03X",var1,var2,var3,RN2903::getVdd());
	char c;
	uint8_t i = 0;

	while ((c = dataFrame[i++]) != 0)
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
	
	//leer comando
	char z;
	char res[150];
	uint16_t pos = 0;
	
	for(uint8_t i = 0; i<2;i++){
		
		while((z = RN2903::readByte()) != '\n'){
			res[pos] = z;
			pos++;
		}
		if(res[0] != 'o'){
			return 0;
		}
	}
	
	if(res[0] == 'o' && res[1] == 'k' && res[3] == 'm' && res[4] == 'a' && res[5] == 'c' && res[6] == '_' && res[7] == 't' && res[8] == 'x' && res[9] == '_' && res[10] == 'o' && res[11] == 'k'){
		return 1;
	}
	else{ //else if for receiving commands
		return 0;
	}
}

//CHECK THIS CODE, IT HAS A COUPLE OF EVENTS THAT ARE NOT CONSIDERED!!!
uint8_t RN2903::sendtoLoRaServer_v1_response(unsigned long var, uint8_t conf){
	
	char charV	[150];
	char hexV	[150];
	char DataBuffer	[150];

	char cTemp;
	
	for(uint8_t j = 0; j<150;j++){
		charV[j] = '\0';
	}
	
	sprintf(charV,"%lu,%i",var,RN2903::getVdd());
	
	if (conf){
		sprintf(DataBuffer,"mac tx cnf 1 ");
	} else {
		sprintf(DataBuffer,"mac tx uncnf 1 ");
	}
	
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

	while ((c = DataBuffer[i++]) != 0){
		
		/* loop until the TX register is empty */
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
		
		while((z = RN2903::readByte()) != '\n'){
			res[pos] = z;
			pos++;
		}
		
		if(res[0] != 'o'){
			response.valid_tx = 0;
		}
	}
	
	if(res[0] == 'o' && res[1] == 'k' && res[3] == 'm' && res[4] == 'a' && res[5] == 'c' && res[6] == '_' && res[7] == 't' && res[8] == 'x' && res[9] == '_' && res[10] == 'o' && res[11] == 'k'){
		response.valid_tx = 1;
	}
	else if(res[0] == 'o' && res[1] == 'k' && res[3] == 'm' && res[4] == 'a' && res[5] == 'c' && res[6] == '_' && res[7] == 'r' && res[8] == 'x'){
		response.valid_tx = 1;
		response.port = (res[10] - '0');
		response.data[0] = res[12];
		response.data[1] = res[13];
		response.data[2] = res[14]; 
		response.data[3] = res[15]; 
	}
	else{
		response.valid_tx = 0;
	}
	return response.valid_tx;
}
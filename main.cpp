/*
 * JSM_PROD_NOEEPROM.cpp
 *
 * Created: 18/02/2020 12:15:45 p. m.
 * Author : SEBASTIAN ARBOLEDA D
 * e-mail: sebasarboleda143@gmail.com
 * 
 * VARIABLES SIZES:
 * MAX CONT1:	28 bits -> 268'435.455	(268435455)
 * MAX CONT2:	28 bits -> 268'435.455	(268435455)
 * MAX Q:		20 bits	-> 1'048.575	(1048575)
 * MAX BAT:		16 bits	-> 4.095		(4095)
 */ 


#define _NWK_S_KEY "16f19a54be94b130b0e5a7bc5c84ea6b"
#define _APP_S_KEY "f6a66e8fc525c6729d47c180e169af0e"
#define _lowCh 0 
#define _highCh 7 
#define _confCh 64

#define _APP_KEY "038e5db40782b49ae380bdd3dd14105e"

//#define _DEBUG_TEST_PULSES
//#define _RADIO_UPDATE
#define EXT_WDT 
#define _RADIO_RN2903
//#define _RADIO_RAK4270
//#define _RADIO_RAK3172

/************************************************************************/
/*                          TX TIME BASE                                */
/************************************************************************/
#ifdef EXT_WDT
	#ifdef _DEBUG_TEST_PULSES
		#define TX_TIME_INTERVAL 1		//1: 297 seconds
	#else
		#define TX_TIME_INTERVAL 12	//: 12: 1 hour - 228 seconds
	#endif
#else
	#ifdef _DEBUG_TEST_PULSES
		#define TX_TIME_INTERVAL 10		//1: 8 seconds
	#else
		#define TX_TIME_INTERVAL 450	//: 29: 1 hour
	#endif	
#endif

#include "credentials.h"
#include <avr/wdt.h>
#include <avr/io.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <avr/power.h>
#include "FRAM_MB85RC_I2C.h"

#ifdef _RADIO_RN2903
	#include "RN2903.h"
#endif

#ifdef _RADIO_RAK4270
	#include "RAK4270.h"
#endif

#ifdef _RADIO_RAK3172
#include "RAK3172.h"
#endif


/************************************************************************/
/*							VARIABLES                                   */
/************************************************************************/
//MAIN VARIABLES
volatile uint32_t p_direct /*__attribute__ ((section (".noinit")))*/;
volatile uint32_t p_reverse /*__attribute__ ((section (".noinit")))*/;

//CONTROL FLOW VARIABLES
volatile uint8_t framError = ERROR_7;
volatile uint16_t TX_time = 0;
volatile uint8_t _WDT_INT_FLAG = 0;
volatile uint8_t _WDT_EXT_FLAG = 0;
volatile uint8_t code_trigger = 0;
uint8_t oneTimeSend = 1; //Flag to send just one time



/************************************************************************/
/*						LOW POWER FUNCTION                              */
/************************************************************************/
#define	lowPowerBodOff(mode)	\
do { 							\
	set_sleep_mode(mode);		\
	cli();						\
	sleep_enable();				\
	sleep_bod_disable();		\
	sei();						\
	sleep_cpu();				\
	sleep_disable();			\
	sei();						\
} while (0);

/************************************************************************/
/*						FUNCTIONS AND ISR                               */
/************************************************************************/
void initMicro();
void LoRaTX();
uint16_t getBattery();

void intWdt_Reset();

#ifdef EXT_WDT
	void extWdt_Reset();
#endif

	
void WDT_Init(void){
	cli();
	wdt_reset();
	MCUSR &= ~(1<<WDRF);
	//set up WDT interrupt and reset
	WDTCSR |= (1<<WDCE) | (1<<WDE);
	//Start watchdog timer with 8s prescaller
	WDTCSR = 0b01101001;
	//Enable global interrupts
	sei();
}

void WDT_Stop(void){
	cli();
	wdt_reset();
	MCUSR &= ~(1<<WDRF);
	//Write logical one to WDCE and WDE
	//Keep old prescaler setting to prevent unintentional time-out
	WDTCSR |= (1<<WDCE) | (1<<WDE);
	//Turn off WDT
	WDTCSR = 0x00;
	sei();
}

ISR (WDT_vect){

#ifndef EXT_WDT 
	TX_time++;
	//_WDT_INT_FLAG++;
#endif
	
	_WDT_INT_FLAG++;
	if( (code_trigger>=0) && (code_trigger<=9) ) { //80 + 16 SECONDS MAX BEFORE RESET
		code_trigger++;
		wdt_disable();
		WDT_Init();
	}
}

/****************************************************************************/
/**Frecuencia maxima admitida para la entrada de pulsos es pulso cada 500ms**/
/****************************************************************************/

ISR(INT0_vect){
	p_direct++;
	//if(p_direct==268435455){p_direct=0;}
	
	//if(framError != ERROR_7){
		PRR &= ~(1<<PRTWI); //POWER ON TWI
		fram.init();
		fram.wakeUp();
		fram.writeFRAM(0x00,0x10,p_direct);
		fram.sleep();
		PRR |= (1<<PRTWI);
	//}

}

ISR(INT1_vect){
	p_reverse++;
	//if(p_reverse==268435455){p_reverse=0;}
		
	//if(framError != ERROR_7){
		PRR &= ~(1<<PRTWI); //POWER ON TWI
		fram.init();
		fram.wakeUp();
		fram.writeFRAM(0x00,0x14,p_reverse);
		fram.sleep();
		PRR |= (1<<PRTWI);
	//}
}

#ifdef EXT_WDT 
	ISR(PCINT0_vect) {
		// pulse every 126 seconds -> 29: for 3654seg
		if( (PINB & (1 << PINB0)) == 1 ){
			TX_time++;
			_WDT_EXT_FLAG++;
			/*PRR &= ~(1<<PRUSART0); //POWER ON UART
			rak3172.writeCmd("Wake_Pulse",1);*/
			
		}else{
			/* HIGH to LOW pin change */
		}
	}
#endif

/************************************************************************/
/*							MAIN                                        */
/************************************************************************/
int main(void){
	
	#ifdef _RADIO_UPDATE
		
		#ifdef _RADIO_RN2903
			//RESET THE MODULE:
			rn2903.reset();
		#endif
		
		while(1){
			intWdt_Reset();
			#ifdef EXT_WDT
			extWdt_Reset();
			#endif
			_delay_ms(250);
			_delay_ms(250);
			_delay_ms(250);
			_delay_ms(250);
		}
	#endif
			
	initMicro();
	
	while (1)
	{
		
		
		#ifdef EXT_WDT //REVIAR EL DISPARO DEL EXT WDT
			#warning "Chech the EXT WDT Reset trigger"	
			if (_WDT_EXT_FLAG>0)
			{
				_WDT_EXT_FLAG=0;
				extWdt_Reset();	
				/*PRR &= ~(1<<PRUSART0); //POWER ON UART
				rak3172.writeCmd("EXT_WDT",1);*/
			} else if (_WDT_INT_FLAG>0)
			{
				_WDT_INT_FLAG=0;
				intWdt_Reset();//...............................................RESTART INTERNAL WDT
				/*PRR &= ~(1<<PRUSART0); //POWER ON UART
				rak3172.writeCmd("Int_WDT",1);*/
			}
			 		
		#else
			if(_WDT_INT_FLAG>0){//..............................................ONLY RESET THE INTERNAL WDT WHEN IT HAS BEEN TRIGGERED
				_WDT_INT_FLAG=0;
				intWdt_Reset();//...............................................RESTART INTERNAL WDT
			}
		#endif
		
		
		if( (TX_time >= TX_TIME_INTERVAL) || (oneTimeSend==1) ){//......LORA TX TIMING
			oneTimeSend = 0;
			LoRaTX();
			TX_time = 0;
		}
		
		lowPowerBodOff(SLEEP_MODE_PWR_DOWN);//...........................ENTER SLEEP MODE. (Will wake up from timer overflow interrupt)
	}
}

void LoRaTX(){

#ifdef EXT_WDT
	extWdt_Reset();
#endif

	intWdt_Reset();
	
	uint32_t l = 0;
	
	if (p_direct>=p_reverse)
	{
		l = p_direct - p_reverse;
	} 
	else
	{
		l = p_direct;
	}
	
	uint16_t batVolt = 0;
			
	PRR &= ~(1<<PRUSART0); //POWER ON UART

#ifdef _RADIO_RN2903
	rn2903.reset();
	rn2903.init();

	rn2903.writeCmd("mac set adr off",1);
	rn2903.writeCmd("mac set ar off",1);
	rn2903.writeCmd("mac set retx 5",1);
	rn2903.writeCmd("mac join abp",2);
	rn2903.writeCmd("mac set sync 12",1);
	rn2903.writeCmd("mac save",1);
	
	if(rn2903.radioStatus()&_ST_SILENT){
		rn2903.writeCmd("mac forceENABLE",1);
	}
	
	rn2903.writeCmd("mac set pwridx 10",1);	//5
	rn2903.writeCmd("mac set dr 3",1);		//
	rn2903.writeCmd("mac set sync 12",1);
	
	if(!rn2903.sendtoLoRaServer_v1(l)){
		
		intWdt_Reset();
		
		if(rn2903.radioStatus()&_ST_SILENT){
			rn2903.writeCmd("mac forceENABLE",1);
		}
		
		rn2903.writeCmd("mac join abp",2);
		rn2903.writeCmd("mac set pwridx 10",1);	//5
		rn2903.writeCmd("mac set dr 1",1);
		rn2903.writeCmd("mac set sync 12",1);
		
		//Send 3 unconfirmed packets with DR=1
		for (uint8_t uncf_packets_DR1 = 0; uncf_packets_DR1<2; uncf_packets_DR1++)
		{
			rn2903.sendtoLoRaServer_v1(l, 0);
		}
		
		intWdt_Reset();
		
		rn2903.writeCmd("mac join abp",2);
		rn2903.writeCmd("mac set pwridx 10",1);	//5
		rn2903.writeCmd("mac set dr 0",1);
		rn2903.writeCmd("mac set sync 12",1);
		
		//Send 3 unconfirmed packets with DR=0
		for (uint8_t uncf_packets_DR0 = 0; uncf_packets_DR0<2; uncf_packets_DR0++)
		{
			rn2903.sendtoLoRaServer_v1(l, 0);
		}
	}
	
	rn2903.sleep();
	 
#endif	

#ifdef _RADIO_RAK4270
	
	#warning "chech getBattery function"
	batVolt = getBattery();
	
	rak4270.wakeUp();

	/************************************************************************/
	/*                          LoRaWAN TX                                  */
	/************************************************************************/
	
	rak4270.writeCmd("at+join", 1);
	rak4270.writeCmd("at+set_config=lora:confirm:1", 1);
	rak4270.writeCmd("at+set_config=lora:tx_power:5", 1);
	rak4270.writeCmd("at+set_config=lora:dr:5", 1);
	rak4270.writeCmd("at+set_config=lora:send_repeat_cnt:2", 1);
	
	
	#warning "chech RAK4270 tx response"
	
	if (!rak4270.sendtoLoRaServer_v1_response(l, batVolt, 2))
	{
		intWdt_Reset();
		rak4270.writeCmd("at+join", 1);
		rak4270.writeCmd("at+set_config=lora:confirm:0", 1);
		rak4270.writeCmd("at+set_config=lora:tx_power:5", 1);
		rak4270.writeCmd("at+set_config=lora:dr:2", 1);
		
		for (uint8_t uncf_packets_DR1 = 0; uncf_packets_DR1<1; uncf_packets_DR1++)
		{
			rak4270.sendtoLoRaServer_v1_response(l, batVolt, 1);
		}
		
		intWdt_Reset();
		rak4270.writeCmd("at+join", 1);
		rak4270.writeCmd("at+set_config=lora:confirm:0", 1);
		rak4270.writeCmd("at+set_config=lora:tx_power:0", 1);
		rak4270.writeCmd("at+set_config=lora:dr:1", 1);
		
		for (uint8_t uncf_packets_DR0 = 0; uncf_packets_DR0<1; uncf_packets_DR0++)
		{
			rak4270.sendtoLoRaServer_v1_response(l, batVolt, 1);
		}
	}
	
	
	rak4270.sleep();
	
#endif

#ifdef _RADIO_RAK3172

#warning "chech getBattery function"
batVolt = getBattery();

/************************************************************************/
/*                          LoRaWAN TX                                  */
/************************************************************************/

//AT+JOIN=Param1:Param2:Param3:Param4
//Param1: Join command: 1 for joining, 0 for stop joining
//Param2: Auto-Join config: 1 for Auto-join on power up), 0 for no auto-join. (0 is default)
//Param3: Reattempt interval: 7 - 255 seconds (8 is default)
//Param4: No. of join attempts: 0 - 255 (0 is default)
rak3172.writeCmd("AT+JOIN=1:0:10:0", 2);

rak3172.writeCmd("AT+CFM=1", 1);
rak3172.writeCmd("AT+TXP=3", 1);
rak3172.writeCmd("AT+DR=3", 1);
rak3172.writeCmd("AT+RETY=2", 1);

#warning "chech rak3172 tx response"

if (!rak3172.sendtoLoRaServer_v1_response(l, batVolt, 3))
{
	intWdt_Reset();
	rak3172.writeCmd("AT+JOIN=1:0:10:0", 2);
	rak3172.writeCmd("AT+CFM=1", 1);
	rak3172.writeCmd("AT+TXP=0", 1);
	rak3172.writeCmd("AT+DR=2", 1);
	rak3172.writeCmd("AT+RETY=1", 1);
	rak3172.sendtoLoRaServer_v1_response(l, batVolt, 3);
	
}

#endif
	
	intWdt_Reset();
	PRR |= (1<<PRUSART0); //POWER DOWN UART
}

void initMicro(){
	//uint8_t _RESET_FLAG = MCUSR;
	//MCUSR = 0x00;
	//_RESET_FLAG = 0x00;
	
	WDT_Init();	
#ifdef EXT_WDT 	
	extWdt_Reset();
#endif											
	intWdt_Reset();
		
	cli();
	
	/************************************************************************/
	/*							SET INTERRUPTS                              */
	/*	INT 0 -> PD2														*/
	/*	INT 1 -> PD3														*/
	/************************************************************************/
	
	DDRD	&= ~(1<<DDD2) & ~(1<<DDD3);					//D2 and D3 Data Direction INPUT (0)
	PORTD	&= ~(1<<PORTD2) & ~(1<<PORTD3);				// PD2 is now an input with pull-up disabled
	
	//INT0 -> p_direct
	EIMSK |= (1<<INT0);									//ENABLE INT0
	EICRA |= (1<<ISC01);								//falling EDGE INT0
	EICRA &= ~(1<<ISC00);								//falling EDGE INT0
	
	//INT1 -> p_reverse
	EIMSK |= (1<<INT1);									//ENABLE INT1
	EICRA |= (1<<ISC11);								//falling EDGE INT1
	EICRA &= ~(1<<ISC10);								//falling EDGE INT1
	
#ifdef EXT_WDT 
	//PCINT0 -> Pin Change Interrupt enable on PCINT0 for WDT wake pin
	DDRB &= ~(1 << DDB0);     //PB0 Data Direction INPUT (0)
	PORTB &= ~(1 << PORTB0);   //PB0 is now an input with pull-up enabled
	PCICR |= (1<<PCIE0);
	PCMSK0 |= (1<<PCINT0);
#endif	
	
	sei();

#ifdef _RADIO_RN2903
	/************************************************************************/
	/*                         LORAWAN RN2903                               */
	/************************************************************************/
	rn2903.reset();
	rn2903.init();
	//rn2903.configOTAA(_DEV_EUI,_APP_KEY,0,7,64);
	rn2903.configABP(_DEV_EUI, _DEV_ADDR, _NWK_S_KEY, _APP_S_KEY, _APP_KEY, _lowCh, _highCh, _confCh);
#endif

#ifdef _RADIO_RAK4270
	/************************************************************************/
	/*                         LORAWAN rak4270                               */
	/************************************************************************/
	rak4270.resetHw();
	rak4270.init(_BAUDRATE_115200);
	rak4270.configBaudRate();// Cambio de 115200  a  57600 para que funcione el uart
	rak4270.init(_BAUDRATE_38400);
	rak4270.configRack4270(_DEV_EUI, _DEV_ADDR, _NWK_S_KEY, _APP_S_KEY, _APP_KEY, _lowCh, _highCh, _confCh);
#endif

#ifdef _RADIO_RAK3172
/************************************************************************/
/*                         LORAWAN rak3172                              */
/************************************************************************/
rak3172.resetHw();
rak3172.init(_BAUDRATE_9600);
rak3172.configRack3172(_DEV_EUI, _DEV_ADDR, _NWK_S_KEY, _APP_S_KEY, _APP_KEY, _lowCh, _highCh, _confCh);
#endif
	/************************************************************************/
	/*							    FRAM					                */
	/************************************************************************/
	char errorChar[64];
	
	//I2C PORT CONFIG: for testing purposes
	DDRC	&= ~(1<<DDC4) & ~(1<<DDD5);
	PORTC  &= ~(1<<DDC4) & ~(1<<DDD5);
	
	#warning "Add new FRAM device ID"
	fram.init();
	fram.wakeUp();
	framError = fram.checkDevice();
	/*
	if(framError == ERROR_7){
		p_direct	= 0;
		p_reverse	= 0;
		
#ifdef _RADIO_RN2903
		rn2903.writeCmd("FRAM1: FRAM_NOT_DETECTED",0);
		rn2903.writeCmd("FRAM2: FRAM_NOT_DETECTED",0);
#endif	

#ifdef _RADIO_RAK4270
		rak4270.writeCmd("FRAM1: FRAM_NOT_DETECTED",0);
		rak4270.writeCmd("FRAM2: FRAM_NOT_DETECTED",0);
#endif
	
#ifdef _RADIO_RAK3172
		rak3172.writeCmd("FRAM1: FRAM_NOT_DETECTED",0);
		rak3172.writeCmd("FRAM2: FRAM_NOT_DETECTED",0);
#endif

	}else{*/
		//fram.writeFRAM(0x00,0x10,0);
	    //fram.writeFRAM(0x00,0x14,0);
		p_direct	= fram.readFRAM(0x00, 0x10);
		p_reverse	= fram.readFRAM(0x00, 0x14);
#ifdef _RADIO_RN2903		
		sprintf(errorChar,"FRAM1: %lu",p_direct);
		rn2903.writeCmd(errorChar,0);
		sprintf(errorChar,"FRAM2: %lu",p_reverse);
		rn2903.writeCmd(errorChar,0);
#endif	

#ifdef _RADIO_RAK4270		
		sprintf(errorChar,"at+send=uart:1:FRAM1_%lu",p_direct);
		rak4270.writeCmd(errorChar,1);
		sprintf(errorChar,"at+send=uart:1:FRAM2_%lu",p_reverse);
		rak4270.writeCmd(errorChar,1);
#endif	

#ifdef _RADIO_RAK3172
		sprintf(errorChar,"FRAM1_%lu",p_direct);
		rak3172.writeCmd(errorChar,1);
		sprintf(errorChar,"FRAM2_%lu",p_reverse);
		rak3172.writeCmd(errorChar,1);
#endif
	
		fram.sleep();
	//}
		
	/************************************************************************/
	/*							POWER DOWN                                  */
	/************************************************************************/

#ifdef _RADIO_RN2903
	rn2903.sleep();
#endif

#ifdef _RADIO_RAK4270
	rak4270.sleep();
#endif

	cli();
	ACSR = (1<<ACD);										// disable A/D comparator
	ADCSRA = (0<<ADEN);										// disable A/D converter
	DIDR0 = 0xff;											// disable all A/D inputs (ADC0-ADC5) //3f
	DIDR1 = 0x03;											// disable AIN0 and AIN1
	PRR |= (1<<PRTWI)|(1<<PRTIM2)|(1<<PRTIM0)|(1<<PRTIM1)|(1<<PRSPI)|(1<<PRUSART0)|(1<<PRADC);	//Disable TWI, Timer0/1/2 , SPI, UART0, ADC.
	sei();
}

uint16_t getBattery(){
	
	float fNumber = 0;
	uint16_t batVolt = 0;

	PRR &= ~(1<<PRADC);
	_delay_ms(5);
	DIDR0 |= (1<<ADC0D);
	
	ADCSRA |= (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1);	//ADC input frequency is between 50 KHz and 200 KHz.
	//The ADC clock is derived from the system clock.
	//With a system frequency of 8 MHz, a prescaler of 64 will result in an ADC frequency of 125Khz
	
	ADCSRB &= ~(1 << ADTS2) & ~(1 << ADTS1) & ~(1 << ADTS0); //free running mode
	ADMUX &= ~(1<<MUX3) & ~(1<<MUX2) & ~(1<<MUX1) & ~ (1 << MUX0); // Seleccion Chanel 0
	ADMUX |= (1<<REFS1) | (1<<REFS0); // referencia de 1.1V
	_delay_ms(10);
	
	ADCSRA|=(1<<ADSC); //Start Single conversion
	while(!(ADCSRA & (1<<ADIF)));//while((ADCSRA & (1<<ADSC))); //Wait for conversion to complete
	batVolt = ADC;
	//ADCSRA|=(1<<ADIF); //Clear ADIF by writing one to it
	
	ADCSRA|=(1<<ADSC); //Start Single conversion
	while(!(ADCSRA & (1<<ADIF)));//while((ADCSRA & (1<<ADSC))); //Wait for conversion to complete
	batVolt = ADC;
	//ADCSRA|=(1<<ADIF); //Clear ADIF by writing one to it
	//float relacion = 2.127;// (R2/R1)  //  10M / 4.7M
	//float relacion = 2.564;// (R2/R1)  //  10M / 3.9M
	
	/*
	Formula: Vbat = Vout * (R1 + R2)/(R2)
	O
	Formula: Vbat = Vout * (R1/R2 + 1)
	*/
	
	uint16_t relacion = 3564;// (R2/R1)  //  10M / 3.9M
	fNumber = ((float)batVolt*1.1) / 1023;  // Vin = ADC * Vref / 1024
	fNumber = fNumber * relacion; //Vbat = Vin(1 + R2/R1)
	batVolt = (uint16_t)fNumber;
	fNumber = 0;

	DIDR0 |= (1<<ADC0D) ;
	ADCSRA &= ~(1 << ADEN);
	PRR |= (1<<PRADC);
	return batVolt;
}

void intWdt_Reset(){
	/************************************************************************/
	/*							INTERNAT WDT RESET				            */
	/************************************************************************/
	code_trigger = 0;
	wdt_reset();
}

#ifdef EXT_WDT
void extWdt_Reset(){
	/************************************************************************/
	/*							EXTERNAL WDT RESET				            */
	/************************************************************************/
	DDRB	|= (1<<PORTB2);			//Declared as an output
	PORTB	&= ~(1<<PORTB2);		//write zero logic bit
	
	PORTB |= (1<<PORTB2);			//write zero logic bit
	_delay_us(1);					//write zero logic bit
	PORTB &= ~(1<<PORTB2);			//write zero logic bit
	/*PRR &= ~(1<<PRUSART0); //POWER ON UART
	rak3172.writeCmd("Done_Pulse",1);*/
}
#endif
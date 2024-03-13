// This program shows how to measure the period of a signal using timer 1 free running counter.

#define F_CPU 16000000UL
#include <stdio.h>
#include <stdlib.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include "usart.h"
#include "lcd.h"
#include <util/delay.h>
#define parapalegic 20.0
#define RA 99000.0
#define RB 100000.0
#define CAP 0.00000001012
#define PIN_PERIOD (PINB & 0b00000010)

/* Pinout for DIP28 ATMega328P:

                           -------
     (PCINT14/RESET) PC6 -|1    28|- PC5 (ADC5/SCL/PCINT13)
       (PCINT16/RXD) PD0 -|2    27|- PC4 (ADC4/SDA/PCINT12)
       (PCINT17/TXD) PD1 -|3    26|- PC3 (ADC3/PCINT11)
      (PCINT18/INT0) PD2 -|4    25|- PC2 (ADC2/PCINT10)
 (PCINT19/OC2B/INT1) PD3 -|5    24|- PC1 (ADC1/PCINT9)
    (PCINT20/XCK/T0) PD4 -|6    23|- PC0 (ADC0/PCINT8)
                     VCC -|7    22|- GND
                     GND -|8    21|- AREF
(PCINT6/XTAL1/TOSC1) PB6 -|9    20|- AVCC
(PCINT7/XTAL2/TOSC2) PB7 -|10   19|- PB5 (SCK/PCINT5)
   (PCINT21/OC0B/T1) PD5 -|11   18|- PB4 (MISO/PCINT4)
 (PCINT22/OC0A/AIN0) PD6 -|12   17|- PB3 (MOSI/OC2A/PCINT3)
      (PCINT23/AIN1) PD7 -|13   16|- PB2 (SS/OC1B/PCINT2)
  (PCINT0/CLKO/ICP1) PB0 -|14   15|- PB1 (OC1A/PCINT1)
                           -------
*/

unsigned int cnt = 0;

void wait_1ms(void)
{
	unsigned int saved_TCNT1;
	
	saved_TCNT1=TCNT1;
	
	while((TCNT1-saved_TCNT1)<(F_CPU/1000L)); // Wait for 1 ms to pass
}

void waitms(int ms)
{
	while(ms--) wait_1ms();
}

#define PIN_PERIOD (PINB & 0b00000010)
#define PIN_PERIODL (PINB & 0b00000100)

// GetPeriod() seems to work fine for frequencies between 30Hz and 300kHz.
long int GetPeriod (int n)
{
	int i, overflow;
	unsigned int saved_TCNT1a, saved_TCNT1b;
	
	overflow=0;
	TIFR1=1; // TOV1 can be cleared by writing a logic one to its bit location.  Check ATmega328P datasheet page 113.
	while (PIN_PERIOD!=0) // Wait for square wave to be 0
	{
		if(TIFR1&1)	{ TIFR1=1; overflow++; if(overflow>5) return 0;}
	}
	overflow=0;
	TIFR1=1;
	while (PIN_PERIOD==0) // Wait for square wave to be 1
	{
		if(TIFR1&1)	{ TIFR1=1; overflow++; if(overflow>5) return 0;}
	}
	
	overflow=0;
	TIFR1=1;
	saved_TCNT1a=TCNT1;
	for(i=0; i<n; i++) // Measure the time of 'n' periods
	{
		while (PIN_PERIOD!=0) // Wait for square wave to be 0
		{
			if(TIFR1&1)	{ TIFR1=1; overflow++; if(overflow>1024) return 0;}
		}
		while (PIN_PERIOD==0) // Wait for square wave to be 1
		{
			if(TIFR1&1)	{ TIFR1=1; overflow++; if(overflow>1024) return 0;}
		}
	}
	saved_TCNT1b=TCNT1;
	if(saved_TCNT1b<saved_TCNT1a) overflow--; // Added an extra overflow.  Get rid of it.

	return overflow*0x10000L+(saved_TCNT1b-saved_TCNT1a);
}

// GetPeriod() seems to work fine for frequencies between 30Hz and 300kHz.
long int GetPeriodL (int n)
{
	int i, overflow;
	unsigned int saved_TCNT1a, saved_TCNT1b;
	
	overflow=0;
	TIFR1=1; // TOV1 can be cleared by writing a logic one to its bit location.  Check ATmega328P datasheet page 113.
	while (PIN_PERIODL!=0) // Wait for square wave to be 0
	{
		if(TIFR1&1)	{ TIFR1=1; overflow++; if(overflow>5) return 0;}
	}
	overflow=0;
	TIFR1=1;
	while (PIN_PERIODL==0) // Wait for square wave to be 1
	{
		if(TIFR1&1)	{ TIFR1=1; overflow++; if(overflow>5) return 0;}
	}
	
	overflow=0;
	TIFR1=1;
	saved_TCNT1a=TCNT1;
	for(i=0; i<n; i++) // Measure the time of 'n' periods
	{
		while (PIN_PERIODL!=0) // Wait for square wave to be 0
		{
			if(TIFR1&1)	{ TIFR1=1; overflow++; if(overflow>1024) return 0;}
		}
		while (PIN_PERIODL==0) // Wait for square wave to be 1
		{
			if(TIFR1&1)	{ TIFR1=1; overflow++; if(overflow>1024) return 0;}
		}
	}
	saved_TCNT1b=TCNT1;
	if(saved_TCNT1b<saved_TCNT1a) overflow--; // Added an extra overflow.  Get rid of it.

	return overflow*0x10000L+(saved_TCNT1b-saved_TCNT1a);
}


void Configure_Pins(void)
{
	DDRB|=0b00000001; // PB0 is output.
	DDRD|=0b11111000; // PD3, PD4, PD5, PD6, and PD7 are outputs.
}

void LCD_pulse (void)
{
	LCD_E_1;
	_delay_us(40);
	LCD_E_0;
}

void LCD_byte (unsigned char x)
{
	//Send high nible
	if(x&0x80) LCD_D7_1; else LCD_D7_0;
	if(x&0x40) LCD_D6_1; else LCD_D6_0;
	if(x&0x20) LCD_D5_1; else LCD_D5_0;
	if(x&0x10) LCD_D4_1; else LCD_D4_0;
	LCD_pulse();
	_delay_us(40);
	//Send low nible
	if(x&0x08) LCD_D7_1; else LCD_D7_0;
	if(x&0x04) LCD_D6_1; else LCD_D6_0;
	if(x&0x02) LCD_D5_1; else LCD_D5_0;
	if(x&0x01) LCD_D4_1; else LCD_D4_0;
	LCD_pulse();
}

void WriteData (unsigned char x)
{
	LCD_RS_1;
	LCD_byte(x);
	_delay_ms(2);
}

void WriteCommand (unsigned char x)
{
	LCD_RS_0;
	LCD_byte(x);
	_delay_ms(5);
}

void LCD_4BIT (void)
{
	LCD_E_0; // Resting state of LCD's enable is zero
	//LCD_RW=0; // We are only writing to the LCD in this program
	_delay_ms(20);
	// First make sure the LCD is in 8-bit mode and then change to 4-bit mode
	WriteCommand(0x33);
	WriteCommand(0x33);
	WriteCommand(0x32); // Change to 4-bit mode

	// Configure the LCD
	WriteCommand(0x28);
	WriteCommand(0x0c);
	WriteCommand(0x01); // Clear screen command (takes some time)
	_delay_ms(20); // Wait for clear screen command to finsih.
}

void LCDprint(char * string, unsigned char line, unsigned char clear)
{
	int j;

	WriteCommand(line==2?0xc0:0x80);
	_delay_ms(5);
	for(j=0; string[j]!=0; j++)	WriteData(string[j]);// Write the message
	if(clear) for(; j<CHARS_PER_LINE; j++) WriteData(' '); // Clear the rest of the line
}


int main(void)
{
	long int count;
	long int countL;
	int counter=0;
	float T, f;
	float TL, fL;
	float fav=0;
	float favL=0;
	float capacitance;
	float inductance;
	float cap_err;
	float ind_err;
	float min=10000000000;
	float minL=10000000000;
	int swap=3;
	float RA_temp;
	float RB_temp;
	float RA_temp_rn;
	float RB_temp_rn;
	
	usart_init(); // Configure the usart and baudrate
	
	// Configure PB2, PB1, and PD2 as inputs with pull-up resistors
	DDRB &= 0b11111101; // Configure PB2 as input
	PORTB |= 0b00000100; // Activate pull-up on PB2
	
	DDRB &= 0b11111110; // Configure PB1 as input
	PORTB |= 0b00000010; // Activate pull-up on PB1
	
	DDRD &= 0b11111011; // Configure PD2 as input
	PORTD |= 0b00000100; // Activate pull-up on PD2
	
	char buff[17];

	usart_init(); // configure the usart and baudrate
	Configure_Pins();
	LCD_4BIT();
	
	_delay_ms(500); // Give putty some time to start.
	printf("ATMega328P 4-bit LCD test.\n");

   	// Display something in the LCD
	//LCDprint("LCD 4-bit test:", 1, 1);
	//LCDprint("Hello, World!", 2, 1);

	// Turn on timer with no prescaler on the clock.  We use it for delays and to measure period.
	TCCR1B |= _BV(CS10); // Check page 110 of ATmega328P datasheet

	waitms(500); // Wait for putty to start
	printf("Period measurement using the free running counter of timer 1.\n"
	       "Connect signal to PB1 (pin 15).\n");
	
	while (1)
	{
		while(counter<parapalegic){
		
				if ((PIND & 0b00000100) == 0)
{
    while((PIND & 0b00000100) == 0);
    
    if(swap==0){
    	swap=1;
    	}
    else if(swap==1){
    	swap=2;
    }
    else if(swap==2){
    	swap=3;
    }
    else{
    	swap=0;
    	}	
    // The pushbutton is pressed
    // Add your code here to handle the button press
    
}

		
		count=GetPeriod(100);
		if(count>0)
		{
			T=count/(F_CPU*100.0);
			f=1/T;
			printf("\r%lf\n",f);
			if (f<min) {
				min=f;
				
			}
			fav=fav+f;
			
		countL=GetPeriodL(100);
		if(countL>0)
		{
			TL=countL/(F_CPU*100.0);
			fL=1/TL;
			//printf("%lf\n",fL);
			if (fL<minL) {
				minL=fL;
				
			}
			favL=favL+fL;
		}	
		
			
			//printf("f=%fHz (count=%lu)     \r", f, count);
			
		}
		else
		{
			printf("NO SIGNAL                     \r");
		}
		
		counter++;
	  }
	  if(swap==0){
	  	cap_err=(1.44/((RA+2.0*RB)*min))*1000000;
	  //printf("f=%fHz (count=%lu)     \r", f, count/parapalegic);
	  counter=0;
	  capacitance=(1.44/((RA+2.0*RB)*(fav/parapalegic)))*1000000;
	 // printf("f=%fHz cap=%2.6f uf  %lf \r", fav/parapalegic, capacitance, cap_err-capacitance);
	  sprintf(buff, "cap=%2.5lf uF", capacitance);
	  LCDprint(buff, 1,1);
	  sprintf(buff, " +_ %4.7lf uf", (cap_err-capacitance)*10.0);
	  LCDprint(buff,2,1);
	  fav=0;
	  min=10000000000;
	  	printf("fluffy bunnies\r");
	  	}
	  else if(swap==1)
	  	{
	  	RA_temp = (1.44/(CAP*fav/parapalegic))-2*RB;
	  	RA_temp_rn = (1.44/(CAP*min))-2*RB;
	  	sprintf(buff, "RA=%.1fohms", RA_temp);
	  	LCDprint(buff, 1,1);
	  	sprintf(buff, "  +-  %4.1lfohms", RA_temp_rn - RA_temp);
	  	LCDprint(buff, 2,1);
	  	counter=0;
	  	 fav=0;
	  	min=10000000000;
	  	printf("fluffy bunnies\r");
	  	}
	  else if(swap==2)
	  	{
	  	RB_temp = ((1.44/(CAP*fav/parapalegic))-RA)/2.0;
	  	RB_temp_rn = ((1.44/(CAP*min))-RA)/2.0;
	  	
	  	sprintf(buff, "RB=%.1fohms", RB_temp);
	  	LCDprint(buff, 1,1);
	  	sprintf(buff, "  +-  %4.2lfohms", RB_temp_rn - RB_temp);
	  	LCDprint(buff, 2,1);
	  	counter=0;
	  	 fav=0;
	  	min=10000000000;
	  	printf("fluffy bunnies\r");
	  	}
	  else if(swap==3){
	  	ind_err=(1/(((RA+2*RB)*(minL)*2*3.1415926535)*CAP/1.8));
	  //printf("f=%fHz (count=%lu)     \r", f, count/parapalegic);
	  counter=0;
	  inductance=(1/(((RA+2*RB)*(favL/parapalegic)*2*3.1415926535)*CAP/1.85));
	 // printf("f=%fHz cap=%2.6f uf  %lf \r", fav/parapalegic, capacitance, cap_err-capacitance);
	  sprintf(buff, "ind=%2.5lf H", inductance);
	  LCDprint(buff, 1,1);
	  sprintf(buff, " +_ %4.7lf H", (ind_err-inductance)*-1.0);
	  LCDprint(buff,2,1);
	  favL=0;
	  minL=10000000000;
	  	printf("fluffy bunnies\r");
	  	}
	}
	
	
}

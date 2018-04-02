/*
 * tracker.c
 *
 * Git practice
 * Created: 3/16/2018 11:15:58 PM
 * Author : Eric Soderberg
 */
#define F_CPU 32000000
#include <stdio.h>
#include <avr/io.h>
#include <util/delay.h>
#include "spic.h"
#include "ICM20602read_inu.h"

void Config2MHzClock(void){
	CCP = CCP_IOREG_gc; //Security Signature to modify clock
	// initialize clock source to be 2MHz internal oscillator (no PLL)
	OSC.CTRL = OSC_RC2MEN_bm; // enable internal 2MHz oscillator
	while(!(OSC.STATUS & OSC_RC2MRDY_bm)); // wait for oscillator ready
	CCP = CCP_IOREG_gc; //Security Signature to modify clock
	CLK.CTRL = 0x00; //select sysclock 2MHz from 8 Mhz clock
	}

void rtc_init(){
	OSC.CTRL |= OSC_RC32KEN_bm;
	while ( !( OSC_STATUS & OSC_RC32KRDY_bm ) ); /* Wait for the int. 32kHz oscillator to stabilize. */
	CLK.RTCCTRL = CLK_RTCEN_bm | 0x0C;//0x0C is for 32.768kHz internal oscillator
	while (RTC.STATUS & RTC_SYNCBUSY_bm);
	RTC.PER = 65534;
	RTC.CTRL = RTC_PRESCALER_DIV256_gc;}

void pwm_init(){//TCD5
	//set up for PWM frequency mode
	TCD5.CTRLB = 0;
	TCD5.CTRLA = TC_CLKSEL_DIV2_gc;
	//output compare enabled
	TCD5.CTRLE = TC5_CCAMODE0_bm;

	TCD5.PER = 181;//181 for 2.75khz at 2 Mhz clock ----  @32mhz clock  2000 for 4 khz     ---  2910 for 2.75khz
	TCD5.CCA = 90;//                                      @32MHz        1000 for 4 khz     ---  1454 for 2.75 khz
																}

void io_init(){
//set PC pin 2/3 to output
PORTC.DIRSET = PIN1_bm | PIN2_bm | PIN3_bm | PIN4_bm | PIN5_bm | PIN6_bm | PIN7_bm;
PORTA.DIRSET = PIN1_bm | PIN2_bm | PIN3_bm | PIN4_bm | PIN5_bm | PIN6_bm | PIN7_bm;
PORTD.DIRSET = PIN1_bm | PIN2_bm | PIN3_bm | PIN4_bm | PIN5_bm | PIN6_bm | PIN7_bm;}

struct axis_long{	int32_t x;
					int32_t y;
					int32_t z;};

struct axis_long a[52];

unsigned int i;



//////////////////////////////////////////////////////////////////////////////

int main(void){//Main

// configure sysclk=2MHz RC oscillator
Config2MHzClock();

//init clock an io pins
rtc_init();
io_init();

//configure pwm output
pwm_init();

//init spi on portc
master_PORTC_SPI_init();

//init gyro
ICM20602Gyro_init(1);

//power reduction setup
PR.PRGEN = 0x83;
PR.PRPA  = 0x07;
PR.PRPC  = 0x57;
PR.PRPD  = 0x5D;

PORTA.OUTSET = PIN4_bm;

//////////////// Main Loop /////////////////////////////////////////////////////
while (1){

a[0].y=ax_read(1);

for(i=0; i<40; i++){a[40-i].y=a[39-i].y;
					a[51].y+=a[40-i].y;}

if (a[51].y>250000)	PORTA.OUTSET = PIN2_bm;
if (a[51].y<-50000)	PORTA.OUTCLR = PIN2_bm;
a[51].y=0;

if (RTC.CNT < 12 )		{TCD5.CTRLGCLR = (1<<5);
						PORTA.OUTSET=PIN4_bm;}
				else	{TCD5.CTRLGSET = (1<<5);
						PORTA.OUTCLR=PIN4_bm;}

if (RTC.CNT > 600) {RTC.CNT=0;}

    }// End Main Loop

}//End Main


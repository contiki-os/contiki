#define GPIO_PAD_DIR0   0x80000000
#define GPIO_DATA0      0x80000008

#define GPIO_FUNC_SEL0  0x80000018 /* GPIO 15 - 0;  2 bit blocks */

#define GPIO_PAD_PU_EN0 0x80000010
#define GPIO_PAD_PU_EN1 0x80000014
#define ADC_CONTROL     0x80000018

#define CRM_WU_CNTL     0x80003004
#define CRM_WU_TIMEOUT  0x80003024
#define CRM_SLEEP_CNTL  0x80003008
#define CRM_STATUS      0x80003018
#define CRM_XTAL_CNTL   0x80000040

#define BASE_UART1      0x80005000
#define UART1_CON       0x80005000
#define UART1_STAT      0x80005004
#define UART1_DATA      0x80005008
#define UR1CON          0x8000500c
#define UT1CON          0x80005010
#define UART1_CTS       0x80005014
#define UART1_BR        0x80005018

#define DELAY 400000

#define USE_32KHZ 1

#include "embedded_types.h"
#include "isr.h"
#include "utils.h"
#include "maca.h"
#include "crm.h"

void putc(uint8_t c);
void puts(uint8_t *s);
void put_hex(uint8_t x);
void put_hex16(uint16_t x);
void put_hex32(uint32_t x);

const uint8_t hex[16]={'0','1','2','3','4','5','6','7',
		 '8','9','a','b','c','d','e','f'};


typedef void (*pfCallback_t)(void);

typedef struct
{
	uint8_t sleepType:1;// 0 hibernate / 1 doze                                                                                                                 
	uint8_t ramRet:2;
	uint8_t mcuRet:1;
	uint8_t digPadRet:1;
	pfCallback_t  pfToDoBeforeSleep;
}crmSleepCtrl_t;

void do_nothing(void) {
	return;
}

void (*crm_gotosleep)(crmSleepCtrl_t *foo) = 0x0000364d;

__attribute__ ((section ("startup"))) void main(void) {
	crmSleepCtrl_t  crmSleepCtrl;

	reg32(GPIO_PAD_DIR0) = 0x00000100;

	reg32(GPIO_DATA0) = 0x00000100;


	/* Restore UART regs. to default */
	/* in case there is still bootloader state leftover */

	*(volatile uint32_t *)UART1_CON = 0x0000c800; /* mask interrupts, 16 bit sample --- helps explain the baud rate */

	/* INC = 767; MOD = 9999 works: 115200 @ 24 MHz 16 bit sample */
	#define INC 767
	#define MOD 9999
	*(volatile uint32_t *)UART1_BR = INC<<16 | MOD; 

	/* see Section 11.5.1.2 Alternate Modes */
	/* you must enable the peripheral first BEFORE setting the function in GPIO_FUNC_SEL */
	/* From the datasheet: "The peripheral function will control operation of the pad IF */
	/* THE PERIPHERAL IS ENABLED. */
	*(volatile uint32_t *)UART1_CON = 0x00000003; /* enable receive and transmit */
	*(volatile uint32_t *)GPIO_FUNC_SEL0 = ( (0x01 << (14*2)) | (0x01 << (15*2)) ); /* set GPIO15-14 to UART (UART1 TX and RX)*/

	reg32(0x00401ffc) = 0x01234567;
	reg32(0x00407ffc) = 0xdeadbeef;
	reg32(0x0040fffc) = 0xface00ff;
	reg32(0x00410000) = 0xabcd0123;

	puts("sleep test\n\r");
	puts("0x00401ffc: ");
	put_hex32(reg32(0x00401ffc));
	puts("\r\n");
	puts("0x00407ffc: ");
	put_hex32(reg32(0x00407ffc));
	puts("\r\n");
	puts("0x0040fffc: ");
	put_hex32(reg32(0x0040fffc));
	puts("\r\n");
	puts("0x00410000: ");
	put_hex32(reg32(0x00410000));
	puts("\r\n");

	/* radio must be OFF before sleeping */
	/* otherwise MCU will not wake up properly */
	/* this is undocumented behavior */
	radio_off();

	/* disable all pullups */
	/* seems to make a slight difference (2.0uA vs 1.95uA)*/
//	reg32(GPIO_PAD_PU_EN0) = 0;
//	reg32(GPIO_PAD_PU_EN1) = 0;
//	reg16(ADC_CONTROL) = 0; /* internal Vref2 */

//	reg16(CRM_XTAL_CNTL) = 0x052; /* default is 0xf52 */ /* doesn't anything w.r.t. power */

#if USE_32KHZ
	/* turn on the 32kHz crystal */
	puts("enabling 32kHz crystal\n\r");
	/* you have to hold it's hand with this on */
	/* once you start the 32xHz crystal it can only be stopped with a reset (hard or soft) */
	/* first, disable the ring osc */
	clear_bit(reg32(CRM_RINGOSC_CNTL),0);
	/* enable the 32kHZ crystal */
	set_bit(reg32(CRM_XTAL32_CNTL),0);
//	reg32(CRM_XTAL_CNTL) = 0;

	/* set the XTAL32_EXISTS bit */
	/* the datasheet says to do this after you've check that RTC_COUNT is changing */
	/* the datasheet is not correct */
	set_bit(reg32(CRM_SYS_CNTL),5);
	{
		static volatile uint32_t old;
		old = reg32(CRM_RTC_COUNT);
		puts("waiting for xtal\n\r");
		while(reg32(CRM_RTC_COUNT) == old) { 
//			put_hex32(reg32(CRM_RTC_COUNT));
//			puts("\n\r");
			continue; 
		}
		/* RTC has started up */

		set_bit(reg32(CRM_SYS_CNTL),5);
		puts("32kHZ xtal started\n\r");

/* 		while(1) { */
/* 			put_hex32(reg32(CRM_RTC_COUNT)); */
/* 			puts("\n\r"); */
/* 		} */

	}
#endif	
		

	/* go to sleep */
//	reg32(CRM_WU_CNTL) = 0; /* don't wake up */
//	reg32(CRM_WU_CNTL) = 0x1; /* enable wakeup from wakeup timer */
//	reg32(CRM_WU_TIMEOUT) = 1875000; /* wake 10 sec later if doze */
//	reg32(CRM_WU_TIMEOUT) = 20000; /* wake 10 sec later  if hibernate w/2kHz*/

//	reg32(CRM_SLEEP_CNTL) = 1; /* hibernate, RAM page 0 only, don't retain state, don't power GPIO */ /* approx. 2.0uA */
//	reg32(CRM_SLEEP_CNTL) = 0x41; /* hibernate, RAM page 0 only, retain state, don't power GPIO */ /* approx. 10.0uA */
//	reg32(CRM_SLEEP_CNTL) = 0x51; /* hibernate, RAM page 0&1 only, retain state, don't power GPIO */ /* approx. 11.7uA */
//	reg32(CRM_SLEEP_CNTL) = 0x61; /* hibernate, RAM page 0,1,2 only, retain state, don't power GPIO */ /* approx. 13.9uA */
//	reg32(CRM_SLEEP_CNTL) = 0x71; /* hibernate, all RAM pages, retain state, don't power GPIO */ /* approx. 16.1uA - possibly with periodic refresh*/
//	reg32(CRM_SLEEP_CNTL) = 0xf1; /* hibernate, all RAM pages, retain state,       power GPIO */ /* approx. 16.1uA - possibly with periodic refresh*/

//	reg32(CRM_SLEEP_CNTL) = 2; /* doze     , RAM page 0 only, don't retain state, don't power GPIO */ /* approx. 69.2 uA */
//	reg32(CRM_SLEEP_CNTL) = 0x42; /* doze     , RAM page 0 only, retain state, don't power GPIO */ /* approx. 77.3uA */
//	reg32(CRM_SLEEP_CNTL) = 0x52; /* doze     , RAM page 0&1 only, retain state, don't power GPIO */ /* approx. 78.9uA */
//	reg32(CRM_SLEEP_CNTL) = 0x62; /* doze     , RAM page 0,1,2 only, retain state, don't power GPIO */ /* approx. 81.2uA */
//	reg32(CRM_SLEEP_CNTL) = 0x72; /* doze     , all RAM pages, retain state, don't power GPIO */ /* approx. 83.4uA - possibly with periodic refresh*/
//	reg32(CRM_SLEEP_CNTL) = 0xf2; /* doze     , all RAM pages, retain state,       power GPIO */ /* approx. 82.8uA - possibly with periodic refresh*/


/* 	crmSleepCtrl.sleepType = 0; */
/* 	crmSleepCtrl.ramRet = 3; */
/* 	crmSleepCtrl.mcuRet = 1; */
/* 	crmSleepCtrl.digPadRet = 1; */
/* 	crmSleepCtrl.pfToDoBeforeSleep = do_nothing; */

/* 	crm_gotosleep(&crmSleepCtrl); */

	/* wait for the sleep cycle to complete */
	while((reg32(CRM_STATUS) & 0x1) == 0) { continue; }
	/* write 1 to sleep_sync --- this clears the bit (it's a r1wc bit) and powers down */
	reg32(CRM_STATUS) = 1; 
	
	/* asleep */

	/* wait for the awake cycle to complete */
	while((reg32(CRM_STATUS) & 0x1) == 0) { continue; }
	/* write 1 to sleep_sync --- this clears the bit (it's a r1wc bit) and finishes wakeup */
	reg32(CRM_STATUS) = 1; 

	puts("\n\r\n\r\n\r");
	puts("0x00401ffc: ");
	put_hex32(reg32(0x00401ffc));
	puts("\r\n");
	puts("0x00407ffc: ");
	put_hex32(reg32(0x00407ffc));
	puts("\r\n");
	puts("0x0040fffc: ");
	put_hex32(reg32(0x0040fffc));
	puts("\r\n");
	puts("0x00410000: ");
	put_hex32(reg32(0x00410000));
	puts("\r\n");


	volatile uint32_t i;
	while(1) {

		reg32(GPIO_DATA0) = 0x00000100;
		
		for(i=0; i<DELAY; i++) { continue; }

		reg32(GPIO_DATA0) = 0x00000000;

		for(i=0; i<DELAY; i++) { continue; }

	};
}



void putc(uint8_t c) {
	while(reg32(UT1CON)==31); /* wait for there to be room in the buffer */
	reg32(UART1_DATA) = c;
}
	
void puts(uint8_t *s) {
	while(s && *s!=0) {
		putc(*s++);
	}
}

void put_hex(uint8_t x)
{
        putc(hex[x >> 4]);
        putc(hex[x & 15]);
}

void put_hex16(uint16_t x)
{
        put_hex((x >> 8) & 0xFF);
        put_hex((x) & 0xFF);
}

void put_hex32(uint32_t x)
{
        put_hex((x >> 24) & 0xFF);
        put_hex((x >> 16) & 0xFF);
        put_hex((x >> 8) & 0xFF);
        put_hex((x) & 0xFF);
}

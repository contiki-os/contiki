#define GPIO_FUNC_SEL0  0x80000018 /* GPIO 15 - 0;  2 bit blocks */

#define BASE_UART1      0x80005000
#define UART1_CON       0x80005000
#define UART1_STAT      0x80005004
#define UART1_DATA      0x80005008
#define UR1CON          0x8000500c
#define UT1CON          0x80005010
#define UART1_CTS       0x80005014
#define UART1_BR        0x80005018

#include "maca.h"
#include "embedded_types.h"

#define reg(x) (*(volatile uint32_t *)(x))

#define DELAY 400000
#define DATA  0x00401000;

#define NL "\033[K\r\n"

void putc(uint8_t c);
void puts(uint8_t *s);
void put_hex(uint8_t x);
void put_hex16(uint16_t x);
void put_hex32(uint32_t x);

const uint8_t hex[16]={'0','1','2','3','4','5','6','7',
		 '8','9','a','b','c','d','e','f'};

void magic(void) {
#define X     0x80009a000
#define Y     0x80009a008
#define VAL   0x0000f7df
	volatile uint32_t x,y;
	x = reg(X);      /* get X */
	x &= 0xfffeffff; /* clear bit 16 */
	reg(X) = x;      /* put it back  */
	y = reg(Y);      /* get Y */
	y |= VAL;        /* or with the VAL */
	x = reg(X);      /* get X again */
	x |= 16;         /* or with 16 */
	reg(X) = x;      /* put X back */
	reg(Y) = y;      /* put Y back */
}

uint32_t ackBox[10];

#define command_xcvr_rx() \
	do { \
		maca_txlen = (uint32_t)1<<16;	\
		maca_dmatx = (uint32_t)&ackBox; \
		maca_dmarx = DATA;		     \
		maca_tmren = (maca_cpl_clk | maca_soft_clk);	      \
		maca_control = (control_prm | control_asap | control_seq_rx); \
	}while(FALSE)


void dump_regs(uint32_t base, uint32_t len) {
	volatile uint32_t i;
    
	puts("base +0       +4       +8       +c       +10      +14      +18      +1c      \n\r");                                                     
	for (i = 0; i < len; i ++) {                                                                                                                   
		if ((i & 7) == 0) {                                                                                                                   
			put_hex16(4 * i);                                                                                                             
		}                                                                                                                                     
		puts(" ");                                                                                                                            
		put_hex32(reg(base+(4*i)));                                                                                                      
		if ((i & 7) == 7)                                                                                                                     
			puts(NL);                                                                                                                     
	}                                                                                                                                             
	puts(NL); 
}

__attribute__ ((section ("startup")))
void main(void) {
	uint8_t c;
	volatile uint32_t i;
	uint32_t tmp;
	volatile uint32_t *data;
	uint16_t status;

	/* Restore UART regs. to default */
	/* in case there is still bootloader state leftover */

	reg(UART1_CON) = 0x0000c800; /* mask interrupts, 16 bit sample --- helps explain the baud rate */

	/* INC = 767; MOD = 9999 works: 115200 @ 24 MHz 16 bit sample */
	#define INC 767
	#define MOD 9999
	reg(UART1_BR) = INC<<16 | MOD; 

	/* see Section 11.5.1.2 Alternate Modes */
	/* you must enable the peripheral first BEFORE setting the function in GPIO_FUNC_SEL */
	/* From the datasheet: "The peripheral function will control operation of the pad IF */
	/* THE PERIPHERAL IS ENABLED. */
	reg(UART1_CON) = 0x00000003; /* enable receive and transmit */
	reg(GPIO_FUNC_SEL0) = ( (0x01 << (14*2)) | (0x01 << (15*2)) ); /* set GPIO15-14 to UART (UART1 TX and RX)*/

	/* turn on the voltage regulators for the radio */
	/* you clod! */
	for(i=0; i<DELAY; i++) { continue; }
	reg(0x80003048) = 0x00000ff8;
	/* use the 24MHz clock for the modem */
	reg(0x80009000) = 0x80050100;

	reg(MACA_RESET) = 0x3; /* reset, turn on the clock */
	for(i=0; i<DELAY; i++) { continue; }

	reg(MACA_RESET) = 0x2; /* unreset, turn on the clock */
	for(i=0; i<DELAY; i++) { continue; }

	reset_maca();
	init_phy();

	/* some kind of sequence in init phy from MACPHY.a dissassmbly */
//	magic();

        reg(MACA_CONTROL) = SMAC_MACA_CNTL_INIT_STATE;
	for(i=0; i<DELAY; i++) { continue; }

	data = (void *)DATA;
	data[0] = 0xdeadbeef;
	reg(MACA_DMARX) = DATA; /* put data somewhere */
//	reg(MACA_PREAMBLE) = 0xface0fff;
	reg(MACA_PREAMBLE) = 0;

	puts("maca_base\n\r");
	dump_regs(MACA_BASE, 96);
	puts("modem write base\n\r");
	dump_regs(0x80009000, 96);
	puts("modem read base\n\r");
	dump_regs(0x800091c0, 96);
	puts("CRM\n\r");
	dump_regs(0x80003000, 96);
	puts("reserved modem_base\n\r");
	dump_regs(0x80009200, 192);

	while(1);

	command_xcvr_rx();

	puts("\033[H\033[2J");
	while(1) {		
		uint32_t TsmRxSteps, LastWarmupStep, LastWarmupData, LastWarmdownStep, LastWarmdownData;
		
		puts("\033[Hrftest-rx --- " );
		puts(" maca_getrxlvl: 0x");
		put_hex(reg(MACA_GETRXLVL));
		puts(" data[0]: 0x");
		put_hex32(data[0]);
		puts(" status: 0x");
		put_hex32(reg(MACA_STATUS));
		puts(" random: 0x");
		put_hex32(reg(MACA_RANDOM));
		puts(NL);

		puts("Maca_base");
		puts(NL);
		dump_regs(MACA_BASE,96);

		puts("0x80009000");
		puts(NL);
		dump_regs(0x80009000,192);
		
/* 		/\* start rx sequence *\/ */
/* 		reg(MACA_CONTROL) = 0x00031a01; /\* abort *\/ */
/* 		while (((tmp = reg(MACA_STATUS)) & 15) == 14) */
/* 			puts("."); */
/* 		puts("abort status is "); put_hex32(tmp); puts(NL); */
/* 		puts("1 status is "); put_hex32(reg(MACA_STATUS)); puts(NL); */
/* 		puts("2 status is "); put_hex32(reg(MACA_STATUS)); puts(NL); */
/* 		puts("3 status is "); put_hex32(reg(MACA_STATUS)); puts(NL); */


  /* read TSM_RX_STEPS */
		TsmRxSteps = (*((volatile uint32_t *)(0x80009204)));
		
		puts("TsmRxSteps: ");
		put_hex32(TsmRxSteps);
		puts(NL); 
		
		/* isolate the RX_WU_STEPS */
		/* shift left to align with 32-bit addressing */
		LastWarmupStep = (TsmRxSteps & 0x1f) << 2;
		/* Read "current" TSM step and save this value for later */
		LastWarmupData = (*((volatile uint32_t *)(0x80009300 + LastWarmupStep)));
		
		puts("LastWarmupData: ");
		put_hex32(LastWarmupData);
		puts(NL);
		
		/* isolate the RX_WD_STEPS */
		/* right-shift bits down to bit 0 position */
		/* left-shift to align with 32-bit addressing */
		LastWarmdownStep = ((TsmRxSteps & 0x1f00) >> 8) << 2;
		/* write "last warmdown data" to current TSM step to shutdown rx */
		LastWarmdownData = (*((volatile uint32_t *)(0x80009300 + LastWarmdownStep)));
		
		puts("LastWarmdownData: ");
		put_hex32(LastWarmdownData);
		puts(NL);
		
		
		status = reg(MACA_STATUS) & 0x0000ffff;
		switch(status) 
		{
		case(cc_aborted):
		{
			puts("aborted\n\r");
			ResumeMACASync();
			break;
			
		}
		case(cc_not_completed):
		{
			puts("not completed\n\r");
			ResumeMACASync();
			break;
			
		}
		case(cc_success):
		{
			puts("success\n\r");
			break;
			
		}
		default:
		{
			puts("status: ");
			put_hex16(status);
		}
		}
		
/* 		reg(MACA_CONTROL) = 0x00031a04; /\* receive *\/ */
/* 		while (((tmp = reg(MACA_STATUS)) & 15) == 14) */
/* 			puts("."); */
/* 		puts("complete status is "); put_hex32(tmp); puts(NL); */
/* 		puts("1 status is "); put_hex32(reg(MACA_STATUS)); puts(NL); */
/* 		puts("2 status is "); put_hex32(reg(MACA_STATUS)); puts(NL); */
/* 		puts("3 status is "); put_hex32(reg(MACA_STATUS)); puts(NL); */

/* 		puts(NL);		 */
/* 		for(i=0; i<DELAY; i++) { continue; } */
/* 		for(i=0; i<DELAY; i++) { continue; } */
/* 		for(i=0; i<DELAY; i++) { continue; } */
/* 		for(i=0; i<DELAY; i++) { continue; } */
/* 		for(i=0; i<DELAY; i++) { continue; } */

		
	};
}

void putc(uint8_t c) {
	while(reg(UT1CON)==31); /* wait for there to be room in the buffer */
	reg(UART1_DATA) = c;
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

#include <mc1322x.h>
#include <stdio.h>

#include "put.h"
#include "tests.h"

void uart1_init(uint16_t inc, uint16_t mod) {
	/* Restore UART regs. to default */
	/* in case there is still bootloader state leftover */
	
	*UART1_CON = 0x0000c800; /* mask interrupts, 16 bit sample --- helps explain the baud rate */
	
	/* INC = 767; MOD = 9999 works: 115200 @ 24 MHz 16 bit sample */
	*UART1_BR = (inc << 16) | mod; 
	
	/* see Section 11.5.1.2 Alternate Modes */
	/* you must enable the peripheral first BEFORE setting the function in GPIO_FUNC_SEL */
	/* From the datasheet: "The peripheral function will control operation of the pad IF */
	/* THE PERIPHERAL IS ENABLED. */
	*UART1_CON = 0x00000003; /* enable receive and transmit */
	*GPIO_FUNC_SEL0 = ( (0x01 << (14*2)) | (0x01 << (15*2)) ); /* set GPIO15-14 to UART (UART1 TX and RX)*/
	
}

void print_welcome(char* testname) {
	printf("mc1322x-test: %s\n\r",testname);
	printf("board: %s\n\r", STR2(BOARD));
}

void print_packet(packet_t *p) { 
	volatile uint8_t i,j,k; 
#define PER_ROW 16
	if(p) {
		printf("len 0x%02x\n\r",p->length);		
		for(j=0, k=0; j < ((p->length)%PER_ROW)-1; j++) {
			for(i=0; i < PER_ROW; i++, k++) {
				if(k>=p->length) { break; }
				printf("%02x ",p->data[j*PER_ROW+i]);
			}
			printf("\n\r");
		}
		printf("\n\r");
	}
	return; 
}

void dump_regs(uint32_t base, uint32_t len) {
	volatile uint32_t i;
	
	printf("base +0       +4       +8       +c       +10      +14      +18      +1c      \n\r");
	for (i = 0; i < len; i ++) {
		if ((i & 7) == 0) {
			printf("%02x",4 * i);
		}
		printf(" %08x",*mem32(base+(4*i)));
		if ((i & 7) == 7)
			printf(NL);
	}
	printf(NL); 
}

#include <mc1322x.h>
#include <stdio.h>

#include "put.h"
#include "tests.h"

void print_welcome(char* testname) {
	printf("mc1322x-test: %s\n\r",testname);
	printf("board: %s\n\r", STR2(BOARD));
}

void print_packet(volatile packet_t *p) { 
	volatile uint8_t i,j,k; 
#define PER_ROW 16
	if(p) {
		printf("len 0x%02x",p->length);		
		for(j=0, k=0; j <= ((p->length)%PER_ROW); j++) {
			printf("\n\r");
			for(i=0; i < PER_ROW; i++, k++) {
				if(k >= p->length ) { 
					printf("\n\r");
					return; 
				} 
				printf("%02x ",p->data[j*PER_ROW + i + p->offset]);
			}
		}
	}
	printf("\n\r");
	return; 
}

void dump_regs(uint32_t base, uint32_t len) {
	volatile uint32_t i;
	
	printf("base +0       +4       +8       +c       +10      +14      +18      +1c      \n\r");
	for (i = 0; i < len; i ++) {
		if ((i & 7) == 0) {
			printf("%02lx",(uint32_t)(4 * i));
		}
		printf(" %08lx",(uint32_t)*mem32(base+(4*i)));
		if ((i & 7) == 7)
			printf(NL);
	}
	printf(NL); 
}

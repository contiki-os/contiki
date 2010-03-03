#include <mc1322x.h>
#include <board.h>
#include <stdio.h>

#include "tests.h"
#include "config.h"

/* This program communicates with itself and determines the packet */
/* error rate (PER) under a variety of powers and packet sizes  */
/* Each test the packets are sent and received as fast as possible */

/* The program first scans on channel 11 and attempts to open a test */
/* session with a node. After opening a session, the nodes begin the */
/* test sequence  */

#define DEBUG_MACA 1

typedef struct {
	int type;
} packet_t;

enum STATES {
	SCANNING,
	MAX_STATE
};

enum PACKET_TYPE {
	PACKET_PERTEST,
	MAX_PACKET_TYPE
};

int get_packet(packet_t *p) { return 0; }
void print_packet(packet_t p) { return; }

void main(void) {
	uint32_t state;
	packet_t p;

	uart_init(INC,MOD);
	
	print_welcome("Packet error test");

	/* standard radio initialization */
	reset_maca();
	radio_init();
	vreg_init();
	flyback_init();
	init_phy();

	/* trim the reference osc. to 24MHz */
	pack_XTAL_CNTL(CTUNE_4PF, CTUNE, FTUNE, IBIAS);

	set_power(0x0f); /* 0dbm */
	set_channel(0); /* channel 11 */

	/* initial radio command */
        /* nop, promiscuous, no cca */
	*MACA_CONTROL = (1 << PRM) | (NO_CCA << MODE); 

	state = SCANNING;
	while(1) { 

		switch(state) {
		case SCANNING:
			if(get_packet(&p)) {
				print_packet(p);
				/* if we have a packet */
				/* check if it's a pertest beacon */
				if(p.type == PACKET_PERTEST) {
					/* try to start a session */
				}
			}
			break;
		default:
			break;
		}
					

	}

}


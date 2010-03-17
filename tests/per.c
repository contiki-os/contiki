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

/* how long to wait between session requests */
#define SESSION_REQ_TIMEOUT 10000     /* phony seconds */

enum STATES {
	SCANNING,
	MAX_STATE
};

typedef uint32_t ptype_t;
enum PACKET_TYPE {
	PACKET_SESS_REQ,
	MAX_PACKET_TYPE
};
/* get protocol level packet type   */
/* this is not 802.15.4 packet type */
ptype_t get_packet_type(packet_t * p __attribute__((unused))) {
	return MAX_PACKET_TYPE;
}

typedef uint32_t session_id_t;



/* phony get_time */
uint32_t get_time(void) {
	static volatile int32_t cur_time = 0;
	cur_time++;
	return cur_time;
}


#define random_short_addr() (*MACA_RANDOM & ones(sizeof(uint16_t)*8))

void build_session_req(volatile packet_t *p) {
	static uint8_t count = 0;
	p->length = 4; p->offset = 0;
	p->data[0] = 0xff;
	p->data[1] = 0x01;
	p->data[2] = 0x02;
	p->data[3] = count++;
	return;
}

void session_req(uint16_t addr __attribute__((unused))) { 	
	static volatile int time = 0;
	volatile packet_t *p;

	if((get_time() - time) > SESSION_REQ_TIMEOUT) {
		time = get_time();
		if((p = get_free_packet())) {
			build_session_req(p);
			tx_packet(p);
		}
	}
	return; 
}

session_id_t open_session(uint16_t addr __attribute((unused))) { return 0; }

void main(void) {
	uint32_t state;
	volatile packet_t *p;
	session_id_t sesid;
	ptype_t type;
	uint16_t addr, my_addr;

	/* trim the reference osc. to 24MHz */
	pack_XTAL_CNTL(CTUNE_4PF, CTUNE, FTUNE, IBIAS);
	
	uart_init(INC, MOD, SAMP);

	vreg_init();

	maca_init();
	
	set_power(0x0f); /* 0dbm */
	set_channel(0); /* channel 11 */

	/* generate a random short address */
	my_addr = random_short_addr();

        /* sets up tx_on, should be a board specific item */
        *GPIO_FUNC_SEL2 = (0x01 << ((44-16*2)*2));
	gpio_pad_dir_set( 1ULL << 44 );

	print_welcome("Packet error test");

	state = SCANNING;
	while(1) { 
		
		switch(state) {
		case SCANNING:
			if((p = rx_packet())) {
				/* extract what we need and free the packet */
				printf("Recv: ");
				print_packet(p);
				type = get_packet_type((packet_t *) p);
				addr = 0; /* FIXME */
				free_packet(p);
				/* pick a new address if someone else is using ours */
				if(addr == my_addr) {
					my_addr = random_short_addr();
					printf("DUP addr received, changing to new addr 0x%x02\n\r",my_addr);
				}
				/* if we have a packet */
				/* check if it's a session request beacon */
				if(type == PACKET_SESS_REQ) {
					/* try to start a session */
					sesid = open_session(addr);
				}
			}  else {
				session_req(my_addr);
			}
			break;
		default:
			break;
		}
					

	}

}


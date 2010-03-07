#ifndef PACKET_H
#define PACKET_H

#ifndef MAX_PACKET_SIZE
#define MAX_PACKET_SIZE 127
#endif

typedef uint16_t short_addr_t;

struct packet {
	short_addr_t addr;
        uint8_t length;
	volatile struct packet * left;
	volatile struct packet * right;
        /* offset into data for first byte of the packet payload */
	/* On TX this should be 0 */
	/* On RX this should be 1 since the maca puts the length as the first byte*/
	uint8_t offset; 	
	uint8_t data[MAX_PACKET_SIZE+1]; /* + 1 since maca returns the length as the first byte */
};
typedef struct packet packet_t;

#endif

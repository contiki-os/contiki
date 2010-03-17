#ifndef PACKET_H
#define PACKET_H

/* does not include 2 byte FCS checksum */
#ifndef MAX_PAYLOAD_SIZE
#define MAX_PAYLOAD_SIZE 125
#endif

struct packet {
        uint8_t length; /* does not include FCS checksum */
	volatile struct packet * left;
	volatile struct packet * right;
        /* offset into data for first byte of the packet payload */
	/* On TX this should be 0 */
	/* On RX this should be 1 since the maca puts the length as the first byte*/
	uint8_t offset; 	
	uint8_t data[MAX_PAYLOAD_SIZE+2+1]; /* +2 for FCS; + 1 since maca returns the length as the first byte */
};
typedef struct packet packet_t;

#endif

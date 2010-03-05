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
	uint8_t data[MAX_PACKET_SIZE];
};
typedef struct packet packet_t;

#endif

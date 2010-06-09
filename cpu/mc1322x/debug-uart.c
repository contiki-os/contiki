#include "contiki-conf.h"

unsigned int
dbg_send_bytes(const unsigned char *s, unsigned int len)
{
	unsigned int i=0;
	while(s && *s!=0) {
		if( i >= len) { break; }
		dbg_putchar(*s++); i++;
	}
	return i;
}

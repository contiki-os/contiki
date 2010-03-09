#include <mc1322x.h>
#include <board.h>

const uint8_t hex[16]={'0','1','2','3','4','5','6','7',
		 '8','9','a','b','c','d','e','f'};

void putchr(char c) {
	while(*UART1_UTXCON == 31); 
        /* wait for there to be room in the buffer */
// 	while( *UART1_UTXCON == 0 ) { continue; }
	*UART1_UDATA = c;
}
	
void putstr(char *s) {
	while(s && *s!=0) {
		putchr(*s++);
	}
}

void put_hex(uint8_t x)
{
        putchr(hex[x >> 4]);
        putchr(hex[x & 15]);
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

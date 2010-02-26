#include <mc1322x.h>
#include <board.h>
#include <uart1.h>
#include <nvm.h>

/* INC = 767; MOD = 9999 works: 115200 @ 24 MHz 16 bit sample */
#define INC 767
#define MOD 9999

#define NBYTES 8
#define WRITE_ADDR 0x1e000
#define WRITEVAL0 0xdeadbeef 
#define WRITEVAL1 0xdeadbeef

void putc(char c);
void puts(char *s);
void put_hex(uint8_t x);
void put_hex16(uint16_t x);
void put_hex32(uint32_t x);

const uint8_t hex[16]={'0','1','2','3','4','5','6','7',
		 '8','9','a','b','c','d','e','f'};

void main(void) {
	nvmType_t type=0;
	nvmErr_t err;
	uint32_t buf[NBYTES/4];
	uint32_t i;

	*UART1_CON = 0x0000c800; /* mask interrupts, 16 bit sample --- helps explain the baud rate */

	/* INC = 767; MOD = 9999 works: 115200 @ 24 MHz 16 bit sample */
	#define INC 767
	#define MOD 9999
	*UART1_BR = INC<<16 | MOD; 

	/* see Section 11.5.1.2 Alternate Modes */
	/* you must enable the peripheral first BEFORE setting the function in GPIO_FUNC_SEL */
	/* From the datasheet: "The peripheral function will control operation of the pad IF */
	/* THE PERIPHERAL IS ENABLED. */
	*UART1_CON = 0x00000003; /* enable receive and transmit */
	*GPIO_FUNC_SEL0 = ( (0x01 << (14*2)) | (0x01 << (15*2)) ); /* set GPIO15-14 to UART (UART1 TX and RX)*/

	vreg_init();

	puts("Detecting internal nvm\n\r");

	err = nvm_detect(gNvmInternalInterface_c, &type);
		
	puts("nvm_detect returned: 0x");
	put_hex(err);
	puts(" type is: 0x");
	put_hex32(type);
	puts("\n\r");


	buf[0] = WRITEVAL0;
	buf[1] = WRITEVAL1;

	err = nvm_erase(gNvmInternalInterface_c, type, 0x40000000); /* erase sector 30 --- sector 31 is the 'secret zone' */
	puts("nvm_erase returned: 0x");
	put_hex(err);
	puts("\n\r");

	err = nvm_write(gNvmInternalInterface_c, type, (uint8_t *)buf, WRITE_ADDR, NBYTES);
	puts("nvm_write returned: 0x");
	put_hex(err);
	puts("\n\r");
	puts("writing\n\r");
	for(i=0; i<NBYTES/4; i++) {
		puts("0x");
		put_hex32(buf[i]);
		puts("\n\r");
		buf[i] = 0x00000000; /* clear buf for the read */
	}

	err = nvm_read(gNvmInternalInterface_c, type, (uint8_t *)buf, WRITE_ADDR, NBYTES);
	puts("nvm_read returned: 0x");
	put_hex(err);
	puts("\n\r");
	puts("reading\n\r");
	for(i=0; i<NBYTES/4; i++) {
		puts("0x");
		put_hex32(buf[i]);
		puts("\n\r");
	}
		

	while(1) {continue;};
}

void putc(char c) {
	while(*UT1CON == 31); /* wait for there to be room in the buffer */
	*UART1_DATA = c;
}
	
void puts(char *s) {
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

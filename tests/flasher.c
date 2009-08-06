#define GPIO_FUNC_SEL0  0x80000018 /* GPIO 15 - 0;  2 bit blocks */

#define BASE_UART1      0x80005000
#define UART1_CON       0x80005000
#define UART1_STAT      0x80005004
#define UART1_DATA      0x80005008
#define UR1CON          0x8000500c
#define UT1CON          0x80005010
#define UART1_CTS       0x80005014
#define UART1_BR        0x80005018

#define GPIO_PAD_DIR0   0x80000000
#define GPIO_DATA0      0x80000008

#include "embedded_types.h"
#include "nvm.h"
#include "maca.h"

#define reg(x) (*(volatile uint32_t *)(x))

#define DELAY 400000

/* if both BOOT_OK and BOOT_SECURE are 0 then flash image will not be bootable */
/* if both are 1 then flash image will be secure */
#define BOOT_OK 1
#define BOOT_SECURE 0

#define DEBUG 1
#if DEBUG
#define dbg_putc(...) putc(__VA_ARGS__)
#define dbg_puts(...) puts(__VA_ARGS__)
#define dbg_put_hex(...) put_hex(__VA_ARGS__)
#define dbg_put_hex16(...) put_hex16(__VA_ARGS__)
#define dbg_put_hex32(...) put_hex32(__VA_ARGS__)
#else
#define dbg_putc(...)
#define dbg_puts(...)
#define dbg_put_hex(...)
#define dbg_put_hex16(...)
#define dbg_put_hex32(...)
#endif

const uint8_t hex[16]={'0','1','2','3','4','5','6','7',
		 '8','9','a','b','c','d','e','f'};

uint8_t getc(void);
void flushrx(void);
uint32_t to_u32(char *c);

#include "isr.h"

#define NBYTES 16

enum parse_states {
	SCAN_X,
	READ_CHARS,
	PROCESS,
	MAX_STATE,
};

__attribute__ ((section ("startup")))
void main(void) {
	nvmType_t type=0;
	nvmErr_t err;
	volatile uint8_t c;
	volatile uint32_t buf[NBYTES/4];
	volatile uint32_t i;
	volatile uint32_t len=0;
	volatile uint32_t state = SCAN_X;
	volatile uint32_t addr,data;

	*(volatile uint32_t *)GPIO_PAD_DIR0 = 0x00000100;
	
	/* Restore UART regs. to default */
	/* in case there is still bootloader state leftover */

	reg(UART1_CON) = 0x0000c800; /* mask interrupts, 16 bit sample --- helps explain the baud rate */

	/* INC = 767; MOD = 9999 works: 115200 @ 24 MHz 16 bit sample */
	#define INC 767
	#define MOD 9999
	reg(UART1_BR) = INC<<16 | MOD; 

	/* see Section 11.5.1.2 Alternate Modes */
	/* you must enable the peripheral first BEFORE setting the function in GPIO_FUNC_SEL */
	/* From the datasheet: "The peripheral function will control operation of the pad IF */
	/* THE PERIPHERAL IS ENABLED. */
	reg(UART1_CON) = 0x00000003; /* enable receive and transmit */
	reg(GPIO_FUNC_SEL0) = ( (0x01 << (14*2)) | (0x01 << (15*2)) ); /* set GPIO15-14 to UART (UART1 TX and RX)*/

	vreg_init();

	dbg_puts("Detecting internal nvm\n\r");

	err = nvm_detect(gNvmInternalInterface_c, &type);
		
	dbg_puts("nvm_detect returned: 0x");
	dbg_put_hex(err);
	dbg_puts(" type is: 0x");
	dbg_put_hex32(type);
	dbg_puts("\n\r");
	
	/* erase the flash */
	err = nvm_erase(gNvmInternalInterface_c, type, 0x4fffffff); 

	dbg_puts("nvm_erase returned: 0x");
	dbg_put_hex(err);
	dbg_puts("\n\r");

	dbg_puts(" type is: 0x");
	dbg_put_hex32(type);
	dbg_puts("\n\r");

	/* say we are ready */
	len = 0;
	puts("ready");
	flushrx();

	/* read the length */
	for(i=0; i<4; i++) {
		c = getc();
		/* bail if the first byte of the length is zero */
		len += (c<<(i*8));
	}

	dbg_puts("len: ");
	dbg_put_hex32(len);
	dbg_puts("\n\r");
	
	/* write the OKOK magic */

#if BOOT_OK
	((uint8_t *)buf)[0] = 'O'; ((uint8_t *)buf)[1] = 'K'; ((uint8_t *)buf)[2] = 'O'; ((uint8_t *)buf)[3] = 'K';	
#elif BOOT_SECURE
	((uint8_t *)buf)[0] = 'S'; ((uint8_t *)buf)[1] = 'E'; ((uint8_t *)buf)[2] = 'C'; ((uint8_t *)buf)[3] = 'U';	
#else
	((uint8_t *)buf)[0] = 'N'; ((uint8_t *)buf)[1] = 'O'; ((uint8_t *)buf)[2] = 'N'; ((uint8_t *)buf)[3] = 'O';
#endif

	dbg_puts(" type is: 0x");
	dbg_put_hex32(type);
	dbg_puts("\n\r");

	err = nvm_write(gNvmInternalInterface_c, type, (uint8_t *)buf, 0, 4);

	dbg_puts("nvm_write returned: 0x");
	dbg_put_hex(err);
	dbg_puts("\n\r");

	/* write the length */
	err = nvm_write(gNvmInternalInterface_c, type, (uint8_t *)&len, 4, 4);

	/* read a byte, write a byte */
	/* byte at a time will make this work as a contiki process better */
	/* for OTAP */
	for(i=0; i<len; i++) {
		c = getc();	       
		err = nvm_write(gNvmInternalInterface_c, type, &c, 8+i, 1); 
	}

	puts("flasher done\n\r");

	state = SCAN_X; addr=0;
	while((c=getc())) {
		if(state == SCAN_X) {
			/* read until we see an 'x' */
			if(c==0) { break; }
			if(c!='x'){ continue; } 	
			/* go to read_chars once we have an 'x' */
			state = READ_CHARS;
			i = 0; 
		}
		if(state == READ_CHARS) {
			/* read all the chars up to a ',' */
			((uint8_t *)buf)[i++] = c;
			/* after reading a ',' */
			/* goto PROCESS state */
			if((c == ',') || (c == 0)) { state = PROCESS; }				
		}
		if(state == PROCESS) {
			if(addr==0) {
				/*interpret the string as the starting address */
				addr = to_u32((uint8_t *)buf);				
			} else {
				/* string is data to write */
				data = to_u32((uint8_t *)buf);
				puts("writing addr ");
				put_hex32(addr);
				puts(" data ");
				put_hex32(data);
				puts("\n\r");
				err = nvm_write(gNvmInternalInterface_c, 1, (uint8_t *)&data, addr, 4);
				addr += 4;
			}
			/* look for the next 'x' */
			state=SCAN_X;
		}
	}

	while(1) {continue;};
}

void flushrx(void)
{
	volatile uint8_t c;
	while(reg(UR1CON) !=0) {
		c = reg(UART1_DATA);
	}
}

/* Convert from ASCII hex.  Returns                                                                                                      
   the value, or 16 if it was space/newline, or                                                                                          
   32 if some other character. */
uint8_t from_hex(uint8_t ch)
{
        if(ch==' ' || ch=='\r' || ch=='\n')
                return 16;

        if(ch < '0')
                goto bad;
        if(ch <= '9')
                return ch - '0';
        ch |= 0x20;
        if(ch < 'a')
                goto bad;
        if(ch <= 'f')
                return ch - 'a' + 10;
bad:
        return 32;
}

uint32_t to_u32(char *c) 
{
	volatile uint32_t ret=0;
	volatile uint32_t i,val;
	
	/* c should be /x\d+,/ */
	i=1; /* skip x */
	while(c[i] != ',') {
		ret = ret<<4;
		val = from_hex(c[i++]);
		ret += val;
	}
	return ret;
}

uint8_t getc(void) 
{
	volatile uint8_t c;
	while(reg(UR1CON) == 0);
	
	c = reg(UART1_DATA);
	return c;
}


void putc(uint8_t c) {
	while(reg(UT1CON)==31); /* wait for there to be room in the buffer */
	reg(UART1_DATA) = c;
}
	
void puts(uint8_t *s) {
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


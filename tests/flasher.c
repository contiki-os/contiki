#include <mc1322x.h>
#include <board.h>

#include "tests.h"
#include "config.h"

#define DEBUG 1
#if DEBUG
#define dbg_putchr(...) putchr(__VA_ARGS__)
#define dbg_putstr(...) putstr(__VA_ARGS__)
#define dbg_put_hex(...) put_hex(__VA_ARGS__)
#define dbg_put_hex16(...) put_hex16(__VA_ARGS__)
#define dbg_put_hex32(...) put_hex32(__VA_ARGS__)
#else
#define dbg_putchr(...)
#define dbg_putstr(...)
#define dbg_put_hex(...)
#define dbg_put_hex16(...)
#define dbg_put_hex32(...)
#endif

uint8_t getc(void) 
{
	volatile uint8_t c;
	while(*UART1_URXCON == 0);
	
	c = *UART1_UDATA;
	return c;
}


void flushrx(void);
uint32_t to_u32(volatile uint32_t *c);

enum parse_states {
	SCAN_X,
	READ_CHARS,
	PROCESS,
	MAX_STATE,
};

void main(void) {
	nvmType_t type=0;
	nvmErr_t err;
	volatile uint8_t c;
	volatile uint32_t i;
	volatile uint32_t buf[4];
	volatile uint32_t len=0;
	volatile uint32_t state = SCAN_X;
	volatile uint32_t addr,data;


	uart_init(INC, MOD, SAMP);
	disable_irq(UART1);

	vreg_init();

	dbg_putstr("Detecting internal nvm\n\r");

	err = nvm_detect(gNvmInternalInterface_c, &type);
		
	dbg_putstr("nvm_detect returned: 0x");
	dbg_put_hex(err);
	dbg_putstr(" type is: 0x");
	dbg_put_hex32(type);
	dbg_putstr("\n\r");
	
	/* erase the flash */
	err = nvm_erase(gNvmInternalInterface_c, type, 0x4fffffff); 

	dbg_putstr("nvm_erase returned: 0x");
	dbg_put_hex(err);
	dbg_putstr("\n\r");

	dbg_putstr(" type is: 0x");
	dbg_put_hex32(type);
	dbg_putstr("\n\r");

	/* say we are ready */
	len = 0;
	putstr("ready");
	flushrx();

	/* read the length */
	for(i=0; i<4; i++) {
		c = uart1_getc();
		/* bail if the first byte of the length is zero */
		len += (c<<(i*8));
	}

	dbg_putstr("len: ");
	dbg_put_hex32(len);
	dbg_putstr("\n\r");
	
	/* write the OKOK magic */

#if BOOT_OK
	((uint8_t *)buf)[0] = 'O'; ((uint8_t *)buf)[1] = 'K'; ((uint8_t *)buf)[2] = 'O'; ((uint8_t *)buf)[3] = 'K';	
#elif BOOT_SECURE
	((uint8_t *)buf)[0] = 'S'; ((uint8_t *)buf)[1] = 'E'; ((uint8_t *)buf)[2] = 'C'; ((uint8_t *)buf)[3] = 'U';	
#else
	((uint8_t *)buf)[0] = 'N'; ((uint8_t *)buf)[1] = 'O'; ((uint8_t *)buf)[2] = 'N'; ((uint8_t *)buf)[3] = 'O';
#endif

	dbg_putstr(" type is: 0x");
	dbg_put_hex32(type);
	dbg_putstr("\n\r");

	err = nvm_write(gNvmInternalInterface_c, type, (uint8_t *)buf, 0, 4);

	dbg_putstr("nvm_write returned: 0x");
	dbg_put_hex(err);
	dbg_putstr("\n\r");

	/* write the length */
	err = nvm_write(gNvmInternalInterface_c, type, (uint8_t *)&len, 4, 4);

	/* read a byte, write a byte */
	/* byte at a time will make this work as a contiki process better */
	/* for OTAP */
	for(i=0; i<len; i++) {
		c = getc();	       
		err = nvm_write(gNvmInternalInterface_c, type, (uint8_t *)&c, 8+i, 1); 
	}

	putstr("flasher done\n\r");

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
				addr = to_u32(buf);				
			} else {
				/* string is data to write */
				data = to_u32(buf);
				putstr("writing addr ");
				put_hex32(addr);
				putstr(" data ");
				put_hex32(data);
				putstr("\n\r");
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
	while(*UART1_URXCON !=0) {
		c = *UART1_UDATA;
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

uint32_t to_u32(volatile uint32_t *c) 
{
	volatile uint32_t ret=0;
	volatile uint32_t i,val;
	
	/* c should be /x\d+,/ */
	i=1; /* skip x */
	while(((uint8_t *)c)[i] != ',') {
		ret = ret<<4;
		val = from_hex(((uint8_t *)c)[i++]);
		ret += val;
	}
	return ret;
}




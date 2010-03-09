#include <mc1322x.h>
#include <board.h>

#include "tests.h"
#include "config.h"

void main(void) {
	nvmType_t type=0;
	nvmErr_t err;
	uint32_t buf[WRITE_NBYTES/4];
	uint32_t i;

	uart_init(INC, MOD, SAMP);

	print_welcome("nvm-write");

	vreg_init();

	putstr("Detecting internal nvm\n\r");

	err = nvm_detect(gNvmInternalInterface_c, &type);
		
	putstr("nvm_detect returned: 0x");
	put_hex(err);
	putstr(" type is: 0x");
	put_hex32(type);
	putstr("\n\r");


	buf[0] = WRITEVAL0;
	buf[1] = WRITEVAL1;

	err = nvm_erase(gNvmInternalInterface_c, type, 0x40000000); /* erase sector 30 --- sector 31 is the 'secret zone' */
	putstr("nvm_erase returned: 0x");
	put_hex(err);
	putstr("\n\r");

	err = nvm_write(gNvmInternalInterface_c, type, (uint8_t *)buf, WRITE_ADDR, WRITE_NBYTES);
	putstr("nvm_write returned: 0x");
	put_hex(err);
	putstr("\n\r");
	putstr("writing\n\r");
	for(i=0; i<WRITE_NBYTES/4; i++) {
		putstr("0x");
		put_hex32(buf[i]);
		putstr("\n\r");
		buf[i] = 0x00000000; /* clear buf for the read */
	}

	err = nvm_read(gNvmInternalInterface_c, type, (uint8_t *)buf, WRITE_ADDR, WRITE_NBYTES);
	putstr("nvm_read returned: 0x");
	put_hex(err);
	putstr("\n\r");
	putstr("reading\n\r");
	for(i=0; i<WRITE_NBYTES/4; i++) {
		putstr("0x");
		put_hex32(buf[i]);
		putstr("\n\r");
	}
		

	while(1) {continue;};
}


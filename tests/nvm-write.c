#include <mc1322x.h>
#include <board.h>

#include "tests.h"
#include "config.h"

void main(void) {
	nvmType_t type=0;
	nvmErr_t err;
	uint32_t buf[WRITE_NBYTES/4];
	uint32_t i;

	uart_init(INC, MOD);

	print_welcome("nvm-write");

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

	err = nvm_write(gNvmInternalInterface_c, type, (uint8_t *)buf, WRITE_ADDR, WRITE_NBYTES);
	puts("nvm_write returned: 0x");
	put_hex(err);
	puts("\n\r");
	puts("writing\n\r");
	for(i=0; i<WRITE_NBYTES/4; i++) {
		puts("0x");
		put_hex32(buf[i]);
		puts("\n\r");
		buf[i] = 0x00000000; /* clear buf for the read */
	}

	err = nvm_read(gNvmInternalInterface_c, type, (uint8_t *)buf, WRITE_ADDR, WRITE_NBYTES);
	puts("nvm_read returned: 0x");
	put_hex(err);
	puts("\n\r");
	puts("reading\n\r");
	for(i=0; i<WRITE_NBYTES/4; i++) {
		puts("0x");
		put_hex32(buf[i]);
		puts("\n\r");
	}
		

	while(1) {continue;};
}


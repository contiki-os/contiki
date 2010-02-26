#include <mc1322x.h>
#include <board.h>

#include "tests.h"
#include "config.h"

void main(void) {
	nvmType_t type=0;
	nvmErr_t err;
	uint32_t buf[READ_NBYTES/4];
	uint32_t i;

	uart_init(INC, MOD);

	print_welcome("nvm-read");

	vreg_init();

	puts("Detecting internal nvm\n\r");

	err = nvm_detect(gNvmInternalInterface_c, &type);
		
	puts("nvm_detect returned: 0x");
	put_hex(err);
	puts(" type is: 0x");
	put_hex32(type);
	puts("\n\r");

	nvm_setsvar(0);

	err = nvm_read(gNvmInternalInterface_c, type, (uint8_t *)buf, READ_ADDR, READ_NBYTES);
	puts("nvm_read returned: 0x");
	put_hex(err);
	puts("\n\r");

	for(i=0; i<READ_NBYTES/4; i++) {
		puts("0x");
		put_hex32(buf[i]);
		puts("\n\r");
	}
		

	while(1) {continue;};
}


#include <mc1322x.h>
#include <board.h>

#define DELAY 400000

void main(void) {
	
	*GPIO_FUNC_SEL0 = 0xffffffff;
	*GPIO_FUNC_SEL1 = 0xffffffff;
	*GPIO_FUNC_SEL2 = 0xffffffff;
	*GPIO_FUNC_SEL3 = 0xffffffff;

	*GPIO_PAD_DIR0 = 0xffffffff;
	*GPIO_PAD_DIR1 = 0xffffffff;

	volatile uint32_t i;

	while(1) {

		*GPIO_DATA0 = 0xffffffff;
		*GPIO_DATA1 = 0xffffffff;
		
		for(i=0; i<DELAY; i++) { continue; }

		*GPIO_DATA0 = 0x00000000;
		*GPIO_DATA1 = 0x00000000;

		for(i=0; i<DELAY; i++) { continue; }

	};
}

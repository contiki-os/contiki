#include <mc1322x.h>
#include <board.h>
#include "led.h"

#define DELAY 400000

#define LED LED_WHITE

void main(void) {	
	volatile uint32_t i;
	
	*GPIO_PAD_DIR0 = LED;

	while(1) {

		*GPIO_DATA0 = LED;
		
		for(i=0; i<DELAY; i++) { continue; }

		*GPIO_DATA0 = 0x00000000;

		for(i=0; i<DELAY; i++) { continue; }

	};

}


#include <mc1322x.h>
#include <board.h>

#include "led.h"

#define DELAY 400000

#define LED LED_WHITE

void main(void) {	
	volatile uint32_t i;
	
	gpio_pad_dir(LED);

	while(1) {

		gpio_data(LED);
		
		for(i=0; i<DELAY; i++) { continue; }

		gpio_data(0);
		
		for(i=0; i<DELAY; i++) { continue; }

	};

}


#include "types.h"
#include "isr.h"
#include "led.h"

#define MBAR_GPIO       0x80000000
#define GPIO_PAD_DIR0   ((volatile uint32_t *) 0x80000000)
#define GPIO_DATA0      ((volatile uint32_t *) 0x80000008)
#define UART1_DATA      ((volatile uint32_t *) 0x80005008)
#define DELAY 400000

#define LED_BITS LED_RED

__attribute__ ((section ("startup"))) 
void main(void) {	
	volatile uint32_t i;
	
	*GPIO_PAD_DIR0 = LED_BITS;


	while(1) {

		*GPIO_DATA0 = LED_BITS;
		
		for(i=0; i<DELAY; i++) { continue; }

		*GPIO_DATA0 = 0x00000000;

		for(i=0; i<DELAY; i++) { continue; }

	};

}


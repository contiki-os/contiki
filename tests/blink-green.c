#define MBAR_GPIO       0x80000000
#define GPIO_PAD_DIR0   0x80000000
#define GPIO_DATA0      0x80000008
#define UART1_DATA      0x80005008
#define DELAY 400000

#include "embedded_types.h"
#include "isr.h"
#include "led.h"

#define LED_BITS LED_GREEN

__attribute__ ((section ("startup")))
void main(void) {
	
	*(volatile uint32_t *)GPIO_PAD_DIR0 = LED_BITS;

	volatile uint32_t i;

	while(1) {

		*(volatile uint32_t *)GPIO_DATA0 = LED_BITS;
		
		for(i=0; i<DELAY; i++) { continue; }

		*(volatile uint32_t *)GPIO_DATA0 = 0x00000000;

		for(i=0; i<DELAY; i++) { continue; }

	};
}

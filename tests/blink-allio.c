#define MBAR_GPIO       0x80000000
#define GPIO_FUNC_SEL0  0x80000018 /* GPIO 15 - 0;  2 bit blocks */
#define GPIO_FUNC_SEL1  0x8000001c 
#define GPIO_FUNC_SEL2  0x80000020 
#define GPIO_FUNC_SEL3  0x80000024 
#define GPIO_PAD_DIR0   0x80000000
#define GPIO_DATA0      0x80000008
#define GPIO_PAD_DIR1   0x80000004
#define GPIO_DATA1      0x8000000c
#define UART1_DATA      0x80005008
#define DELAY 400000

#include "embedded_types.h"
#include "isr.h"
#include "led.h"

#define LED_BITS LED_WHITE

__attribute__ ((section ("startup")))
void main(void) {
	
	*(volatile uint32_t *)GPIO_FUNC_SEL0 = 0xffffffff;
	*(volatile uint32_t *)GPIO_FUNC_SEL1 = 0xffffffff;
	*(volatile uint32_t *)GPIO_FUNC_SEL2 = 0xffffffff;
	*(volatile uint32_t *)GPIO_FUNC_SEL3 = 0xffffffff;

	*(volatile uint32_t *)GPIO_PAD_DIR0 = 0xffffffff;
	*(volatile uint32_t *)GPIO_PAD_DIR1 = 0xffffffff;

	volatile uint32_t i;

	while(1) {

		*(volatile uint32_t *)GPIO_DATA0 = 0xffffffff;
		*(volatile uint32_t *)GPIO_DATA1 = 0xffffffff;
		
		for(i=0; i<DELAY; i++) { continue; }

		*(volatile uint32_t *)GPIO_DATA0 = 0x00000000;
		*(volatile uint32_t *)GPIO_DATA1 = 0x00000000;

		for(i=0; i<DELAY; i++) { continue; }

	};
}

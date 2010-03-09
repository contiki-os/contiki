#ifndef TESTS_H
#define TESTS_H

#include "put.h"
#include "led.h"

#define NL "\033[K\r\n"

void print_welcome(char* testname);
void dump_regs(uint32_t base, uint32_t len);
void print_packet(volatile packet_t *p);

#define toggle_gpio0(x) do { \
	if(bit_is_set(*GPIO_DATA0,x)) {	\
		*GPIO_DATA_RESET0 = (1 << x);		\
	} else {					\
		*GPIO_DATA_SET0 = (1 << x) ;		\
        } \
        } while(0);

#endif

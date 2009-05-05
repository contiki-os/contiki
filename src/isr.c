#include "embedded_types.h"
#include "isr.h"

#define reg32(x) (*(volatile uint32_t *)(x))

__attribute__ ((section (".irq")))
__attribute__ ((interrupt("IRQ"))) 
void irq(void)
{
	if(tmr_isr != NULL) {
		tmr_isr(); 
	}
}

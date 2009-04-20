#include "embedded_types.h"
#include "interrupt-utils.h"
#include "isr.h"

#define reg32(x) (*(volatile uint32_t *)(x))

__attribute__ ((interrupt("IRQ"))) 
void isr(void)
{
//	ISR_ENTRY();
	/* check for TMR0 interrupt */
//	tmr_isr(); // led turns off if I have this, indicating that the isr does jump to tmr_isr
//	if(reg32(INTSRC) & (1<<5)) { tmr_isr(); }
//	asm("SUBS PC,R14_IRQ,#4")
//	enableIRQ(); // I think this is necessary, but the LED never turns off when I have this
//	ISR_EXIT();  // behavior doesn't change if I have this or not. 

/* putting anything in here breaks the other code :( */

}

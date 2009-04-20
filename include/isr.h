#ifndef ISR_H
#define ISR_H

#include "embedded_types.h"

#define INTBASE (0x80020000)
#define INTENNUM_OFF (0x8)
#define INTSRC_OFF (0x30)

#define INTENNUM INTBASE + INTENNUM_OFF
#define INTSRC   INTBASE + INTSRC_OFF 


#define no_isrs() no_tmr_isr();

#define enable_tmr_irq() *(volatile uint32_t *)(INTENNUM)
#define no_tmr_isr() void tmr_isr(void) { return; }

extern void tmr_isr(void);

#endif


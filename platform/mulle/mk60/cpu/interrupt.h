#ifndef INTERRUPT_H
#define INTERRUPT_H

/*!< Macro to enable all interrupts. */
#define EnableInterrupts enable_interrupts() //asm(" CPSIE i");

  /*!< Macro to disable all interrupts. */
#define DisableInterrupts disable_interrupts() //asm(" CPSID i");

/***********************************************************************/

void enable_interrupts(void);
void disable_interrupts(void);

#endif //INTERRUPT_H

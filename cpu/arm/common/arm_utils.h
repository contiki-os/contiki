

#ifndef _ARM_UTILS_H
#define _ARM_UTILS_H

#include <contiki.h>
#include <core_cmFunc.h>
#include <core_cmInstr.h>
#include "irq.h"

static inline void __attribute__((section(".data"))) arm_jump_to_application(uint32_t startaddr)
{
    uint32_t stack_pointer, resethandler = 0;
    void (*user_code_entry)(void);

    //irq_disable();
    // At the beginning of the binary layout is the vector table
    // Set Vector table to that of the app
    SCB->VTOR = startaddr;

    // Conforming to startup-sim3l and linker script :
    // the first data is the initial stack pointer
    // The second is the reset handler address
    stack_pointer = *((uint32_t *) startaddr);
    //resethandler  = *((uint32_t *) (startaddr + 1));
    resethandler  = *((uint32_t *) (startaddr + 4));

    //printf("Update Stack Pointer = 0x%lX -> 0x%lX \n\r",__get_MSP(),stack_pointer);
    // Update stack pointer
    __set_MSP(stack_pointer);
    // Update process stack pointer
    __set_PSP(stack_pointer);

    // Make sure vector table offset and stack pointer are updated
    __DSB();

    // Jump To reset handler of application
    user_code_entry = (void (*)(void)) resethandler;
    // printf("user_code_entry = %p\n\r",user_code_entry);

    user_code_entry();
}

#endif

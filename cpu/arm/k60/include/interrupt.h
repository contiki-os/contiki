#ifndef INTERRUPT_H
#define INTERRUPT_H

#define MK60_ENABLE_INTERRUPT() asm(" CPSIE i")
#define MK60_DISABLE_INTERRUPT() asm(" CPSID i")

#define MK60_ENTER_CRITICAL_REGION() \
  unsigned int CRITICAL_PRIMASK; \
  asm volatile ( \
    "mrs %[old], primask\n" \
    "cpsid i" \
    :[old] "=&r" (CRITICAL_PRIMASK));

#define MK60_LEAVE_CRITICAL_REGION() \
  asm volatile ( \
    "msr primask, %[old]" \
    : \
    :[old] "r" (CRITICAL_PRIMASK));

#endif /* INTERRUPT_H */

#ifndef INTERRUPT_H
#define INTERRUPT_H

#ifdef __cplusplus
extern "C" {
#endif

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

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* INTERRUPT_H */

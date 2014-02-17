/**
 * \file
 *         Rtimer Library port for MK60DZ10.
 * \author
 *         Tony Persson <tony.persson@rubico.com>
 */

#ifndef __RTIMER_ARCH_H__
#define __RTIMER_ARCH_H__

#define RTIMER_ARCH_SECOND 2930 /* TODO: Change me as soon as the clocking is done */

rtimer_clock_t rtimer_arch_now(void);
void rtimer_arch_disable_irq(void);
void rtimer_arch_enable_irq(void);

#endif /* __RTIMER_ARCH_H__ */

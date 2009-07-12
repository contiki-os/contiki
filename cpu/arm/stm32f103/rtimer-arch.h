/**
 * \file
 *         Header file for the STM32F103-specific rtimer code
 * \author
 *         Simon Berg <ksb@users.sourceforge.net>
 */

#ifndef __RTIMER_ARCH_H__
#define __RTIMER_ARCH_H__

#include "sys/rtimer.h"

#define RTIMER_ARCH_SECOND (MCK/1024)

void rtimer_arch_set(rtimer_clock_t t);

rtimer_clock_t rtimer_arch_now(void);

#endif /* __RTIMER_ARCH_H__ */

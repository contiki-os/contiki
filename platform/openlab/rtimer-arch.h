/*
 * This file is part of HiKoB Openlab.
 *
 * HiKoB Openlab is free software: you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation, version 3.
 *
 * HiKoB Openlab is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with HiKoB Openlab. If not, see
 * <http://www.gnu.org/licenses/>.
 *
 * Copyright (C) 2011,2012 HiKoB.
 */

/**
 * \file
 *         rtimer-arch.h for OpenLab platforms
 * \author
 *         Clement Burin Des Roziers <clement.burin-des-roziers.at.hikob.com> 
 *         Antoine Fraboulet <antoine.fraboulet.at.hikob.com>
 */

#ifndef __RTIMER_ARCH_H__
#define __RTIMER_ARCH_H__

#include "soft_timer_delay.h"

#define RTIMER_ARCH_SECOND (SOFT_TIMER_FREQUENCY)

/* Do the math in 32bits to save precision.
 * Round to nearest integer rather than truncate. */
#define US_TO_RTIMERTICKS(US)  ((US) >= 0 ?                        \
                               (((int32_t)(US) * (RTIMER_ARCH_SECOND) + 500000) / 1000000L) :      \
                               ((int32_t)(US) * (RTIMER_ARCH_SECOND) - 500000) / 1000000L)

#define RTIMERTICKS_TO_US(T)   ((T) >= 0 ?                     \
                               (((int32_t)(T) * 1000000L + ((RTIMER_ARCH_SECOND) / 2)) / (RTIMER_ARCH_SECOND)) : \
                               ((int32_t)(T) * 1000000L - ((RTIMER_ARCH_SECOND) / 2)) / (RTIMER_ARCH_SECOND))

/* A 64-bit version because the 32-bit one cannot handle T >= 4295 ticks.
   Intended only for positive values of T. */
#define RTIMERTICKS_TO_US_64(T)  ((uint32_t)(((uint64_t)(T) * 1000000 + ((RTIMER_ARCH_SECOND) / 2)) / (RTIMER_ARCH_SECOND)))

/*
 * Contiki support functions
 */

rtimer_clock_t rtimer_arch_now(void);

#endif /* __RTIMER_ARCH_H__ */

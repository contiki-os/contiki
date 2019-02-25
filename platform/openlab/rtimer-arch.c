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
 *         rtimer-arch.c for OpenLab platforms
 * \author
 *         Clement Burin Des Roziers <clement.burin-des-roziers.at.hikob.com>
 *         Antoine Fraboulet <antoine.fraboulet.at.hikob.com>
 */

#include "contiki-conf.h"
#include "sys/energest.h"
#include "sys/rtimer.h"
#include "rtimer-arch.h"

#include "stm32/timer_.h"
#include "stm32f1xx/rcc.h"
#include "cortex-m3/cm3_nvic_registers.h"
#include "softtimer/soft_timer_.h"

#define LOG_LEVEL LOG_LEVEL_INFO
#define NO_DEBUG_HEADER
#include "lib/debug.h"

/*
 * rtimer is mapped on the soft_timer
 */
#define RTIMER_TIMER        ((const _openlab_timer_t *) softtim.timer)
#define RTIMER_CHANNEL      softtim.channel
#define RTIMER_IRQ_LINE     RTIMER_TIMER->irq_line

/*-----------------------------------------------------------------------------------*/

void rtimer_arch_init(void)
{
    /* timer is already started in agilefox_drivers.c */
    log_debug("rtimer_arch_init() called");
    // ensure lowest priority so that rtimer tasks can be interrupted
    nvic_set_priority(RTIMER_IRQ_LINE, 0xff);
}

/*-----------------------------------------------------------------------------------*/

rtimer_clock_t rtimer_arch_now(void)
{
    return timer_time( RTIMER_TIMER );
}

/*-----------------------------------------------------------------------------------*/

static void rtimer_cb( handler_arg_t arg, uint16_t value )
{
    (void) value;

    ENERGEST_ON(ENERGEST_TYPE_IRQ);
    timer_set_channel_compare( RTIMER_TIMER, RTIMER_CHANNEL, 0, NULL, NULL );
    rtimer_run_next();
    ENERGEST_OFF(ENERGEST_TYPE_IRQ);
}

void rtimer_arch_schedule(rtimer_clock_t t)
{
    timer_set_channel_compare( RTIMER_TIMER, RTIMER_CHANNEL, t, rtimer_cb, NULL );
}

/*-----------------------------------------------------------------------------------*/



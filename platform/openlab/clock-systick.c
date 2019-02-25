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
 *         Contiki add-ons for HiKoB OpenLab Fox
 *
 * \author
 *         Clement Burin Des Roziers <clement.burin-des-roziers.at.hikob.com> 
 *         Antoine Fraboulet <antoine.fraboulet.at.hikob.com>
 *         
 */

#include "platform.h"
#include "nvic.h"
#include "soft_timer_delay.h"
#define LOG_LEVEL LOG_LEVEL_DEBUG
#include "debug.h"

#include "contiki-conf.h"
#include "etimer.h"
#include "clock.h"

/*-----------------------------------------------------------------------------------*/

static volatile clock_time_t clock_cnt     = 0;
static volatile unsigned int clock_sec     = 0;
static volatile clock_time_t clock_sec_cnt = 0;
 
/*-----------------------------------------------------------------------------------*/

static void clock_alarm(handler_arg_t arg)
{
    (void)arg;
    clock_cnt ++;
    clock_sec_cnt ++;

    if (etimer_pending() && etimer_next_expiration_time() <= clock_cnt) 
    {
	etimer_request_poll();
    }

    if (clock_sec_cnt == CLOCK_SECOND) 
    {
	clock_sec ++;
	clock_sec_cnt = 0;
    }
}

/*-----------------------------------------------------------------------------------*/

void clock_init(void) 
{ 
    // Enable the SysTick, CLOCK_SECOND Hz
    log_debug("Starting systick timer at %dHz", CLOCK_SECOND);
    nvic_enable_systick(CLOCK_SECOND, clock_alarm, 0);
}

/*-----------------------------------------------------------------------------------*/

clock_time_t clock_time(void) 
{
    return clock_cnt; 
}

/*-----------------------------------------------------------------------------------*/

unsigned long clock_seconds(void)
{
    return clock_sec;
}

/*-----------------------------------------------------------------------------------*/

/*
 * delay the CPU in µs
 */

void clock_delay(unsigned int delay)
{
    soft_timer_delay_us(delay);
}

void clock_delay_usec(uint16_t delay)
{
    soft_timer_delay_us(delay);
}

/*-----------------------------------------------------------------------------------*/

/*
 * delay the CPU in clock ticks
 */
void clock_wait(clock_time_t delay) 
{
    soft_timer_delay_us( soft_timer_ticks_to_us(delay) );
}

/*-----------------------------------------------------------------------------------*/

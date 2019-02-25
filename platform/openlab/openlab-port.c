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
 * \file openlab-port.c
 *         Contiki add-ons for openlab platforms
 *
 * \author
 *         Antoine Fraboulet <antoine.fraboulet.at.hikob.com>
 *
 */

#include <stdint.h>

#include "platform-headers.h"
#include "softtimer/soft_timer_.h"
#include "event.h"

#include "debug.h"

/*-----------------------------------------------------------------------------------*/
/*
 * Function wrappers for Contiki
 *
 */

void xputc(char c)
{
    uart_transfer(uart_print, (const uint8_t *) &c, 1);
}

int
puts(char *str)
{
    printf((char*)str);
    return 0;
}

void
abort(void)
{
    log_error("abort() called");
    HALT();
}

/*-----------------------------------------------------------------------------------*/
/*
 * Empty wrappers for Openlab event library
 */
void event_debug() { }
void event_init() { }
/*---------------------------------------------------------------------------*/
/*
 * Empty wrappers for advanced options of Openlab soft_timer library
 */
void soft_timer_debug() { }

void soft_timer_update(handler_arg_t arg, uint16_t count)
{
    // Increment the update counter
    softtim.update_count += 1;
}

/*-----------------------------------------------------------------------------------*/
/*
 * Missing platform functions
 */
void platform_prevent_low_power() { }
void platform_release_low_power() { }
void platform_net_setup(void) { }

platform_reset_cause_t platform_reset_cause;

static int critical_count = 0;

void platform_enter_critical()
{
    asm volatile ("cpsid i");
    critical_count++;
}

void platform_exit_critical()
{
    critical_count--;
    if (critical_count == 0)
    {
        asm volatile("cpsie i");
    }
}

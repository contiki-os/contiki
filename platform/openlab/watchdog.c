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
 * \file watchdog.c
 *         Contiki add-ons for HiKoB OpenLab platforms
 *
 * \author
 *         Antoine Fraboulet <antoine.fraboulet.at.hikob.com>
 *         
 */

#include "platform.h"
#include "watchdog.h"
#include "nvic.h"
#include "soft_timer_delay.h"
#define LOG_LEVEL LOG_LEVEL_INFO
#include "debug.h"

#include "contiki.h"
#include "dev/watchdog.h"

/*---------------------------------------------------------------------------*/
void
watchdog_init(void)
{
    log_debug("watchdog_init");
}
/*---------------------------------------------------------------------------*/
void
watchdog_start(void)
{
    log_debug("watchdog_start");
    // Start the watchdog timer, max period
    watchdog_enable(WATCHDOG_DIVIDER_256, 0x0FFF);
}
/*---------------------------------------------------------------------------*/
void
watchdog_periodic(void)
{
    log_debug("watchdog_periodic");
    watchdog_clear();
}
/*---------------------------------------------------------------------------*/
void
watchdog_stop(void)
{
    //log_error("watchdog_stop not implemented");
}
/*---------------------------------------------------------------------------*/
void
watchdog_reboot(void)
{
    log_debug("watchdog_reboot");
    NVIC_RESET();
}
/*---------------------------------------------------------------------------*/

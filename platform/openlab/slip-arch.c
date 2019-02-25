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
 * Copyright (C) 2014 HiKoB.
 */

/**
 * \file slip-arch.c
 *         Slip device for HiKoB OpenLab platforms
 *
 * \author
 *         Damien Hedde <damien.hedde.at.hikob.com>
 *
 */

#include "platform.h"
#include "uart.h"

#define NO_DEBUG_HEADER
#include "debug.h"

#include "contiki-conf.h"
#include "dev/slip.h"

// use uart_print by default
#ifndef SLIP_ARCH_CONF_UART
#define SLIP_ARCH_CONF_UART uart_print
#endif

static void rx_handler(handler_arg_t arg, uint8_t c);

void slip_arch_writeb (unsigned char c)
{
  uart_transfer(SLIP_ARCH_CONF_UART, &c, 1);
}

void slip_arch_init (unsigned long ubr)
{
  uart_enable(SLIP_ARCH_CONF_UART, ubr);
  uart_set_rx_handler(SLIP_ARCH_CONF_UART, rx_handler, NULL);
  // configure highest priority to avoid missing bytes
  uart_set_irq_priority(SLIP_ARCH_CONF_UART, 0);
}

static void rx_handler(handler_arg_t arg, uint8_t c)
{
    slip_input_byte(c);
}

/*
 * Copyright (c) 2015, 3B Scientific GmbH - http://www.3bscientific.com/
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/**
 * \addtogroup cc3200-launchxl
 * @{
 *
 * \defgroup
 *
 * Implementation of TI CC2520 architecture dependent functions for
 * cc3200-launchxl platform
 * @{
 *
 * \file
 * Implementation of TI CC2520 architecture dependent functions for
 * cc3200-launchxl platform
 *
 * \author
 * 		   Björn Rennfanz <bjoern.rennfanz@3bscientific.com>
 *         Dominik Grauert <dominik.grauert@3bscientific.com>
 */

#include "contiki.h"
#include "contiki-net.h"

#include "dev/spi.h"
#include "dev/cc2520/cc2520.h"
#include "cc2520-arch.h"

#include <stdio.h>
#include <stdbool.h>

#define DEBUG 0
#if DEBUG
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...) do {} while (0)
#endif

/*
 * External CC2520 lock flag
 */
extern uint8_t locked;

/*
 * Soft interrupt process
 */
PROCESS(cc2520_arch_int_process, "CC2520 soft interrupt");
static uint8_t cc2520_arch_fifop_int;
/*---------------------------------------------------------------------------*/
void
cc2520_arch_init(void)
{
	// Initialize cc32xx SPI driver
	spi_init();

	// Unselect radio
	CC2520_SPI_DISABLE();
}
/*---------------------------------------------------------------------------*/
uint8_t
cc2520_arch_getreg(uint16_t regname)
{
  uint8_t reg;
  CC2520_READ_REG(regname, reg);
  return reg;
}
/*---------------------------------------------------------------------------*/
static void
cc2520_arch_int_pollhandler()
{
	// Check if soft interrupt is enabled
	if (cc2520_arch_fifop_int)
	{
		// Poll own process
		process_poll(&cc2520_arch_int_process);

		// Check if packet is pending
		if (CC2520_FIFOP_IS_1)
		{
			// Fire CC2520 interrupt handler
			cc2520_interrupt();
		}
	}
}
/*---------------------------------------------------------------------------*/
void
cc2520_arch_fifop_int_init()
{
	// Start soft interrupt process
	process_start(&cc2520_arch_int_process, NULL);
}
/*---------------------------------------------------------------------------*/
void
cc2520_arch_enable_fifop_int()
{
	// Enable soft interrupt
	cc2520_arch_fifop_int = true;
	process_poll(&cc2520_arch_int_process);
}
/*---------------------------------------------------------------------------*/
void
cc2520_arch_disable_fifop_int()
{
	// Disable soft interrupt
	cc2520_arch_fifop_int = false;
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(cc2520_arch_int_process, ev, data)
{
  PROCESS_POLLHANDLER(cc2520_arch_int_pollhandler());
  PROCESS_BEGIN();
  PRINTF("cc2520_arch_int_process: started\n");

  process_poll(&cc2520_arch_int_process);
  PROCESS_WAIT_UNTIL(ev == PROCESS_EVENT_EXIT);

  PRINTF("cc2520_arch_int_process: ended\n");
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/

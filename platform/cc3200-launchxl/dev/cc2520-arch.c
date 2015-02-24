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
int
splhigh(void)
{
	return 0;
}
/*---------------------------------------------------------------------------*/
void
splx(int saved)
{
}
/*---------------------------------------------------------------------------*/

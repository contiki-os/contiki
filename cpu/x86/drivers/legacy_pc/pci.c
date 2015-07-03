/*
 * Copyright (C) 2015, Intel Corporation. All rights reserved.
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

#include "pci.h"
#include "helpers.h"

/* I/O port for PCI configuration address */
#define PCI_CONFIG_ADDR_PORT 0xCF8
/* I/O port for PCI configuration data */
#define PCI_CONFIG_DATA_PORT 0xCFC

/*---------------------------------------------------------------------------*/
/* Initialize PCI configuration register address in preparation for accessing
 * the specified register.
 */
static void
set_addr(pci_config_addr_t addr)
{
  addr.en_mapping = 1;

  outl(PCI_CONFIG_ADDR_PORT, addr.raw);
}
/*---------------------------------------------------------------------------*/
/**
 * \brief      Read from the specified PCI configuration register.
 * \param addr Address of PCI configuration register.
 * \return     Value read from PCI configuration register.
 */
uint32_t
pci_config_read(pci_config_addr_t addr)
{
  set_addr(addr);

  return inl(PCI_CONFIG_DATA_PORT);
}
/*---------------------------------------------------------------------------*/

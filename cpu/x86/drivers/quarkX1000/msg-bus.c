/*
 * Copyright (C) 2015-2016, Intel Corporation. All rights reserved.
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

#include "msg-bus.h"
#include "pci.h"
#include "syscalls.h"

PROT_DOMAINS_ALLOC(dom_client_data_t, quarkX1000_msg_bus);

/** Message bus control register */
#define MCR_PCI_REG_ADDR 0xD0
/** Message data register */
#define MDR_PCI_REG_ADDR 0xD4
/** Message control register extension */
#define MCRX_PCI_REG_ADDR 0xD8

typedef union mcr {
  struct {
    uint32_t         : 4;
    uint32_t byte_en : 4;
    uint32_t reg_off : 8;
    uint32_t port    : 8;
    uint32_t opcode  : 8;
  };
  uint32_t raw;
} mcr_t;

typedef union mcrx {
  struct {
    uint32_t           : 8;
    uint32_t reg_off   : 24;
  };
  uint32_t raw;
} mcrx_t;

/*---------------------------------------------------------------------------*/
static void
request_op(uint8_t port, uint32_t reg_off, uint8_t opcode)
{
  pci_config_addr_t pci_addr = { .raw = 0 };
  mcr_t mcr = { .raw = 0 };
  mcrx_t mcrx = { .raw = 0 };

  pci_addr.reg_off = MCR_PCI_REG_ADDR;
  mcr.opcode = opcode;
  mcr.byte_en = 0xF;
  mcr.port = port;
  mcr.reg_off = reg_off & 0xFF;
  pci_config_write(pci_addr, mcr.raw);

  pci_addr.reg_off = MCRX_PCI_REG_ADDR;
  mcrx.reg_off = reg_off >> 8;
  pci_config_write(pci_addr, mcrx.raw);
}
/*---------------------------------------------------------------------------*/
/**
 * \brief         Read from a message bus register.
 * \param port    Port of message bus register to be read.
 * \param reg_off Register/offset identifier of message bus register to read.
 * \param val     Storage location for value that has been read.
 */
SYSCALLS_DEFINE_SINGLETON(quarkX1000_msg_bus_read,
                          quarkX1000_msg_bus,
                          uint8_t port,
                          uint32_t reg_off,
                          uint32_t *val)
{
  uint32_t *loc_val;
  pci_config_addr_t pci_addr = { .raw = 0 };

  PROT_DOMAINS_VALIDATE_PTR(loc_val, val, sizeof(*val));

  request_op(port, reg_off, 0x10);

  pci_addr.reg_off = MDR_PCI_REG_ADDR;
  *loc_val = pci_config_read(pci_addr);
}
/*---------------------------------------------------------------------------*/
/**
 * \brief         Write to a message bus register.
 * \param port    Port of message bus register to be written.
 * \param reg_off Register/offset identifier of message bus register to write.
 * \param val     Value to write.
 */
SYSCALLS_DEFINE_SINGLETON(quarkX1000_msg_bus_write,
                          quarkX1000_msg_bus,
                          uint8_t port,
                          uint32_t reg_off,
                          uint32_t val)
{
  pci_config_addr_t pci_addr = { .raw = 0 };

  pci_addr.reg_off = MDR_PCI_REG_ADDR;
  pci_config_write(pci_addr, val);

  request_op(port, reg_off, 0x11);
}
/*---------------------------------------------------------------------------*/
void
quarkX1000_msg_bus_init(void)
{
  PROT_DOMAINS_INIT_ID(quarkX1000_msg_bus);
  prot_domains_reg(&quarkX1000_msg_bus, 0, 0, 0, 0, true);
  SYSCALLS_INIT(quarkX1000_msg_bus_read);
  SYSCALLS_AUTHZ(quarkX1000_msg_bus_read, quarkX1000_msg_bus);
  SYSCALLS_INIT(quarkX1000_msg_bus_write);
  SYSCALLS_AUTHZ(quarkX1000_msg_bus_write, quarkX1000_msg_bus);
}
/*---------------------------------------------------------------------------*/
void
quarkX1000_msg_bus_lock(void)
{
  SYSCALLS_DEAUTHZ(quarkX1000_msg_bus_read, quarkX1000_msg_bus);
  SYSCALLS_DEAUTHZ(quarkX1000_msg_bus_write, quarkX1000_msg_bus);
}
/*---------------------------------------------------------------------------*/

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

#include <assert.h>

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
/**
 * \brief      Write to the PCI configuration data port.
 * \param addr Address of PCI configuration register.
 * \param data Value to write.
 */
void
pci_config_write(pci_config_addr_t addr, uint32_t data)
{
  set_addr(addr);

  outl(PCI_CONFIG_DATA_PORT, data);
}
/*---------------------------------------------------------------------------*/
/**
 * \brief       Enable PCI command bits of the specified PCI configuration
 *              register.
 * \param addr  Address of PCI configuration register.
 * \param flags Flags used to enable PCI command bits.
 */
void
pci_command_enable(pci_config_addr_t addr, uint32_t flags)
{
  uint32_t data;

  addr.reg_off = 0x04; /* PCI COMMAND_REGISTER */

  data = pci_config_read(addr);
  pci_config_write(addr, data | flags);
}
/*---------------------------------------------------------------------------*/
/**
 * \brief       Set current PIRQ to interrupt queue agent. PCI based interrupts
 *              PIRQ[A:H] are then available for consumption by either the 8259
 *              PICs or the IO-APIC depending on configuration of the 8 PIRQx
 *              Routing Control Registers PIRQ[A:H]. See also pci_pirq_set_irq().
 * \param agent Interrupt Queue Agent to be used, IRQAGENT[0:3].
 * \param pin   Interrupt Pin Route to be used, INT[A:D].
 * \param pirq  PIRQ to be used, PIRQ[A:H].
 * \return      Returns 0 on success and a negative number otherwise.
 */
int
pci_irq_agent_set_pirq(IRQAGENT agent, INTR_PIN pin, PIRQ pirq)
{
  pci_config_addr_t pci;
  uint16_t value;
  uint32_t rcba_addr, offset = 0;

  assert(agent >= IRQAGENT0 && agent <= IRQAGENT3);
  assert(pin >= INTA && pin <= INTD);
  assert(pirq >= PIRQA && pirq <= PIRQH);

  pci.raw = 0;
  pci.bus = 0;
  pci.dev = 31;
  pci.func = 0;
  pci.reg_off = 0xF0; /* Root Complex Base Address Register */

  /* masked to clear non-address bits. */
  rcba_addr = pci_config_read(pci) & ~0x3FFF;

  switch(agent) {
  case IRQAGENT0:
    if (pin != INTA)
      return -1;
    offset = 0x3140;
    break;
  case IRQAGENT1:
    offset = 0x3142;
    break;
  case IRQAGENT2:
    if (pin != INTA)
      return -1;
    offset = 0x3144;
    break;
  case IRQAGENT3:
    offset = 0x3146;
  }

  value = *(uint16_t*)(rcba_addr + offset);

  /* clear interrupt pin route and set corresponding pirq. */
  switch(pin) {
  case INTA:
    value &= ~0xF;
    value |= pirq;
    break;
  case INTB:
    value &= ~0xF0;
    value |= (pirq << 4);
    break;
  case INTC:
    value &= ~0xF00;
    value |= (pirq << 8);
    break;
  case INTD:
    value &= ~0xF000;
    value |= (pirq << 12);
  }

  *(uint16_t*)(rcba_addr + offset) = value;

  return 0;
}
/*---------------------------------------------------------------------------*/
/**
 * \brief                 Set current IRQ to PIRQ. The interrupt router can be
 *                        programmed to allow PIRQ[A:H] to be routed internally
 *                        to the 8259 as ISA compatible interrupts. See also
 *                        pci_irq_agent_set_pirq().
 * \param pirq            PIRQ to be used, PIRQ[A:H].
 * \param pin             IRQ to be used, IRQ[0:15].
 * \param route_to_legacy Whether or not the interrupt should be routed to PIC 8259.
 */
void
pci_pirq_set_irq(PIRQ pirq, uint8_t irq, uint8_t route_to_legacy)
{
  pci_config_addr_t pci;
  uint32_t value;

  assert(pirq >= PIRQA && pirq <= PIRQH);
  assert(irq >= 0 && irq <= 0xF);
  assert(route_to_legacy == 0 || route_to_legacy == 1);

  pci.raw = 0;
  pci.bus = 0;
  pci.dev = 31;
  pci.func = 0;
  pci.reg_off = (pirq <= PIRQD) ? 0x60 : 0x64; /* PABCDRC and PEFGHRC Registers */

  value = pci_config_read(pci);

  switch(pirq) {
  case PIRQA:
  case PIRQE:
    value &= ~0x8F;
    value |= irq;
    value |= (!route_to_legacy << 7);
    break;
  case PIRQB:
  case PIRQF:
    value &= ~0x8F00;
    value |= (irq << 8);
    value |= (!route_to_legacy << 15);
    break;
  case PIRQC:
  case PIRQG:
    value &= ~0x8F0000;
    value |= (irq << 16);
    value |= (!route_to_legacy << 23);
    break;
  case PIRQD:
  case PIRQH:
    value &= ~0x8F000000;
    value |= (irq << 24);
    value |= (!route_to_legacy << 31);
  }

  set_addr(pci);
  outl(PCI_CONFIG_DATA_PORT, value);
}
/*---------------------------------------------------------------------------*/
/**
 * \brief          Initialize a structure for a PCI device driver that performs
 *                 MMIO to address range 0.  Assumes that device has already
 *                 been configured with an MMIO address range 0, e.g. by
 *                 firmware.
 * \param c_this   Structure that will be initialized to represent the driver.
 * \param pci_addr PCI base address of device.
 * \param meta     Base address of optional driver-defined metadata.
 */
void
pci_init(pci_driver_t *c_this, pci_config_addr_t pci_addr, uintptr_t meta)
{
  /* The BAR value is masked to clear non-address bits. */
  c_this->mmio = pci_config_read(pci_addr) & ~0xFFF;
  c_this->meta = meta;
}
/*---------------------------------------------------------------------------*/

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

#include "gpio.h"
#include "helpers.h"
#include "interrupt.h"
#include "pic.h"

/* GPIO Controler Registers */
#define SWPORTA_DR    0x00
#define SWPORTA_DDR   0x04
#define INTEN         0x30
#define INTMASK       0x34
#define INTTYPE_LEVEL 0x38
#define INT_POLARITY  0x3c
#define INTSTATUS     0x40
#define RAW_INTSTATUS 0x44
#define DEBOUNCE      0x48
#define PORTA_EOI     0x4c
#define EXT_PORTA     0x50
#define LS_SYNC       0x60

#define PINS 8

#define GPIO_IRQ 10
#define GPIO_INT PIC_INT(GPIO_IRQ)

struct gpio_internal_data {
  pci_driver_t pci;
  quarkX1000_gpio_callback callback;
};

static struct gpio_internal_data data;

static inline uint32_t
read(uint32_t offset)
{
  uint32_t res;
  PCI_MMIO_READL(data.pci, res, offset);
  return res;
}

static inline void
write(uint32_t offset, uint32_t val)
{
  PCI_MMIO_WRITEL(data.pci, offset, val);
}

/* value must be 0x0 or 0x1 */
static void
set_bit(uint32_t offset, uint32_t bit, uint32_t value)
{
  uint32_t reg;

  reg = read(offset);

  reg &= ~BIT(bit);
  reg |= value << bit;

  write(offset, reg);
}

static void
gpio_isr(void)
{
  uint32_t int_status;

  int_status = read(INTSTATUS);

  if (data.callback)
    data.callback(int_status);

  write(PORTA_EOI, -1);
}

static void
gpio_interrupt_config(uint8_t pin, int flags)
{
  /* set as input */
  set_bit(SWPORTA_DDR, pin, 0);

  /* set interrupt enabled */
  set_bit(INTEN, pin, 1);

  /* unmask interrupt */
  set_bit(INTMASK, pin, 0);

  /* set active high/low */
  set_bit(INT_POLARITY, pin, !!(flags & QUARKX1000_GPIO_ACTIVE_HIGH));

  /* set level/edge */
  set_bit(INTTYPE_LEVEL, pin, !!(flags & QUARKX1000_GPIO_EDGE));

  /* set debounce */
  set_bit(DEBOUNCE, pin, !!(flags & QUARKX1000_GPIO_DEBOUNCE));

  /* set clock synchronous */
  set_bit(LS_SYNC, 0, !!(flags & QUARKX1000_GPIO_CLOCK_SYNC));
}

int
quarkX1000_gpio_config(uint8_t pin, int flags)
{
  if (((flags & QUARKX1000_GPIO_IN) && (flags & QUARKX1000_GPIO_OUT)) ||
    ((flags & QUARKX1000_GPIO_INT) && (flags & QUARKX1000_GPIO_OUT))) {
    return -1;
  }

  if (flags & QUARKX1000_GPIO_INT) {
    gpio_interrupt_config(pin, flags);
  } else {
    /* set direction */
    set_bit(SWPORTA_DDR, pin, !!(flags & QUARKX1000_GPIO_OUT));

    /* set interrupt disabled */
    set_bit(INTEN, pin, 0);
  }

  return 0;
}

int
quarkX1000_gpio_config_port(int flags)
{
  uint8_t i;

  for (i = 0; i < PINS; i++) {
    if (quarkX1000_gpio_config(i, flags) < 0) {
      return -1;
    }
  }

  return 0;
}

int
quarkX1000_gpio_read(uint8_t pin, uint8_t *value)
{
  uint32_t value32 = read(EXT_PORTA);
  *value = !!(value32 & BIT(pin));

  return 0;
}

int
quarkX1000_gpio_write(uint8_t pin, uint8_t value)
{
  set_bit(SWPORTA_DR, pin, !!value);
  return 0;
}

int
quarkX1000_gpio_read_port(uint8_t *value)
{
  uint32_t value32 = read(EXT_PORTA);
  *value = value32 & ~0xFFFFFF00;

  return 0;
}

int
quarkX1000_gpio_write_port(uint8_t value)
{
  write(SWPORTA_DR, value);
  return 0;
}

int
quarkX1000_gpio_set_callback(quarkX1000_gpio_callback callback)
{
  data.callback = callback;
  return 0;
}

void
quarkX1000_gpio_clock_enable(void)
{
  set_bit(LS_SYNC, 0, 1);
}

void
quarkX1000_gpio_clock_disable(void)
{
  set_bit(LS_SYNC, 0, 0);
}

static void
gpio_handler(void)
{
  gpio_isr();

  pic_eoi(GPIO_IRQ);
}

int
quarkX1000_gpio_init(void)
{
  pci_config_addr_t pci_addr;

  pci_addr.raw = 0;
  pci_addr.bus = 0;
  pci_addr.dev = 21;
  pci_addr.func = 2;
  pci_addr.reg_off = PCI_CONFIG_REG_BAR1;

  pci_command_enable(pci_addr, PCI_CMD_1_MEM_SPACE_EN);

  SET_INTERRUPT_HANDLER(GPIO_INT, 0, gpio_handler);

  if (pci_irq_agent_set_pirq(IRQAGENT3, INTA, PIRQC) < 0)
    return -1;

  pci_pirq_set_irq(PIRQC, GPIO_IRQ, 1);

  pci_init(&data.pci, pci_addr, 0);

  data.callback = 0;

  quarkX1000_gpio_clock_enable();

  /* clear registers */
  write(INTEN, 0);
  write(INTMASK, 0);
  write(PORTA_EOI, 0);

  pic_unmask_irq(GPIO_IRQ);

  return 0;
}

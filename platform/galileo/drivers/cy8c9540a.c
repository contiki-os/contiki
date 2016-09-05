/*
 * Copyright (C) 2016, Intel Corporation. All rights reserved.
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

#include "cy8c9540a.h"

#include <assert.h>
#include "i2c.h"
#include <stdio.h>

/* Change this to 0x21 if J2 is set to 1-2
 * (covering the pin marked with the white triangle). */
#define I2C_ADDR 0x20

#define REG_PORT_SEL 0x18
#define REG_PORT_DIR 0x1C

#define PORT_CNT 6

/* Cache the current state of each port to obviate the need for reading before
 * writing to output ports when simply updating a single pin.
 */
static uint8_t out_cache[PORT_CNT];

/*---------------------------------------------------------------------------*/
static void
write_reg(uint8_t reg, uint8_t data)
{
  uint8_t pkt[] = { reg, data };
  assert(quarkX1000_i2c_polling_write(pkt, sizeof(pkt), I2C_ADDR) == 0);
}
/*---------------------------------------------------------------------------*/
static uint8_t
read_reg(uint8_t reg)
{
  uint8_t data;
  assert(quarkX1000_i2c_polling_write(&reg, 1, I2C_ADDR) == 0);
  assert(quarkX1000_i2c_polling_read(&data, 1, I2C_ADDR) == 0);
  return data;
}
/*---------------------------------------------------------------------------*/
void
cy8c9540a_init(void)
{
  uint8_t status;

  /* has to init after I2C master */
  assert(quarkX1000_i2c_is_available());

  status = read_reg(0x2E);
  if((status >> 4) != 4) {
    fprintf(stderr, "Failed to communicate with CY8C9540A. Perhaps jumper J2 "
                    "is not set to 2-3? Halting.\n");
    halt();
  }
}
/*---------------------------------------------------------------------------*/
/**
 * \brief Set the direction (in or out) for the indicated GPIO pin.
 */
void
cy8c9540a_set_port_dir(cy8c9540a_bit_addr_t addr, cy8c9540a_port_dir_t dir)
{
  uint8_t mask;

  assert(addr.port < PORT_CNT);

  write_reg(REG_PORT_SEL, addr.port);
  mask = read_reg(REG_PORT_DIR);
  mask &= ~(1 << addr.pin);
  mask |= ((uint8_t)dir) << addr.pin;
  write_reg(REG_PORT_DIR, mask);
}
/*---------------------------------------------------------------------------*/
/**
 * \brief Set the drive mode for the indicated GPIO pin.
 */
void
cy8c9540a_set_drive_mode(cy8c9540a_bit_addr_t addr,
                         cy8c9540a_drive_mode_t drv_mode)
{
  assert(addr.port < PORT_CNT);

  write_reg(REG_PORT_SEL, addr.port);
  write_reg((uint8_t)drv_mode, 1 << addr.pin);
}
/*---------------------------------------------------------------------------*/
bool
cy8c9540a_read(cy8c9540a_bit_addr_t addr)
{
  assert(addr.port < PORT_CNT);

  return ((read_reg(addr.port) >> addr.pin) & 1) == 1;
}
/*---------------------------------------------------------------------------*/
void
cy8c9540a_write(cy8c9540a_bit_addr_t addr, bool val)
{
  assert(addr.port < PORT_CNT);

  out_cache[addr.port] &= ~(1 << addr.pin);
  out_cache[addr.port] |= ((uint8_t)val) << addr.pin;
  write_reg(8 + addr.port, out_cache[addr.port]);
}
/*---------------------------------------------------------------------------*/

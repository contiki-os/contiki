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

#ifndef CPU_X86_DRIVERS_QUARKX1000_I2C_H_
#define CPU_X86_DRIVERS_QUARKX1000_I2C_H_

#include "pci.h"

typedef enum {
  QUARKX1000_I2C_SPEED_STANDARD,
  QUARKX1000_I2C_SPEED_FAST
} QUARKX1000_I2C_SPEED;

typedef enum {
  QUARKX1000_I2C_ADDR_MODE_7BIT,
  QUARKX1000_I2C_ADDR_MODE_10BIT
} QUARKX1000_I2C_ADDR_MODE;

typedef void (*quarkX1000_i2c_callback)(void);

struct quarkX1000_i2c_config {
  QUARKX1000_I2C_SPEED speed;
  QUARKX1000_I2C_ADDR_MODE addressing_mode;

  quarkX1000_i2c_callback cb_rx;
  quarkX1000_i2c_callback cb_tx;
  quarkX1000_i2c_callback cb_err;
};

int quarkX1000_i2c_init(void);
int quarkX1000_i2c_configure(struct quarkX1000_i2c_config *config);
int quarkX1000_i2c_is_available(void);

int quarkX1000_i2c_read(uint8_t *buf, uint8_t len, uint16_t addr);
int quarkX1000_i2c_write(uint8_t *buf, uint8_t len, uint16_t addr);

int quarkX1000_i2c_polling_read(uint8_t *buf, uint8_t len, uint16_t addr);
int quarkX1000_i2c_polling_write(uint8_t *buf, uint8_t len, uint16_t addr);

#endif /* CPU_X86_DRIVERS_QUARKX1000_I2C_H_ */

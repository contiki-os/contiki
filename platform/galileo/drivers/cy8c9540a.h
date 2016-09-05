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

#ifndef PLATFORM_GALILEO_DRIVERS_CY8C9540A_H_
#define PLATFORM_GALILEO_DRIVERS_CY8C9540A_H_

#include <stdbool.h>
#include <stdint.h>

/* Driver for Cypress Semiconductors CY8C9540A device used for GPIO, PWM, and
 * pinmuxing on the first generation Intel Galileo.
 */

/* The numeric value of each drive mode corresponds to the device register
 * address for selecting that mode. Only a subset of the available modes are
 * listed here.
 */
typedef enum cy8c9540a_drive_mode {
  CY8C9540A_DRIVE_PULL_UP = 0x1D,
  CY8C9540A_DRIVE_PULL_DOWN = 0x1E,
  CY8C9540A_DRIVE_STRONG = 0x21,
  CY8C9540A_DRIVE_HI_Z = 0x23
} cy8c9540a_drive_mode_t;

typedef enum cy8c9540a_port_dir {
  CY8C9540A_PORT_DIR_OUT = 0,
  CY8C9540A_PORT_DIR_IN = 1
} cy8c9540a_port_dir_t;

typedef struct cy8c9540a_bit_addr {
  uint8_t port;
  int pin;
} cy8c9540a_bit_addr_t;

#define CY8C9540A_BIT_ADDR_INVALID_PORT 0xFF

void cy8c9540a_init(void);
void cy8c9540a_set_port_dir(cy8c9540a_bit_addr_t addr,
                            cy8c9540a_port_dir_t dir);
void cy8c9540a_set_drive_mode(cy8c9540a_bit_addr_t addr,
                              cy8c9540a_drive_mode_t drv_mode);
bool cy8c9540a_read(cy8c9540a_bit_addr_t addr);
void cy8c9540a_write(cy8c9540a_bit_addr_t addr, bool val);

#endif /* PLATFORM_GALILEO_DRIVERS_CY8C9540A_H_ */

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

#ifndef CPU_X86_DRIVERS_QUARKX1000_MSG_BUS_H_
#define CPU_X86_DRIVERS_QUARKX1000_MSG_BUS_H_

#include <stdint.h>

/* Routines for accessing the message bus.
 *
 * The Intel Quark X1000 SoC includes a message bus that is accessible
 * via PCI configuration registers.  It communicates to various SoC
 * components such as the Isolated Memory Region (IMR) registers and the
 * Remote Management Unit.
 *
 * Refer to Intel Quark SoC X1000 Datasheet, Section 12.5 for more details on
 * the message bus.
 */

void quarkX1000_msg_bus_init(void);
void quarkX1000_msg_bus_lock(void);
void quarkX1000_msg_bus_read(uint8_t port, uint32_t reg_off, uint32_t *val);
void quarkX1000_msg_bus_write(uint8_t port, uint32_t reg_off, uint32_t val);

#endif /* CPU_X86_DRIVERS_QUARKX1000_MSG_BUS_H_ */

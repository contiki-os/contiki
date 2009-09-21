/*
 * Copyright (c) 2008, Swedish Institute of Computer Science
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
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * This file is part of the Contiki operating system.
 *
 */

/**
 * \file
 *	Coffee architecture-dependent functionality for the ESB platform.
 * \author
 * 	Nicolas Tsiftes <nvt@sics.se>
 * 	Niclas Finne <nfi@sics.se>
 */

#include "cfs-coffee-arch.h"

static const unsigned char nullb[COFFEE_SECTOR_SIZE < 32 ? COFFEE_SECTOR_SIZE : 32] = {0};

void
cfs_coffee_arch_erase(uint16_t sector)
{
  unsigned int i;
  for(i = 0; i <= COFFEE_SECTOR_SIZE - sizeof(nullb); i += sizeof(nullb)) {
    eeprom_write(COFFEE_START + sector * COFFEE_SECTOR_SIZE + i,
                 (unsigned char *)nullb, sizeof(nullb));
  }
  if(i < COFFEE_SECTOR_SIZE) {
    eeprom_write(COFFEE_START + sector * COFFEE_SECTOR_SIZE + i,
                 (unsigned char *)nullb, COFFEE_SECTOR_SIZE - i);

  }
}

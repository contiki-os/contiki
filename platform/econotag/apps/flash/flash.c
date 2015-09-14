/*
 * Copyright (c) 2014, Lars Schmertmann <SmallLars@t-online.de>.
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
 *
 * This file is part of the Contiki operating system.
 *
 */

#include "flash.h"

#define FLASH_BLOCK_SIZE 0x01000

#define FLASH_BLOCK_11   0x18000
#define FLASH_BLOCK_21   0x1A000

#ifndef FLASH_CONF_B1
#define FLASH_CONF_B1      FLASH_BLOCK_SIZE
#endif

#ifndef FLASH_CONF_B2
#define FLASH_CONF_B2      FLASH_BLOCK_SIZE
#endif

#define DEBUG 0

#if DEBUG
#include <stdio.h>
#include "mc1322x.h"
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif

uint16_t stackPointer;

/* private prototypes  ----------------------------------------------------- */

flash_addr_t getAddr(flash_addr_t address);

/* public functions -------------------------------------------------------- */

void
flash_init()
{
  PRINTF("Initializing flash ... ");

  nvm_erase(gNvmInternalInterface_c, gNvmType_SST_c, 0x0F000000);

  nvm_write(gNvmInternalInterface_c, gNvmType_SST_c, "\001", FLASH_BLOCK_11, 1);
  nvm_write(gNvmInternalInterface_c, gNvmType_SST_c, "\001", FLASH_BLOCK_21, 1);

  uint32_t i;
  for(i = 1; i < 0x2000; i++) {
#if DEBUG
    if(i % 0x400 == 0) {
      PRINTF(" .");
    }
#endif
    nvm_write(gNvmInternalInterface_c, gNvmType_SST_c, "\0", FLASH_BLOCK_11 + i, 1);
    nvm_write(gNvmInternalInterface_c, gNvmType_SST_c, "\0", FLASH_BLOCK_21 + i, 1);
  }

  PRINTF("DONE\n");
}
nvmErr_t
flash_getVar(void *dest, flash_addr_t address, uint32_t numBytes)
{
  address = getAddr(address);

  if(address >= 0x18000 && address <= 0x1EFFF) {
    PRINTF("Read from Adress: %p\n", address);
    nvmErr_t err = nvm_read(gNvmInternalInterface_c, gNvmType_SST_c, dest, address, numBytes);
    if(err) {
      PRINTF("Read error, nmv_error: %u\n", err);
      return err;
    }
    return gNvmErrNoError_c;
  }

  PRINTF("Read error - Invalid pointer.\n");
  return gNvmErrInvalidPointer_c;
}
nvmErr_t
flash_setVar(void *src, flash_addr_t address, uint32_t numBytes)
{
#if DEBUG
  printf("SetVar - START . ");
  uint32_t time = *MACA_CLK;
#endif

  if(address >= 8192) {
    PRINTF("Write error - Invalid pointer.\n");
    return gNvmErrInvalidPointer_c;
  }
  uint32_t block_len = (address < 4096 ? FLASH_CONF_B1 : FLASH_CONF_B2);

  address = getAddr(address);

  flash_addr_t src_block = address & 0xFF000;
  flash_addr_t dst_block = src_block ^ 0x01000;
  address = address & 0x00FFF;

  if(address < 1 || address >= block_len) {
    PRINTF("Write error - Invalid pointer.\n");
    return gNvmErrInvalidPointer_c;
  }
  if(address + numBytes > block_len) {
    PRINTF("Write error - Var is to long.\n");
    return gNvmErrAddressSpaceOverflow_c;
  }

  nvm_erase(gNvmInternalInterface_c, gNvmType_SST_c, 1 << (dst_block / FLASH_BLOCK_SIZE));

  uint32_t i;
  uint8_t buf;
  for(i = 0; i < address; i++) {
    nvm_read(gNvmInternalInterface_c, gNvmType_SST_c, &buf, src_block + i, 1);
    nvm_write(gNvmInternalInterface_c, gNvmType_SST_c, &buf, dst_block + i, 1);
  }
  PRINTF("Write to adress: %p\n", dst_block + i);
  nvm_write(gNvmInternalInterface_c, gNvmType_SST_c, src, dst_block + i, numBytes);
  for(i += numBytes; i < block_len; i++) {
    nvm_read(gNvmInternalInterface_c, gNvmType_SST_c, &buf, src_block + i, 1);
    nvm_write(gNvmInternalInterface_c, gNvmType_SST_c, &buf, dst_block + i, 1);
  }

  nvm_erase(gNvmInternalInterface_c, gNvmType_SST_c, 1 << (src_block / FLASH_BLOCK_SIZE));

#if DEBUG
  time = *MACA_CLK - time;
  printf("FINISHED AFTER %u MS\n", time / 250);
#endif

  return gNvmErrNoError_c;
}
nvmErr_t
flash_cmp(void *src, flash_addr_t address, uint32_t numBytes)
{
  address = getAddr(address);

  if(address >= 0x18000 && address <= 0x1EFFF) {
    return nvm_verify(gNvmInternalInterface_c, gNvmType_SST_c, src, address, numBytes);
  }
  PRINTF("Read error - Invalid pointer.\n");
  return gNvmErrInvalidPointer_c;
}
void
flash_stack_init()
{
  stackPointer = 0;
  nvm_erase(gNvmInternalInterface_c, gNvmType_SST_c, 1 << (FLASH_STACK / FLASH_BLOCK_SIZE));
}
nvmErr_t
flash_stack_push(uint8_t *src, uint32_t numBytes)
{
  if(stackPointer + numBytes > FLASH_BLOCK_SIZE) {
    return gNvmErrAddressSpaceOverflow_c;
  }

  nvm_write(gNvmInternalInterface_c, gNvmType_SST_c, src, FLASH_STACK + stackPointer, numBytes);
  stackPointer += numBytes;
  return gNvmErrNoError_c;
}
uint32_t
flash_stack_size()
{
  return stackPointer;
}
/* private functions ------------------------------------------------------- */

flash_addr_t
getAddr(flash_addr_t address)
{
  if(address >= 0x02000) {
    return address;
  }

  flash_addr_t block = (address & 0x01000 ? FLASH_BLOCK_21 : FLASH_BLOCK_11);
  uint8_t blockcheck = (flash_cmp("\001", block, 1) == 0 ? 0 : 1);
  return block + (blockcheck << 12) + (address & 0x00FFF);
}

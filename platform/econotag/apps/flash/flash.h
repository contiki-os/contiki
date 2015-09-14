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

/**
 * \file
 *      App for easy usage of additional flash memory
 *
 *      Purposes of the different flash blocks
 *      1 : 0x18000 - 0x18FFF : Random access block 1.1
 *      2 : 0x19000 - 0x19FFF : Random access block 1.2
 *      3 : 0x1A000 - 0x1AFFF : Random access block 2.1
 *      4 : 0x1B000 - 0x1BFFF : Random access block 2.2
 *      5 : 0x1C000 - 0x1CFFF : Stack without pop function
 *      6 : 0x1D000 - 0x1DFFF : Read only
 *      7 : 0x1E000 - 0x1EFFF : Read only
 *      8 : 0x1F000 - 0x1FFFF : System reserved
 *
 *      This app only allows write access to blocks 1 - 5
 *      and read access to blocks 1 - 7.
 *
 *      To use the stack in block 5 you need: flash_stack_init,
 *      flash_stack_push, flash_stack_size, flash_stack_read.
 *
 *      To use the random access blocks 1.x and 2.x you need: flash_init,
 *      flash_getVar, flash_setVar, flash_cmp.
 *
 *      Blocks 1.x and 2.x are accessible with adresses
 *      0x0001 - 0x0FFF and 0x1001 - 0x1FFF.
 *
 *      To be able to write to flash memory, its required to delete
 *      it first, but its only possible to delete a full block. So this
 *      app copies the data of a block, changing the requested data.
 *      Copying a block needs time. So when you only use the first N bytes,
 *      you can set FLASH_CONF_B1=N and FLASH_CONF_B2=N in your makefile
 *      to optimize speed.
 *
 *      You can find an example in examples/econotag-flash-test.
 *
 * \author
 *      Lars Schmertmann <SmallLars@t-online.de>
 */

#ifndef FLASH_H_
#define FLASH_H_

#include <nvm.h>

#define FLASH_STACK 0x1C000

typedef uint32_t flash_addr_t;

/**
 * \brief    Initialize or clear random access blocks
 *
 *           To use the random access blocks, you need to call this
 *           function first. You can also use it to delete all data
 *           in this blocks.
 */
void flash_init();

/**
 * \brief    Read data from flash memory
 *
 *           Reads data from flash memory and stores it into RAM.
 *           You can read the flash area 0x18000 - 0x1EFFF. Addresses
 *           0x0000 - 0x1FFF will be mapped to 0x18000 - 0x1BFFF.
 *
 * \param    dest       Memory area to store the data
 * \param    address    Area in flash memory to read from
 * \param    numBytes   Number of bytes to read
 *
 * \return   gNvmErrNoError_c (0) if read was successfull
 */
nvmErr_t flash_getVar(void *dest, flash_addr_t address, uint32_t numBytes);

/**
 * \brief    Write data to flash memory
 *
 *           Writes data to flash memory. Valid addresses are
 *           0x0001 - 0x0FFF and 0x1001 - 0x1FFF -> Mapped to
 *           0x18000 - 0x1BFFF.
 *
 * \param    src        Memory area with data to store in flash memory
 * \param    address    Area in flash memory to write
 * \param    numBytes   Number of bytes to write
 *
 * \return   gNvmErrNoError_c (0) if write was successfull
 */
nvmErr_t flash_setVar(void *src, flash_addr_t address, uint32_t numBytes);

/**
 * \brief    Compares data from RAM with flash memory
 *
 *           Compares data from RAM with flash memory.
 *           Valid addresses are 0x18000 - 0x1EFFF. Addresses
 *           0x0 - 0x1FFF will be mapped to 0x18000 - 0x1BFFF.
 *
 * \param    src        Memory area with data to compare
 * \param    address    Area in flash memory
 * \param    numBytes   Number of bytes to compare
 *
 * \return   0 if data is matching
 */
nvmErr_t flash_cmp(void *src, flash_addr_t address, uint32_t numBytes);

/**
 * \brief    Stack initialisation
 *
 *           Clears and initializes the stack.
 */
void flash_stack_init();

/**
 * \brief    Push data to stack
 *
 *           Pushes numBytes from src to stack into flash memory.
 *
 * \param    src        Memory area with data to store in flash memory
 * \param    numBytes   Number of bytes to write
 *
 * \return   gNvmErrNoError_c (0) if push was successfull
 */
nvmErr_t flash_stack_push(uint8_t *src, uint32_t numBytes);

/**
 * \brief    Stacksize
 *
 *           Returns the size of data in stack
 *
 * \return   Number of bytes in stack
 */
uint32_t flash_stack_size();

/**
 * \brief    Read data from stack
 *
 *           Reads data from stack (without removing it) and stores it into RAM.
 *
 * \param    dest       Memory area to store the data
 * \param    offset     Position in stack to read from
 * \param    numBytes   Number of bytes to read
 *
 * \return   gNvmErrNoError_c (0) if read was successfull
 */
#define flash_stack_read(dest, offset, numBytes) flash_getVar(dest, FLASH_STACK + (offset), numBytes)

#endif /* FLASH_H_ */

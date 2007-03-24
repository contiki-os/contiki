/*
 * Copyright (c) 2005, Swedish Institute of Computer Science
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
 * @(#)$Id: elfloader-arch.h,v 1.4 2007/03/24 15:21:34 oliverschmidt Exp $
 */
/**
 * \addtogroup elfloader
 * @{
 */

/**
 * \defgroup elfloaderarch Architecture specific functionality for the ELF loader.
 *
 * The architecture specific functionality for the Contiki ELF loader
 * has to be implemented for each processor type Contiki runs on.
 *
 * Since the ELF format is slightly different for different processor
 * types, the Contiki ELF loader is divided into two parts: the
 * generic ELF loader module (\ref elfloader) and the architecture
 * specific part (this module). The architecture specific part deals
 * with memory allocation, code and data relocation, and writing the
 * relocated ELF code into program memory.
 *
 * To port the Contiki ELF loader to a new processor type, this module
 * has to be implemented for the new processor type.
 *
 * @{
 */

/**
 * \file
 *         Header file for the architecture specific parts of the Contiki ELF loader.
 *
 * \author
 *         Adam Dunkels <adam@sics.se>
 *
 */

#ifndef __ELFLOADER_ARCH_H__
#define __ELFLOADER_ARCH_H__

#include "loader/elfloader.h"

/**
 * \brief      Allocate RAM for a new module.
 * \param size The size of the requested memory.
 * \return     A pointer to the allocated RAM
 *
 *             This function is called from the Contiki ELF loader to
 *             allocate RAM for the module to be loaded into.
 *
 * \bug        The Contiki ELF loader currently does not contain a
 *             mechanism for deallocating the memory allocated with
 *             this function.
 */
void *elfloader_arch_allocate_ram(int size);

/**
 * \brief      Allocate program memory for a new module.
 * \param size The size of the requested memory.
 * \return     A pointer to the allocated program memory
 *
 *             This function is called from the Contiki ELF loader to
 *             allocate program memory (typically ROM) for the module
 *             to be loaded into.
 *
 * \bug        The Contiki ELF loader currently does not contain a
 *             mechanism for deallocating the memory allocated with
 *             this function.
 */
void *elfloader_arch_allocate_rom(int size);

/**
 * \brief      Perform a relocation.
 * \param fd   The file descriptor for the ELF file.
 * \param sectionoffset The file offset at which the relocation can be found.
 * \param sectionaddr The section start address (absolute runtime).
 * \param rela A pointer to an ELF32 rela structure (struct elf32_rela).
 * \param addr The relocated address.
 *
 *             This function is called from the Contiki ELF loader to
 *             perform a relocation on a piece of code or data. The
 *             relocated address is calculated by the Contiki ELF
 *             loader, based on information in the ELF file, and it is
 *             the responsibility of this function to patch the
 *             executable code. The Contiki ELF loader passes a
 *             pointer to an ELF32 rela structure (struct elf32_rela)
 *             that contains information about how to patch the
 *             code. This information is different from processor to
 *             processor.
 */
void elfloader_arch_relocate(int fd, unsigned int sectionoffset,
			     char *sectionaddr,
			     struct elf32_rela *rela, char *addr);

/**
 * \brief      Write to read-only memory (for example the text segment).
 * \param fd   The file descriptor for the ELF file.
 * \param textoff	Offset of text segment relative start of file.
 * \param size The size of the text segment.
 * \param mem  A pointer to the where the text segment should be flashed
 *
 *             This function is called from the Contiki ELF loader to
 *             write the program code (text segment) of a loaded
 *             module into memory. The function is called when all
 *             relocations have been performed.
 */
void elfloader_arch_write_rom(int fd, unsigned short textoff, unsigned int size, char *mem);

#endif /* __ELFLOADER_ARCH_H__ */

/** @} */
/** @} */

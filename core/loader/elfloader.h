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
 */

/**
 * \file
 *         Header file for the Contiki ELF loader.
 * \author
 *         Adam Dunkels <adam@sics.se>
 *
 */

/**
 * \addtogroup loader
 * @{
 */

/**
 * \defgroup elfloader The Contiki ELF loader
 *
 * The Contiki ELF loader links, relocates, and loads ELF
 * (Executable Linkable Format) object files into a running Contiki
 * system.
 *
 * ELF is a standard format for relocatable object code and executable
 * files. ELF is the standard program format for Linux, Solaris, and
 * other operating systems.
 *
 * An ELF file contains either a standalone executable program or a
 * program module. The file contains both the program code, the
 * program data, as well as information about how to link, relocate,
 * and load the program into a running system.
 *
 * The ELF file is composed of a set of sections. The sections contain
 * program code, data, or relocation information, but can also contain
 * debugging information.
 *
 * To link and relocate an ELF file, the Contiki ELF loader first
 * parses the ELF file structure to find the appropriate ELF
 * sections. It then allocates memory for the program code and data in
 * ROM and RAM, respectively. After allocating memory, the Contiki ELF
 * loader starts relocating the code found in the ELF file.
 *
 * @{
 */

#ifndef ELFLOADER_H_
#define ELFLOADER_H_

#include "cfs/cfs.h"

/**
 * Return value from elfloader_load() indicating that loading worked.
 */
#define ELFLOADER_OK                  0
/**
 * Return value from elfloader_load() indicating that the ELF file had
 * a bad header.
 */
#define ELFLOADER_BAD_ELF_HEADER      1
/**
 * Return value from elfloader_load() indicating that no symbol table
 * could be found in the ELF file.
 */
#define ELFLOADER_NO_SYMTAB           2
/**
 * Return value from elfloader_load() indicating that no string table
 * could be found in the ELF file.
 */
#define ELFLOADER_NO_STRTAB           3
/**
 * Return value from elfloader_load() indicating that the size of the
 * .text segment was zero.
 */
#define ELFLOADER_NO_TEXT             4
/**
 * Return value from elfloader_load() indicating that a symbol
 * specific symbol could not be found.
 *
 * If this value is returned from elfloader_load(), the symbol has
 * been copied into the elfloader_unknown[] array.
 */
#define ELFLOADER_SYMBOL_NOT_FOUND    5
/**
 * Return value from elfloader_load() indicating that one of the
 * required segments (.data, .bss, or .text) could not be found.
 */
#define ELFLOADER_SEGMENT_NOT_FOUND   6
/**
 * Return value from elfloader_load() indicating that no starting
 * point could be found in the loaded module.
 */
#define ELFLOADER_NO_STARTPOINT       7

/**
 * elfloader initialization function.
 *
 * This function should be called at boot up to initialize the elfloader.
 */
void elfloader_init(void);

/**
 * \brief      Load and relocate an ELF file.
 * \param fd   An open CFS file descriptor.
 * \return     ELFLOADER_OK if loading and relocation worked.
 *             Otherwise an error value.
 *
 *             This function loads and relocates an ELF file. The ELF
 *             file must have been opened with cfs_open() prior to
 *             calling this function.
 *
 *             If the function is able to load the ELF file, a pointer
 *             to the process structure in the model is stored in the
 *             elfloader_loaded_process variable.
 *
 * \note       This function modifies the ELF file opened with cfs_open()!
 *             If the contents of the file is required to be intact,
 *             the file must be backed up first.
 *
 */
int elfloader_load(int fd);

/**
 * A pointer to the processes loaded with elfloader_load().
 */
extern struct process * const * elfloader_autostart_processes;

/**
 * If elfloader_load() could not find a specific symbol, it is copied
 * into this array.
 */
extern char elfloader_unknown[30];

#ifndef ELFLOADER_DATAMEMORY_SIZE
#ifdef ELFLOADER_CONF_DATAMEMORY_SIZE
#define ELFLOADER_DATAMEMORY_SIZE ELFLOADER_CONF_DATAMEMORY_SIZE
#else
#define ELFLOADER_DATAMEMORY_SIZE 0x100
#endif
#endif /* ELFLOADER_DATAMEMORY_SIZE */

#ifndef ELFLOADER_TEXTMEMORY_SIZE
#ifdef ELFLOADER_CONF_TEXTMEMORY_SIZE
#define ELFLOADER_TEXTMEMORY_SIZE ELFLOADER_CONF_TEXTMEMORY_SIZE
#else
#define ELFLOADER_TEXTMEMORY_SIZE 0x100
#endif
#endif /* ELFLOADER_TEXTMEMORY_SIZE */

typedef unsigned long  elf32_word;
typedef   signed long  elf32_sword;
typedef unsigned short elf32_half;
typedef unsigned long  elf32_off;
typedef unsigned long  elf32_addr;

struct elf32_rela {
  elf32_addr      r_offset;       /* Location to be relocated. */
  elf32_word      r_info;         /* Relocation type and symbol index. */
  elf32_sword     r_addend;       /* Addend. */
};


#endif /* ELFLOADER_H_ */

/** @} */
/** @} */

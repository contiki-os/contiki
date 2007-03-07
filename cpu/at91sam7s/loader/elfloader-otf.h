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

/**
 * \file
 *         Header file for the Contiki ELF loader.
 * \author
 *         Adam Dunkels <adam@sics.se>
 *	   Simon Berg <ksb@users.sourceforge.net>
 *
 */

/*
 * Copyright (c) 2005, Swedish Institute of Computer Science
 * Copyright (c) 2007, Simon Berg
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

#ifndef __ELFLOADER_H__
#define __ELFLOADER_H__

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
 * could be find in the ELF file.
 */
#define ELFLOADER_NO_SYMTAB           2
/**
 * Return value from elfloader_load() indicating that no string table
 * could be find in the ELF file.
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
 * Return value from elfloader_load() indicating that the ELF file contained
 * a relocation type that the implementation can't handle.
 */
#define ELFLOADER_UNHANDLED_RELOC     8

/**
 * Return value from elfloader_load() indicating that the offset for
 * a relative addressing mode was too big.
 */
#define ELFLOADER_OUTOF_RANGE	      9

/**
 * Return value from elfloader_load() indicating that the relocations
 * where not sorted by offset
 */
#define ELFLOADER_RELOC_NOT_SORTED    10

/**
 * Return value from elfloader_load() indicating that reading from the
 * ELF file failed in some way.
 */
#define ELFLOADER_INPUT_ERROR	      11

/**
 * Return value from elfloader_load() indicating that writing to a segment
 * failed.
 */
#define ELFLOADER_OUTPUT_ERROR	      12


#define ELFLOADER_SEG_TEXT 1
#define ELFLOADER_SEG_RODATA 2
#define ELFLOADER_SEG_DATA 3
#define ELFLOADER_SEG_BSS 4

/**
 * elfloader output object
 *
 * This object defines methods (callbacks) for writing the segments to memory.
 * It can be extended by the user to include any necessary state.
 */

struct elfloader_output {
  const struct elfloader_output_ops *ops;
};
/**
 * \brief	Allocate a new segment
 * \param input The output object
 * \param type	Type of segment
 * \param size	Size of segment in bytes
 * \return	A pointer to the start of the segment.
 *
 * The returned address doesn't need to correspond to any real memory,
 * since it's only used for calculating the relocations.
 */

void *elfloader_allocate_segment(struct elfloader_output *output,
				 unsigned int type, int size);

/**
 * \brief	Start writing to a new segment
 * \param input The output object
 * \param type	Type of segment
 * \param addr  Address of segment from elfloader_allocate_segment
 * \param size	Size of segment in bytes
 * \return	Returns ELFLOADER_OK if successful, otherwise an error code
 *
 */

int elfloader_start_segment(struct elfloader_output *output,
			      unsigned int type, void *addr, int size);
/**
 * \brief	Mark end of segment
 * \param input The output object
 * \return	Zero if successful
 */

int elfloader_end_segment(struct elfloader_output *output);

/**
 * \brief	Write data to a segment
 * \param input The output object
 * \param buf	Data to be written
 * \param len	Length of data
 * \return	The number of bytes actually written, or negative if failed.
 */

int elfloader_write_segment(struct elfloader_output *output, const char *buf,
			    unsigned int len);

/**
 * \brief	Get the current offset in the file where the next data will
 *		be written.
 * \param input The output object
 * \return	The current offset.
 */

unsigned int elfloader_segment_offset(struct elfloader_output *output);

#define elfloader_output_alloc_segment(output, type, size) \
((output)->ops->allocate_segment(output, type, size))

#define elfloader_output_start_segment(output, type, addr, size) \
((output)->ops->start_segment(output, type, addr, size))

#define elfloader_output_end_segment(output) \
((output)->ops->end_segment(output))

#define elfloader_output_write_segment(output, buf, len) \
((output)->ops->write_segment(output, buf, len))

#define elfloader_output_segment_offset(output) \
((output)->ops->segment_offset(output))


struct elfloader_output_ops {
  void * (*allocate_segment)(struct elfloader_output *output,
			   unsigned int type, int size);
  int (*start_segment)(struct elfloader_output *output,
		       unsigned int type, void *addr, int size);
  int (*end_segment)(struct elfloader_output *output);
  int (*write_segment)(struct elfloader_output *output, const char *buf,
		       unsigned int len);
  unsigned int (*segment_offset)(struct elfloader_output *output);
};


/**
 * elfloader initialization function.
 *
 * This function should be called at boot up to initilize the elfloader.
 */
void elfloader_init(void);

/**
 * \brief      Load and relocate an ELF file.
 * \param input Input object defining how to read from the ELF file
 * \param output Output object defining how to create and write to seegments.
 * \return     ELFLOADER_OK if loading and relocation worked.
 *             Otherwise an error value.
 *
 *             If the function is able to load the ELF file, a pointer
 *             to the process structure in the model is stored in the
 *             elfloader_loaded_process variable.
 *
 */
int elfloader_load(int input_fd,
		   struct elfloader_output *output);

/**
 * A pointer to the processes loaded with elfloader_load().
 */
extern struct process **elfloader_autostart_processes;

/**
 * If elfloader_load() could not find a specific symbol, it is copied
 * into this array.
 */
extern char elfloader_unknown[30];

#ifdef ELFLOADER_CONF_DATAMEMORY_SIZE
#define ELFLOADER_DATAMEMORY_SIZE ELFLOADER_CONF_DATAMEMORY_SIZE
#else
#define ELFLOADER_DATAMEMORY_SIZE 0x100
#endif

#ifdef ELFLOADER_CONF_TEXTMEMORY_SIZE
#define ELFLOADER_TEXTMEMORY_SIZE ELFLOADER_CONF_TEXTMEMORY_SIZE
#else
#define ELFLOADER_TEXTMEMORY_SIZE 0x100
#endif

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


#endif /* __ELFLOADER_H__ */

/** @} */
/** @} */

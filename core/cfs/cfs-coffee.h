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
 * \addtogroup cfs
 * @{
 */

#ifndef CFS_COFFEE_H
#define CFS_COFFEE_H

#include "cfs.h"

/**
 * Instruct Coffee that the access pattern to this file is adapted to 
 * flash I/O semantics by design, and Coffee should therefore not 
 * invoke its own micro logs when file modifications occur.
 *
 * This semantical I/O setting is useful when implementing flash storage
 * algorithms such as database indices on top of Coffee.
 *
 * \sa cfs_coffee_set_io_semantics()
 */
#define CFS_COFFEE_IO_FLASH_AWARE	0x1

/**
 * Instruct Coffee not to attempt to extend the file upon a request
 * to write past the reserved file size.
 *
 * A case when this is necessary is when the file has a firm size limit,
 * and a safeguard is needed to protect against writes beyond this limit.
 *
 * \sa cfs_coffee_set_io_semantics()
 */
#define CFS_COFFEE_IO_FIRM_SIZE		0x2

/**
 * Instruct Coffee to set unused bytes in the destination buffer to zero.
 * Trailing zeros may cause a wrong file size, this option ensures that
 * the corresponding bytes get set, so Coffee does not read unexpected data.
 *
 * \sa cfs_coffee_set_io_semantics()
 */
#define CFS_COFFEE_IO_ENSURE_READ_LENGTH		0x4

/**
 * \file
 *	Header for the Coffee file system.
 * \author
 * 	Nicolas Tsiftes <nvt@sics.se>
 *
 * \name Functions called from application programs
 * @{
 */

/**
 * \brief Reserve space for a file.
 * \param name The file name.
 * \param size The initial size to be reserved for the file.
 * \return 0 on success, -1 on failure.
 *
 * Coffee uses sequential page structures for files. The sequential 
 * structure can be reserved with a certain size. If a file has not 
 * been reserved when it is opened for the first time, it will be 
 * allocated with a default size.
 */
int cfs_coffee_reserve(const char *name, cfs_offset_t size);

/**
 * \brief Configure the on-demand log file.
 * \param file The file name.
 * \param log_size The total log file size.
 * \param log_entry_size The log entry size.
 * \return 0 on success, -1 on failure.
 *
 * When file data is first modified, Coffee creates a micro log for the
 * file. The micro log stores a table of modifications whose parameters --
 * the log size and the log entry size -- can be modified through the
 * cfs_coffee_configure_log function.
 */
int cfs_coffee_configure_log(const char *file, unsigned log_size,
                             unsigned log_entry_size);

/**
 * \brief Set the I/O semantics for accessing a file.
 *
 * \param fd The file descriptor through which the file is accessed.
 * \param flags A bit vector of flags.
 *
 * Coffee is used on a wide range of storage types, and the default 
 * I/O file semantics may not be optimal for the access pattern 
 * of a certain file. Hence, this function allows programmers to
 * switch the I/O semantics on a file that is accessed through a
 * particular file descriptor.
 *
 */
int cfs_coffee_set_io_semantics(int fd, unsigned flags);

/**
 * \brief Format the storage area assigned to Coffee.
 * \return 0 on success, -1 on failure.
 *
 * Coffee formats the underlying storage by setting all bits to zero.
 * Formatting must be done before using Coffee for the first time in
 * a mote.
 *
 * Notice that the erased bits may be set to 1 on the physical storage
 * when using flash memory. In this case, Coffee requires that the
 * COFFEE_READ and COFFEE_WRITE functions used to access the flash memory
 * invert all bits.
 */
int cfs_coffee_format(void);

/** @} */
/** @} */

#endif /* !COFFEE_H */

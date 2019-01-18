/**
 * \addtogroup c64fs
 * @{
 *
 */

/**
 * \file
 * C64 direct disk I/O.
 * \author Adam Dunkels <adam@dunkels.com>
 *
 */ 

/*
 * Copyright (c) 2003, Adam Dunkels.
 * All rights reserved. 
 *
 * Redistribution and use in source and binary forms, with or without 
 * modification, are permitted provided that the following conditions 
 * are met: 
 * 1. Redistributions of source code must retain the above copyright 
 *    notice, this list of conditions and the following disclaimer. 
 * 2. Redistributions in binary form must reproduce the above
 *    copyright notice, this list of conditions and the following
 *    disclaimer in the documentation and/or other materials provided
 *    with the distribution. 
 * 3. The name of the author may not be used to endorse or promote
 *    products derived from this software without specific prior
 *    written permission.  
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.  
 *
 * This file is part of the Contiki desktop OS
 *
 * $Id: c64-dio.c,v 1.1 2007/05/23 23:11:28 oliverschmidt Exp $
 *
 */

#include "c64-dio.h"
#include "c64-dio-asm.h"

/*-----------------------------------------------------------------------------------*/
/**
 * Read a block of data (256 bytes) from the disk.
 *
 * \param track The track of the disk block to be read.
 *
 * \param sector The sector of the disk block to be read.
 *
 * \param ptr A pointer to a buffer than must be able to accomodate
 * 256 bytes of data.
 *
 * \return An error code or C64_DIO_OK if the data was successfully
 * read.
 */
/*-----------------------------------------------------------------------------------*/
unsigned char
c64_dio_read_block(unsigned char track,
		   unsigned char sector,
		   unsigned char *ptr)
{
  c64_dio_asm_track = track;
  c64_dio_asm_sector = sector;
  c64_dio_asm_ptr = ptr;
  return c64_dio_asm_read_block();
}
/*-----------------------------------------------------------------------------------*/
/**
 * Write a block of data (256 bytes) to the disk.
 *
 * \param track The track of the disk block to be written.
 *
 * \param sector The sector of the disk block to be written.
 *
 * \param ptr A pointer to a buffer containing the 256 bytes of data
 * to be written.
 *
 * \return An error code or C64_DIO_OK if the data was successfully
 * written.
 */
/*-----------------------------------------------------------------------------------*/
unsigned char
c64_dio_write_block(unsigned char track,
		   unsigned char sector,
		   unsigned char *ptr)
{
  c64_dio_asm_track = track;
  c64_dio_asm_sector = sector;
  c64_dio_asm_ptr = ptr;
  return c64_dio_asm_write_block();
}
/*-----------------------------------------------------------------------------------*/
/**
 * Initialize the direct disk I/O routines for a particular disk drive.
 *
 * This function must be called before any of the other direct disk
 * I/O functions can be used.
 *
 * \param drive The drive number of the disk drive for which the
 * direct disk I/O should be enabled.
 */
/*-----------------------------------------------------------------------------------*/
void
c64_dio_init(unsigned char drive)
{
  c64_dio_asm_init(drive);
}
/*-----------------------------------------------------------------------------------*/
/** @} */

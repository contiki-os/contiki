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
 *         Header file for the CRC16 calculcation
 * \author
 *         Adam Dunkels <adam@sics.se>
 *
 */

/** \addtogroup lib
 * @{ */

/**
 * \defgroup crc16 Cyclic Redundancy Check 16 (CRC16) calculation
 *
 * The Cyclic Redundancy Check 16 is a hash function that produces a
 * checksum that is used to detect errors in transmissions. The CRC16
 * calculation module is an iterative CRC calculator that can be used
 * to cumulatively update a CRC checksum for every incoming byte.
 *
 * @{
 */

#ifndef CRC16_H_
#define CRC16_H_

/**
 * \brief      Update an accumulated CRC16 checksum with one byte.
 * \param b    The byte to be added to the checksum
 * \param crc  The accumulated CRC that is to be updated.
 * \return     The updated CRC checksum.
 *
 *             This function updates an accumulated CRC16 checksum
 *             with one byte. It can be used as a running checksum, or
 *             to checksum an entire data block.
 *
 *             \note The algorithm used in this implementation is
 *             tailored for a running checksum and does not perform as
 *             well as a table-driven algorithm when checksumming an
 *             entire data block.
 *
 */
unsigned short crc16_add(unsigned char b, unsigned short crc);

/**
 * \brief      Calculate the CRC16 over a data area
 * \param data Pointer to the data
 * \param datalen The length of the data
 * \param acc  The accumulated CRC that is to be updated (or zero).
 * \return     The CRC16 checksum.
 *
 *             This function calculates the CRC16 checksum of a data area.
 *
 *             \note The algorithm used in this implementation is
 *             tailored for a running checksum and does not perform as
 *             well as a table-driven algorithm when checksumming an
 *             entire data block.
 */
unsigned short crc16_data(const unsigned char *data, int datalen,
			  unsigned short acc);

#endif /* CRC16_H_ */

/** @} */
/** @} */

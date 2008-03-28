
/*
Copyright 2007, Freie Universitaet Berlin. All rights reserved.

These sources were developed at the Freie Universität Berlin, Computer
Systems and Telematics group.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are
met:

- Redistributions of source code must retain the above copyright
notice, this list of conditions and the following disclaimer.

- Redistributions in binary form must reproduce the above copyright
notice, this list of conditions and the following disclaimer in the
documentation and/or other materials provided with the distribution.
 
- Neither the name of Freie Universitaet Berlin (FUB) nor the names of its
contributors may be used to endorse or promote products derived from
this software without specific prior written permission.

This software is provided by FUB and the contributors on an "as is"
basis, without any representations or warranties of any kind, express
or implied including, but not limited to, representations or
warranties of non-infringement, merchantability or fitness for a
particular purpose. In no event shall FUB or contributors be liable
for any direct, indirect, incidental, special, exemplary, or
consequential damages (including, but not limited to, procurement of
substitute goods or services; loss of use, data, or profits; or
business interruption) however caused and on any theory of liability,
whether in contract, strict liability, or tort (including negligence
or otherwise) arising in any way out of the use of this software, even
if advised of the possibility of such damage.

This implementation was developed by the CST group at the FUB.

For documentation and questions please use the web site
http://scatterweb.mi.fu-berlin.de and the mailinglist
scatterweb@lists.spline.inf.fu-berlin.de (subscription via the Website).
Berlin, 2007
*/


/**
 * @file	ScatterWeb.Sd.cache.c
 * @ingroup	libsd
 * @brief	MMC-/SD-Card library, cached read and write
 * 
 * @author	Michael Baar	<baar@inf.fu-berlin.de>
 * @version	$Revision: 1.2 $
 *
 * $Id: sd_cache.c,v 1.2 2008/03/28 15:58:43 nvt-se Exp $
 */


/**
 * @addtogroup	libsd
 * @{
 */
#include "sd_internals.h"

#if SD_CACHE

void
_sd_cache_init()
{

  uint32_t adr = 0;


  sd_state.Cache->address = 1;

  sd_state.Cache->state = 0;

  // pre-read first block
  sd_cache_read_block(&adr);

  SD_FREE_LOCK(sd_state.Cache);

}

void
_sd_cache_flush()
{

#if SD_WRITE
  SD_GET_LOCK(sd_state.Cache);

  if (sd_state.Cache->state & SD_CACHE_DIRTY) {

    sd_set_blocklength(SD_WRITE_BLOCKLENGTH_BIT);

    sd_write_block(sd_state.Cache->address, sd_state.Cache->buffer);

    sd_state.Cache->state &= ~SD_CACHE_DIRTY;

  }

  SD_FREE_LOCK(sd_state.Cache);

#endif /* 
        */
}


sd_cache_t *
sd_cache_read_block(const uint32_t * pblAdr)
{

  SD_GET_LOCK(sd_state.Cache);

  if (sd_state.Cache->address != *pblAdr) {

    sd_set_blocklength(SD_WRITE_BLOCKLENGTH_BIT);

    if (sd_state.Cache->state & SD_CACHE_DIRTY) {

      sd_write_block(sd_state.Cache->address, sd_state.Cache->buffer);

      sd_state.Cache->state &= ~SD_CACHE_DIRTY;

    }

    sd_state.Cache->address = *pblAdr;

    if (!sd_read_block(sd_state.Cache->buffer, *pblAdr)) {

      SD_FREE_LOCK(sd_state.Cache);

      return false;

    }

  }

  return sd_state.Cache;

}


#if SD_READ_ANY
uint16_t
sd_read(void *pBuffer, uint32_t address, uint16_t size)
{

  uint16_t offset;		// bytes from aligned address to start of first byte to keep
  char *p;			// pointer to current pos in receive buffer
  uint16_t bytes_left;		// num bytes to read
  uint16_t read_count;		// num bytes to read from current block


  //
  // parameter processing
  //
  p = (char *)pBuffer;

  bytes_left = size;

  // align to block
  offset = sd_AlignAddress(&address);

  //
  // Data transfer
  //
  do {

    // calculate block
    if ((offset == 0) && (bytes_left >= sd_state.BlockLen)) {

      read_count = sd_state.BlockLen;

      sd_read_block(p, address);

    } else {

      sd_cache_read_block(&address);

      read_count = bytes_left + offset;

      if (read_count > sd_state.BlockLen)

	read_count = sd_state.BlockLen - offset;

      else

	read_count = bytes_left;


      memcpy(p, sd_state.Cache->buffer + offset, read_count);

      SD_FREE_LOCK(sd_state.Cache);

    }


    bytes_left -= read_count;

    if (bytes_left == 0)

      return size;


    p += read_count;

    address += sd_state.BlockLen;

  } while (1);

}


#endif // SD_READ_ANY

#if SD_WRITE
uint16_t
sd_write(uint32_t address, void *pBuffer, uint16_t size)
{

  uint16_t offset;		// bytes from aligned address to start of first byte to keep
  char *p;			// pointer to current pos in receive buffer
  uint16_t bytes_left;		// num bytes to read
  uint16_t read_count;		// num bytes to read from current block


  //
  // parameter processing
  //
  p = (char *)pBuffer;

  bytes_left = size;

  // align to block
  offset = sd_AlignAddress(&address);


  sd_set_blocklength(SD_WRITE_BLOCKLENGTH_BIT);

  //
  // Data transfer
  //
  do {

    // calculate block
    if ((offset == 0) && (bytes_left >= sd_state.BlockLen)) {

      read_count = sd_state.BlockLen;

      sd_write_block(address, p);

    } else {

      sd_cache_read_block(&address);

      read_count = bytes_left + offset;

      if (read_count > sd_state.BlockLen)

	read_count = sd_state.BlockLen - offset;

      else

	read_count = bytes_left;


      memcpy(sd_state.Cache->buffer + offset, p, read_count);

      sd_state.Cache->state |= SD_CACHE_DIRTY;


      SD_FREE_LOCK(sd_state.Cache);

    }


    if (bytes_left == 0)

      return size;


    p += read_count;

    bytes_left -= read_count;

    address += sd_state.BlockLen;

  } while (1);

}


#endif // SD_WRITE

#endif // SD_CACHE


/** @} */

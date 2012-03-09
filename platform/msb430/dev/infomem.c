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
 * @file		infomem.c
 * @addtogroup	storage
 * @brief		MSP430 Infomemory Storage
 * @author		Michael Baar	<baar@inf.fu-berlin.de>
 *
 * Functions to store and read data from the two infomemories (2 x 128 Bytes).
 * Offset addresses start at zero, size has a maximum of 128, write operations
 * across both blocks are not allowed.
 */
#include <string.h>
#include <stdarg.h>
#include "contiki-conf.h"
#include "infomem.h"

void
infomem_read(void *buffer, unsigned int offset, unsigned char size)
{
  uint8_t *address = (uint8_t *)INFOMEM_START + offset;
  memcpy(buffer, address, size);
}

bool
infomem_write(unsigned int offset, unsigned char count, ...)
{
  char backup[INFOMEM_BLOCK_SIZE];	
  uint8_t *buffer;
  uint16_t i;
  uint8_t *flash;
  va_list argp;
  uint16_t size;
  uint8_t *data;
  int s;

  if(offset > (2 * INFOMEM_BLOCK_SIZE)) {
    return FALSE;
  }

  flash = (uint8_t *)INFOMEM_START;

  s = splhigh();

  /* backup into RAM */
  memcpy(backup, flash, INFOMEM_BLOCK_SIZE);

  /* merge backup with new data */
  va_start(argp, count);

  buffer = (uint8_t *)backup + offset;
  for(i = 0; i < count; i++) {
    data = va_arg(argp, uint8_t *);
    size = va_arg(argp, uint16_t);
    memcpy(buffer, data, size);
    buffer += size;
  }

  va_end(argp);

  /* init flash access */
  FCTL2 = FWKEY + FSSEL1 + FN2;
  FCTL3 = FWKEY;

  /* erase flash */
  FCTL1 = FWKEY + ERASE;
  *flash = 0;

  /* write flash */
  FCTL1 = FWKEY + WRT;
  buffer = (uint8_t *)backup;
  for(i = 0; i < INFOMEM_BLOCK_SIZE; i++) {
    *flash++ = *buffer++;
  }

  FCTL1 = FWKEY;
  FCTL3 = FWKEY + LOCK;

  splx(s);

  return TRUE;
}

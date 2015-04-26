/*
 * Copyright (c) 2014, Texas Instruments Incorporated - http://www.ti.com/
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
 */
/*---------------------------------------------------------------------------*/
#ifndef __DATA_ENTRY_H
#define __DATA_ENTRY_H

#include <stdint.h>
#include "mailbox.h"

typedef struct rfc_dataEntry_s rfc_dataEntry_t;

#define _POSITION_dataEntry_pNextEntry                          0
#define _TYPE_dataEntry_pNextEntry                              uint8_t*
#define _POSITION_dataEntry_status                              4
#define _TYPE_dataEntry_status                                  uint8_t
#define _POSITION_dataEntry_config                              5
#define _TYPE_dataEntry_config                                  uint8_t
#define _BITPOS_dataEntry_config_type                           0
#define _NBITS_dataEntry_config_type                            2
#define _BITPOS_dataEntry_config_lenSz                          2
#define _NBITS_dataEntry_config_lenSz                           2
#define _BITPOS_dataEntry_config_irqIntv                        4
#define _NBITS_dataEntry_config_irqIntv                         4
#define _POSITION_dataEntry_length                              6
#define _TYPE_dataEntry_length                                  uint16_t
#define _POSITION_dataEntry_data                                8
#define _TYPE_dataEntry_data                                    uint8_t
#define _POSITION_dataEntry_pData                               8
#define _TYPE_dataEntry_pData                                   uint8_t*
#define _POSITION_dataEntry_numElements                         8
#define _TYPE_dataEntry_numElements                             uint16_t
#define _POSITION_dataEntry_pktStatus                           8
#define _TYPE_dataEntry_pktStatus                               uint16_t
#define _BITPOS_dataEntry_pktStatus_numElements                 0
#define _NBITS_dataEntry_pktStatus_numElements                  13
#define _BITPOS_dataEntry_pktStatus_bEntryOpen                  13
#define _NBITS_dataEntry_pktStatus_bEntryOpen                   1
#define _BITPOS_dataEntry_pktStatus_bFirstCont                  14
#define _NBITS_dataEntry_pktStatus_bFirstCont                   1
#define _BITPOS_dataEntry_pktStatus_bLastCont                   15
#define _NBITS_dataEntry_pktStatus_bLastCont                    1
#define _POSITION_dataEntry_nextIndex                           10
#define _TYPE_dataEntry_nextIndex                               uint16_t
#define _POSITION_dataEntry_rxData                              12
#define _TYPE_dataEntry_rxData                                  uint8_t
#define _LAST_POSITION_dataEntry                                12
#define _LAST_TYPE_dataEntry                                    uint8_t

struct rfc_dataEntry_s {
   uint8_t* pNextEntry;
   uint8_t status;
   struct {
      uint8_t type:2;
      uint8_t lenSz:2;
      uint8_t irqIntv:4;
   } config;
   uint16_t length;
   uint8_t data;
   uint8_t __dummy0;
   uint16_t nextIndex;
   uint8_t rxData;
};

#endif

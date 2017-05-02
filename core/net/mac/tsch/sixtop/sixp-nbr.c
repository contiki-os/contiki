/*
 * Copyright (c) 2016, Yasuyuki Tanaka
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
/**
 * \addtogroup sixtop
 * @{
 */
/**
 * \file
 *         Neighbor Management for 6top Protocol (6P)
 * \author
 *         Yasuyuki Tanaka <yasuyuki.tanaka@inf.ethz.ch>
 */

#include "contiki-lib.h"

#include "lib/assert.h"
#include "net/nbr-table.h"

#include "sixp.h"

#define DEBUG DEBUG_PRINT
#include "net/net-debug.h"

/**
 * \brief 6P Neighbor Data Structure (for internal use)
 *
 * XXX: for now, we have one nbr object per neighbor, which is shared with
 * multiple SFs. It's unclear whether we should use a different generation
 * counter for each SFs.
 */
typedef struct sixp_nbr {
  struct sixp_nbr *next;
  linkaddr_t addr;
  uint8_t next_seqno;
  uint8_t gtx;
  uint8_t grx;
} sixp_nbr_t;

static int advance_generation_counter(uint8_t *gc);

NBR_TABLE(sixp_nbr_t, sixp_nbrs);

/*---------------------------------------------------------------------------*/
static int
advance_generation_counter(uint8_t *gc)
{
  assert(gc != NULL);
  if(gc == NULL) {
    return -1;
  }
  switch(*gc) {
    case 0x00:
    case 0x02:
      *gc = 0x01;
      break;
    case 0x01:
      *gc = 0x02;
      break;
    default:
      /* unexpected condition */
      PRINTF("6P-nbr: advance_generation_counter() has unexpected gc %02x\n",
             *gc);
      return -1;
  }
  return 0;
}
/*---------------------------------------------------------------------------*/
sixp_nbr_t *
sixp_nbr_find(const linkaddr_t *addr)
{
  assert(addr != NULL);
  if(addr == NULL) {
    return NULL;
  }
  return (sixp_nbr_t *)nbr_table_get_from_lladdr(sixp_nbrs, addr);
}
/*---------------------------------------------------------------------------*/
sixp_nbr_t *
sixp_nbr_alloc(const linkaddr_t *addr)
{
  sixp_nbr_t *nbr;

  assert(addr != NULL);
  if(addr == NULL) {
    PRINTF("6P-nbr: sixp_nbr_alloc() fails because of invalid argument\n");
    return NULL;
  }

  if(sixp_nbr_find(addr) != NULL) {
    PRINTF("6P-nbr: sixp_nbr_alloc() fails because of duplication [peer_addr:");
    PRINTLLADDR((const uip_lladdr_t *)addr);
    PRINTF("]\n");
    return NULL;
  }

  if((nbr = (sixp_nbr_t *)nbr_table_add_lladdr(sixp_nbrs,
                                               addr,
                                               NBR_TABLE_REASON_SIXTOP,
                                               NULL)) == NULL) {
    PRINTF("6P-nbr: sixp_nbr_alloc() fails to add nbr because of no memory\n");
    return NULL;
  }

  linkaddr_copy(&nbr->addr, addr);
  nbr->next_seqno = SIXP_INITIAL_SEQUENCE_NUMBER;
  nbr->gtx = 0;
  nbr->grx = 0;

  return nbr;
}
/*---------------------------------------------------------------------------*/
void
sixp_nbr_free(sixp_nbr_t *nbr)
{
  assert(nbr != NULL);
  if(nbr != NULL) {
    (void)nbr_table_remove(sixp_nbrs, nbr);
  }
}
/*---------------------------------------------------------------------------*/
int16_t
sixp_nbr_get_gtx(sixp_nbr_t *nbr)
{
  assert(nbr != NULL);
  if(nbr == NULL) {
    PRINTF("6P-nbr: sixp_nbr_get_gtx() fails because of invalid argument\n");
    return -1;
  }
  return nbr->gtx;
}
/*---------------------------------------------------------------------------*/
int8_t
sixp_nbr_get_grx(sixp_nbr_t *nbr)
{
  assert(nbr != NULL);
  if(nbr == NULL) {
    PRINTF("6P-nbr: sixp_nbr_get_grx() fails because of invalid argument\n");
    return -1;
  }
  return nbr->grx;
}
/*---------------------------------------------------------------------------*/
int
sixp_nbr_advance_gtx(sixp_nbr_t *nbr)
{
  assert(nbr != NULL);
  if(nbr == NULL) {
    PRINTF("6P-nbr: sixp_nbr_advance_gtx() fails because of invalid arg\n");
    return -1;
  }
  return advance_generation_counter(&nbr->gtx);
}
/*---------------------------------------------------------------------------*/
int
sixp_nbr_advance_grx(sixp_nbr_t *nbr)
{
  assert(nbr != NULL);
  if(nbr == NULL) {
    PRINTF("6P-nbr: sixp_nbr_advance_grx() fails because of invalid arg\n");
    return -1;
  }
  return advance_generation_counter(&nbr->grx);
}
/*---------------------------------------------------------------------------*/
int
sixp_nbr_get_next_seqno(sixp_nbr_t *nbr)
{
  assert(nbr != NULL);
  if(nbr == NULL) {
    PRINTF("6P-nbr: sixp_nbr_get_next_seqno() fails because of invalid arg\n");
    return -1;
  }
  return nbr->next_seqno;
}
/*---------------------------------------------------------------------------*/
int
sixp_nbr_increment_next_seqno(sixp_nbr_t *nbr)
{
  assert(nbr != NULL);
  if(nbr == NULL) {
    PRINTF("6P-nbr: sixp_nbr_increment_next_seqno() fails; invalid arg\n");
    return -1;
  }
  nbr->next_seqno++;
  if(nbr->next_seqno > 0x0f) {
    /*
     * nbr->next_seqno, which represents a value of the SeqNum field, won't be
     * larger than 0x0f since he length of SeqNum field is 4 bits.
     */
    nbr->next_seqno = 0;
  }
  return 0;
}
/*---------------------------------------------------------------------------*/
int
sixp_nbr_init(void)
{
  sixp_nbr_t *nbr, *next_nbr;
  if(nbr_table_is_register(sixp_nbrs) == 0) {
    nbr_table_register(sixp_nbrs, NULL);
  } else {
    /* remove all the existing nbrs */
    nbr = (sixp_nbr_t *)nbr_table_head(sixp_nbrs);
    while(nbr != NULL) {
      next_nbr = (sixp_nbr_t *)nbr_table_next(sixp_nbrs, nbr);
      sixp_nbr_free(nbr);
      nbr = next_nbr;
    }
  }
  return 0;
}
/*---------------------------------------------------------------------------*/
/** @} */

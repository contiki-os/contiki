/**
 * \addtogroup rime
 * @{
 */

/**
 * \defgroup rimeaddr Rime addresses
 * @{
 *
 * The rimeaddr module is an abstract representation of addresses in
 * Rime.
 *
 */

/*
 * Copyright (c) 2007, Swedish Institute of Computer Science.
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
 *         Header file for the Rime address representation
 * \author
 *         Adam Dunkels <adam@sics.se>
 */

#ifndef __RIMEADDR_H__
#define __RIMEADDR_H__

#include "contiki-conf.h"

#ifdef RIMEADDR_CONF_SIZE
#define RIMEADDR_SIZE RIMEADDR_CONF_SIZE
#else /* RIMEADDR_SIZE */
#define RIMEADDR_SIZE 2
#endif /* RIMEADDR_SIZE */

#ifndef RIMEADDR_MEM_FUNC
// 0 is the default from the original contiki
// 1 uses string-functions. This is useful if the compiler supports optimizing these functions.
// 2 uses inline-functions. This is noramlly faster and smaller than the original solution. 
#define RIMEADDR_MEM_FUNC 0
#endif

#if RIMEADDR_MEM_FUNC == 1
#include <string.h>
#endif

#if RIMEADDR_MEM_FUNC != 0
# ifndef RIMEADDR_INLINE
#  if __GNUC__ && !__GNUC_STDC_INLINE__
#   define RIMEADDR_INLINE extern inline
#  else
#   define RIMEADDR_INLINE inline
#  endif
# endif
#endif



typedef union {
  unsigned char u8[RIMEADDR_SIZE];
} rimeaddr_t;


/**
 * \brief      The Rime address of the node
 *
 *             This variable contains the Rime address of the
 *             node. This variable should not be changed directly;
 *             rather, the rimeaddr_set_node_addr() function should be
 *             used.
 *
 */
extern rimeaddr_t rimeaddr_node_addr;

/**
 * \brief      The null Rime address
 *
 *             This variable contains the null Rime address. The null
 *             address is used in route tables to indicate that the
 *             table entry is unused. Nodes with no configured address
 *             has the null address. Nodes with their node address set
 *             to the null address will have problems communicating
 *             with other nodes.
 *
 */
extern const rimeaddr_t rimeaddr_null;



/**
 * \brief      Copy a Rime address
 * \param dest The destination
 * \param from The source
 *
 *             This function copies a Rime address from one location
 *             to another.
 *
 */
#if ! RIMEADDR_MEM_FUNC
void rimeaddr_copy(rimeaddr_t *dest, const rimeaddr_t *from);
#elif RIMEADDR_MEM_FUNC == 1
RIMEADDR_INLINE void rimeaddr_copy(rimeaddr_t *dest, const rimeaddr_t *from)
{
	memcpy((char *)dest, (char *)from, RIMEADDR_SIZE);
}
#else
RIMEADDR_INLINE void rimeaddr_copy(rimeaddr_t *dest, const rimeaddr_t *src)
{
  u8_t i;
  for(i = 0; i < RIMEADDR_SIZE; i++) {
    dest->u8[i] = src->u8[i];
  }
}
#endif

/**
 * \brief      Compare two Rime addresses
 * \param addr1 The first address
 * \param addr2 The second address
 * \return     Non-zero if the addresses are the same, zero if they are different
 *
 *             This function compares two Rime addresses and returns
 *             the result of the comparison. The function acts like
 *             the '==' operator and returns non-zero if the addresses
 *             are the same, and zero if the addresses are different.
 *
 */

#if ! RIMEADDR_MEM_FUNC
int rimeaddr_cmp(const rimeaddr_t *addr1, const rimeaddr_t *addr2);
#elif RIMEADDR_MEM_FUNC == 1
RIMEADDR_INLINE int rimeaddr_cmp(const rimeaddr_t *addr1, const rimeaddr_t *addr2)
{
	return !memcmp((char *)addr1, (char *)addr2, RIMEADDR_SIZE);
}
#else
RIMEADDR_INLINE int rimeaddr_cmp(const rimeaddr_t *addr1, const rimeaddr_t *addr2)
{
  u8_t i;
  for(i = 0; i < RIMEADDR_SIZE; i++) {
    if(addr1->u8[i] != addr2->u8[i]) {
      return 0;
    }
  }
  return 1;
}
#endif


/**
 * \brief      Set the address of the current node
 * \param addr The address
 *
 *             This function sets the Rime address of the node.
 *
 */
#if ! RIMEADDR_MEM_FUNC
void rimeaddr_set_node_addr(rimeaddr_t *addr);
#else
RIMEADDR_INLINE void rimeaddr_set_node_addr(rimeaddr_t *t)
{
  rimeaddr_copy(&rimeaddr_node_addr, t);
}
#endif




#endif /* __RIMEADDR_H__ */
/** @} */
/** @} */

/*
 * Copyright (c) 2012, Thingsquare, http://www.thingsquare.com/.
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
 *
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
 *
 */
#include "ip64-addrmap.h"

#include "lib/memb.h"
#include "lib/list.h"

#include "ip64-conf.h"

#include "lib/random.h"

#include <string.h>

#ifdef IP64_ADDRMAP_CONF_ENTRIES
#define NUM_ENTRIES IP64_ADDRMAP_CONF_ENTRIES
#else /* IP64_ADDRMAP_CONF_ENTRIES */
#define NUM_ENTRIES 32
#endif /* IP64_ADDRMAP_CONF_ENTRIES */

MEMB(entrymemb, struct ip64_addrmap_entry, NUM_ENTRIES);
LIST(entrylist);

#define FIRST_MAPPED_PORT 10000
#define LAST_MAPPED_PORT  20000
static uint16_t mapped_port = FIRST_MAPPED_PORT;

#define printf(...)

/*---------------------------------------------------------------------------*/
struct ip64_addrmap_entry *
ip64_addrmap_list(void)
{
  return list_head(entrylist);
}
/*---------------------------------------------------------------------------*/
void
ip64_addrmap_init(void)
{
  memb_init(&entrymemb);
  list_init(entrylist);
  mapped_port = FIRST_MAPPED_PORT;
}
/*---------------------------------------------------------------------------*/
static void
check_age(void)
{
  struct ip64_addrmap_entry *m;

  /* Walk through the list of address mappings, throw away the ones
     that are too old. */
  m = list_head(entrylist);
  while(m != NULL) {
    if(timer_expired(&m->timer)) {
      list_remove(entrylist, m);
      memb_free(&entrymemb, m);
      m = list_head(entrylist);
    } else {
      m = list_item_next(m);
    }
  }
}
/*---------------------------------------------------------------------------*/
static int
recycle(void)
{
  /* Find the oldest recyclable mapping and remove it. */
  struct ip64_addrmap_entry *m, *oldest;

  /* Walk through the list of address mappings, throw away the ones
     that are too old. */

  oldest = NULL;
  for(m = list_head(entrylist);
      m != NULL;
      m = list_item_next(m)) {
    if(m->flags & FLAGS_RECYCLABLE) {
      if(oldest == NULL) {
        oldest = m;
      } else {
        if(timer_remaining(&m->timer) <
           timer_remaining(&oldest->timer)) {
          oldest = m;
        }
      }
    }
  }

  /* If we found an oldest recyclable entry, remove it and return
     non-zero. */
  if(oldest != NULL) {
    list_remove(entrylist, oldest);
    memb_free(&entrymemb, oldest);
    return 1;
  }

  return 0;
}
/*---------------------------------------------------------------------------*/
struct ip64_addrmap_entry *
ip64_addrmap_lookup(const uip_ip6addr_t *ip6addr,
		    uint16_t ip6port,
		    const uip_ip4addr_t *ip4addr,
		    uint16_t ip4port,
		    uint8_t protocol)
{
  struct ip64_addrmap_entry *m;

  printf("lookup ip4port %d ip6port %d\n", uip_htons(ip4port),
	 uip_htons(ip6port));
  check_age();
  for(m = list_head(entrylist); m != NULL; m = list_item_next(m)) {
    printf("protocol %d %d, ip4port %d %d, ip6port %d %d, ip4 %d ip6 %d\n",
	   m->protocol, protocol,
	   m->ip4port, ip4port,
	   m->ip6port, ip6port,
	   uip_ip4addr_cmp(&m->ip4addr, ip4addr),
	   uip_ip6addr_cmp(&m->ip6addr, ip6addr));
    if(m->protocol == protocol &&
       m->ip4port == ip4port &&
       m->ip6port == ip6port &&
       uip_ip4addr_cmp(&m->ip4addr, ip4addr) &&
       uip_ip6addr_cmp(&m->ip6addr, ip6addr)) {
      m->ip6to4++;
      return m;
    }
  }
  return NULL;
}
/*---------------------------------------------------------------------------*/
struct ip64_addrmap_entry *
ip64_addrmap_lookup_port(uint16_t mapped_port, uint8_t protocol)
{
  struct ip64_addrmap_entry *m;

  check_age();
  for(m = list_head(entrylist); m != NULL; m = list_item_next(m)) {
    printf("mapped port %d %d, protocol %d %d\n",
	   m->mapped_port, mapped_port,
	   m->protocol, protocol);
    if(m->mapped_port == mapped_port &&
       m->protocol == protocol) {
      m->ip4to6++;
      return m;
    }
  }
  return NULL;
}
/*---------------------------------------------------------------------------*/
static void
increase_mapped_port(void)
{
  mapped_port = (random_rand() % (LAST_MAPPED_PORT - FIRST_MAPPED_PORT)) +
    FIRST_MAPPED_PORT;
}
/*---------------------------------------------------------------------------*/
struct ip64_addrmap_entry *
ip64_addrmap_create(const uip_ip6addr_t *ip6addr,
		    uint16_t ip6port,
		    const uip_ip4addr_t *ip4addr,
		    uint16_t ip4port,
		    uint8_t protocol)
{
  struct ip64_addrmap_entry *m;

  check_age();
  m = memb_alloc(&entrymemb);
  if(m == NULL) {
    /* We could not allocate an entry, try to recycle one and try to
       allocate again. */
    if(recycle()) {
      m = memb_alloc(&entrymemb);
    }
  }
  if(m != NULL) {
    uip_ip4addr_copy(&m->ip4addr, ip4addr);
    m->ip4port = ip4port;
    uip_ip6addr_copy(&m->ip6addr, ip6addr);
    m->ip6port = ip6port;
    m->protocol = protocol;
    m->flags = FLAGS_NONE;
    m->ip6to4 = 1;
    m->ip4to6 = 0;
    timer_set(&m->timer, 0);

    /* Pick a new, unused local port. First make sure that the
       mapped_port number does not belong to any active connection. If
       so, we keep increasing the mapped_port until we're free. */
    {
      struct ip64_addrmap_entry *n;
      n = list_head(entrylist);
      while(n != NULL) {
	if(n->mapped_port == mapped_port) {
	  increase_mapped_port();
	  n = list_head(entrylist);
	} else {
	  n = list_item_next(m);
	}
      }
    }
    m->mapped_port = mapped_port;
    increase_mapped_port();

    list_add(entrylist, m);
    return m;
  }
  return NULL;
}
/*---------------------------------------------------------------------------*/
void
ip64_addrmap_set_lifetime(struct ip64_addrmap_entry *e,
                          clock_time_t time)
{
  if(e != NULL) {
    timer_set(&e->timer, time);
  }
}
/*---------------------------------------------------------------------------*/
void
ip64_addrmap_set_recycleble(struct ip64_addrmap_entry *e)
{
  if(e != NULL) {
    e->flags |= FLAGS_RECYCLABLE;
  }
}
/*---------------------------------------------------------------------------*/

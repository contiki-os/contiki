/*
 * Copyright (c) 2010, Vrije Universiteit Brussel
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
 *
 * Author: Joris Borms <joris.borms@vub.ac.be>
 *
 */

#include "contiki.h"

#include "lib/memb.h"
#include "lib/list.h"
#include <stddef.h>
#include <string.h>

#include "net/neighbor-attr.h"

#define DEBUG 0

#if DEBUG
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif

static uint16_t timeout = 0;

MEMB(neighbor_addr_mem, struct neighbor_addr, MAX_NEIGHBORS);

LIST(neighbor_addrs);
LIST(neighbor_attrs);
/*---------------------------------------------------------------------------*/
static struct neighbor_addr *
neighbor_addr_get(const rimeaddr_t *addr)
{
  /* check if addr is derived from table, inside memb */
  if(memb_inmemb(&neighbor_addr_mem, (struct queuebuf *)addr)) {
    return (struct neighbor_addr *)
        (((void *)addr) - offsetof(struct neighbor_addr, addr));
  }
  struct neighbor_addr *item = list_head(neighbor_addrs);

  while(item != NULL) {
    if(rimeaddr_cmp(addr, &item->addr)) {
      return item;
    }
    item = item->next;
  }
  return NULL;
}
/*---------------------------------------------------------------------------*/
struct neighbor_addr *
neighbor_attr_list_neighbors(void)
{
  return list_head(neighbor_addrs);
}
/*---------------------------------------------------------------------------*/
int
neighbor_attr_register(struct neighbor_attr *def)
{
  list_push(neighbor_attrs, def);
  /* set default values for already existing neighbors */
  struct neighbor_addr *addr = list_head(neighbor_addrs);

  while(addr != NULL) {
    if(def->default_value != NULL) {
      memcpy(def->data + addr->index * def->size, def->default_value,
             def->size);
    } else {
      /* fill with zeroes */
      memset(def->data + addr->index * def->size, 0, def->size);
    }
    addr = addr->next;
  }
  return 1;
}
/*---------------------------------------------------------------------------*/
int
neighbor_attr_has_neighbor(const rimeaddr_t *addr)
{
  return neighbor_addr_get(addr) != NULL;
}
/*---------------------------------------------------------------------------*/
int
neighbor_attr_add_neighbor(const rimeaddr_t *addr)
{
  if(neighbor_attr_has_neighbor(addr)) {
    return 0;
  }

  struct neighbor_addr *item = memb_alloc(&neighbor_addr_mem);

  if(item == NULL) {
    return -1;
  }

  list_push(neighbor_addrs, item);

  item->time = 0;
  rimeaddr_copy(&item->addr, addr);

  /* look up index and set default values */
  uint16_t i;
  struct neighbor_addr *ptr = neighbor_addr_mem.mem;

  for(i = 0; i < neighbor_addr_mem.num; ++i) {
    if(&ptr[i] == item) {
      break;
    }
  }

  item->index = i;

  struct neighbor_attr *def = list_head(neighbor_attrs);

  while(def != NULL) {
    if(def->default_value != NULL) {
      memcpy(def->data + i * def->size, def->default_value, def->size);
    } else {
      /* fill with zeroes */
      memset(def->data + i * def->size, 0, def->size);
    }
    def = def->next;
  }

  return 1;
}
/*---------------------------------------------------------------------------*/
void *
neighbor_attr_get_data(struct neighbor_attr *def, const rimeaddr_t *addr)
{
  struct neighbor_addr *attr = neighbor_addr_get(addr);

  if(attr != NULL) {
    return (void *)(def->data + attr->index * def->size);
  }
  return NULL;
}
/*---------------------------------------------------------------------------*/
int
neighbor_attr_set_data(struct neighbor_attr *def, const rimeaddr_t *addr,
                       void *data)
{
  struct neighbor_addr *attr = neighbor_addr_get(addr);

  if(attr == NULL) {
    if(neighbor_attr_add_neighbor(addr)) {
      attr = neighbor_addr_get(addr);
    }
  }
  if(attr != NULL) {
    attr->time = 0;
    memcpy((void *)(def->data + attr->index * def->size), data, def->size);
    return 1;
  }
  return 0;
}
/*---------------------------------------------------------------------------*/
void
neighbor_attr_tick(const rimeaddr_t * addr)
{
  struct neighbor_addr *attr = neighbor_addr_get(addr);

  if(attr != NULL) {
    attr->time = 0;
  }
}
/*---------------------------------------------------------------------------*/
uint16_t
neighbor_attr_get_timeout(void)
{
  return timeout;
}
/*---------------------------------------------------------------------------*/
static struct ctimer ct;

#define TIMEOUT_SECONDS 5
static void
timeout_check(void *ptr)
{
  if(timeout > 0) {
    struct neighbor_addr *item = neighbor_attr_list_neighbors();

    while(item != NULL) {
      item->time += TIMEOUT_SECONDS;
      if(item->time >= timeout) {
        struct neighbor_addr *next_item = item->next;

        memb_free(&neighbor_addr_mem, item);
        list_remove(neighbor_addrs, item);
        item = next_item;
      } else {
        item = item->next;
      }
    }
    ctimer_set(&ct, TIMEOUT_SECONDS * CLOCK_SECOND, timeout_check, ptr);
  }
}
/*---------------------------------------------------------------------------*/
void
neighbor_attr_set_timeout(uint16_t time)
{
  if(timeout == 0 && time > 0) {
    ctimer_set(&ct, TIMEOUT_SECONDS * CLOCK_SECOND, timeout_check, NULL);
  } else if(timeout > 0 && time == 0) {
    ctimer_stop(&ct);
  }
  timeout = time;
}
/*---------------------------------------------------------------------------*/

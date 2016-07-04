/*
 * Copyright (c) 2006, Swedish Institute of Computer Science.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in the
 *   documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Institute nor the names of its contributors
 *   may be used to endorse or promote products derived from this software
 *   without specific prior written permission.
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
 */

/**
 * \addtogroup uip6
 * @{
 */

/**
 * \file
 *    IPv6 address registration list manuiplation
 * \author Mohamed Seliem <mseliem11@gmail.com>
 * \author Khaled Elsayed <khaled@ieee.org> 
 * \author Ahmed Khattab <ahmed.khattab@gmail.com> 
 *
 */
#if UIP_CONF_IPV6_LOWPAN_ND
#include <string.h>
#include <stdlib.h>
#include <stddef.h>
#include "lib/random.h"
#include "net/ipv6/uip-6lowpan-nd6.h"
#include "net/ipv6/uip-ds6.h"
#include "net/ipv6/multicast/uip-mcast6.h"
#include "net/ip/uip-packetqueue.h"
#include "net/ipv6/uip-ds6-reg.h"

#define DEBUG DEBUG_NONE
#include "net/ip/uip-debug.h"
uip_ds6_reg_t uip_ds6_reg_list[UIP_DS6_REG_LIST_SIZE];            /**< Registrations list */

static uip_ds6_reg_t *locreg;
static uip_ds6_defrt_t *min_defrt; /* default router with minimum lifetime */ 
static unsigned long min_lifetime; /* minimum lifetime */ 

/*---------------------------------------------------------------------------*/
uip_ds6_reg_t*
uip_ds6_reg_add(uip_ds6_addr_t* addr, uip_ds6_defrt_t* defrt, uint8_t state, uint16_t lifetime) {

        uip_ds6_reg_t* candidate = NULL;

        for (locreg = uip_ds6_reg_list;
                        locreg < uip_ds6_reg_list + UIP_DS6_REG_LIST_SIZE; locreg++) {
                if (!locreg->isused) {
                        candidate = locreg;
                        break;
                } else if (locreg->state == REG_GARBAGE_COLLECTIBLE) {
                        candidate = locreg;
                }
        }
        /* If there was an entry not in use, use it; otherwise overwrite
         * our canditate entry in Garbage-collectible state*/
        if (candidate != NULL) {
                candidate->isused = 1;
                candidate->addr = addr;
                candidate->defrt = defrt;
                candidate->state = state;
                timer_set(&candidate->registration_timer, 0);
        	candidate->reg_count = 0;
                if(candidate->state == REG_GARBAGE_COLLECTIBLE) {
                        stimer_set(&candidate->reg_lifetime, UIP_DS6_GARBAGE_COLLECTIBLE_REG_LIFETIME);
                } else if (candidate->state == REG_TENTATIVE) {
                        stimer_set(&candidate->reg_lifetime, UIP_DS6_TENTATIVE_REG_LIFETIME);
                }
		else{
			stimer_set(&candidate->reg_lifetime, lifetime);
			}
                defrt->registrations++;
                return candidate;
        }
        return NULL;
}
/*---------------------------------------------------------------------------*/
void
uip_ds6_reg_rm(uip_ds6_reg_t* reg){

        reg->defrt->registrations--;
        reg->isused = 0;

}

/*---------------------------------------------------------------------------*/
uip_ds6_reg_t*
uip_ds6_reg_lookup(uip_ds6_addr_t* addr, uip_ds6_defrt_t* defrt){

        uip_ds6_reg_t* reg;

        for (reg = uip_ds6_reg_list;
                        reg < uip_ds6_reg_list + UIP_DS6_REG_LIST_SIZE; reg++) {
                if ((reg->isused) && (reg->addr == addr) && (reg->defrt == defrt)) {
                        return reg;
                }
        }
        return NULL;
}
/*---------------------------------------------------------------------------*/
void
uip_ds6_reg_cleanup_defrt(uip_ds6_defrt_t* defrt) {

        uip_ds6_reg_t* reg;

        for (reg = uip_ds6_reg_list;
                        reg < uip_ds6_reg_list + UIP_DS6_REG_LIST_SIZE; reg++) {
                if ((reg->isused) && (reg->defrt == defrt)) {
                        uip_ds6_reg_rm(reg);
                }
        }
}

/*---------------------------------------------------------------------------*/
void
uip_ds6_reg_cleanup_addr(uip_ds6_addr_t* addr) {

        uip_ds6_reg_t* reg;

        for (reg = uip_ds6_reg_list;
                        reg < uip_ds6_reg_list + UIP_DS6_REG_LIST_SIZE; reg++) {
                if ((reg->isused) && (reg->addr == addr)) {
                        if (reg->state != REG_REGISTERED) {
                                uip_ds6_reg_rm(reg);
                        } else {
                                /* Mark it as TO_BE_UNREGISTERED */
                                reg->state = REG_TO_BE_UNREGISTERED;
                        }
                }
        }
}

/*---------------------------------------------------------------------------*/

uint8_t uip_ds6_get_registrations(uip_ds6_defrt_t *defrt) {

        if ((defrt == NULL) || (!defrt->isused)) {
                return 0;
        }

        return defrt->registrations;
}

/*---------------------------------------------------------------------------*/
uip_ds6_defrt_t*
uip_ds6_defrt_choose_min_reg(uip_ds6_addr_t* addr)
{
        uint8_t min = 0;
        uip_ds6_defrt_t* min_defrt = NULL;

  for(locdefrt = uip_ds6_defrt_list;
      locdefrt < uip_ds6_defrt_list + UIP_DS6_DEFRT_NB; locdefrt++) {
    if (locdefrt->isused) {
        if (NULL == uip_ds6_reg_lookup(addr, locdefrt)) {
              if ((min_defrt == NULL) ||
                        ((min_defrt != NULL) && (uip_ds6_get_registrations(locdefrt) < min))) {
                min_defrt = locdefrt;
                min = uip_ds6_get_registrations(locdefrt);
                if (min == 0) {
                        /* We are not going to find a better candidate */
                        return min_defrt;
                }
              }
        }
    }
  }
  return min_defrt;
}
#endif

/** @}*/

/*
 * Copyright (c) 2013, Swedish Institute of Computer Science.
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
 */

/**
 * \addtogroup uip6
 * @{
 */

/**
 * \file
 *    IPv6 address registration list
 * \author Mohamed Seliem <mseliem11@gmail.com>
 * \author Khaled Elsayed <khaled@ieee.org> 
 * \author Ahmed Khattab <ahmed.khattab@gmail.com> *
 */
#ifndef UIP_DS6_REG_H_
#define UIP_DS6_REG_H_

#include "net/ip/uip.h"
#include "net/nbr-table.h"
#include "sys/stimer.h"
#include "net/ipv6/uip-ds6.h"
#include "net/ipv6/uip-ds6-nbr.h"
#include "net/ipv6/uip-ds6-route.h"

#if UIP_CONF_IPV6_QUEUE_PKT
#include "net/ip/uip-packetqueue.h"
#endif                          /*UIP_CONF_QUEUE_PKT */

/* The host SHOULD start sending Router Solicitations "well before the
 * minimum of those lifetimes" (across all the prefixes and all the
 * contexts) expire. RFC 6775. We define thus a threshold
 * value to start sending RS messages (in seconds).*/
#ifdef  UIP_DS6_CONF_LIFETIME_THRESHOLD
#define UIP_DS6_LIFETIME_THRESHOLD UIP_DS6_CONF_LIFETIME_THRESHOLD
#else
#define UIP_DS6_LIFETIME_THRESHOLD 60
#endif

/* 6lowpan-nd default lifetimes (in seconds)*/
#ifdef UIP_DS6_GARBAGE_COLLECTIBLE_REG_LIFETIME
#define UIP_DS6_GARBAGE_COLLECTIBLE_REG_LIFETIME UIP_DS6_GARBAGE_COLLECTIBLE_REG_LIFETIME
#else
#define UIP_DS6_GARBAGE_COLLECTIBLE_REG_LIFETIME 20
#endif
#ifdef UIP_DS6_TENTATIVE_REG_LIFETIME
#define UIP_DS6_TENTATIVE_REG_LIFETIME UIP_DS6_TENTATIVE_REG_LIFETIME
#else
#define UIP_DS6_TENTATIVE_REG_LIFETIME 20 /* Default value in RFC 6775*/
#endif

/** \brief Possible states for the nbr cache entries,
 * if 6lowpan-nd is used, new states are defined (new states are
 * orthogonal to those defined in rfc4861) */
#define  REG_GARBAGE_COLLECTIBLE 0
#define  REG_TENTATIVE 1
#define  REG_REGISTERED 2
#define  REG_TO_BE_UNREGISTERED 3 /* Auxiliary registration entry state */

/*registration list*/
#ifdef UIP_DS6_CONF_REGS_PER_ADDR
#define UIP_DS6_REGS_PER_ADDR UID_DS6_CONF_REGS_PER_ADDR
#else
#define UIP_DS6_REGS_PER_ADDR UIP_DS6_NBR_NB
#endif
#define UIP_DS6_REG_LIST_SIZE UIP_DS6_REGS_PER_ADDR * UIP_DS6_ADDR_NB

/* Structure to handle 6lowpan-nd registrations */
typedef struct uip_ds6_reg {
  uint8_t isused;
  uint8_t state;
  uip_ds6_addr_t* addr;
  uip_ds6_defrt_t* defrt;
  struct stimer reg_lifetime;
  struct timer registration_timer;
  uint8_t reg_count;
} uip_ds6_reg_t;

/**
 * \brief Adds a registration to the registrations list. It also
 *increases the value of the number of registrations of
 *the corresponding default router.
 *
 * \param addr The address for which we are adding the registration
 * \param defrt The default router with which we are registering the address
 * \param state The state of the registration we are adding (defined in RFC 6775-6lowpan-nd).
 * \param lifetime registration life time
 *
 * \return A pointer to the newly create registration if the
 *registration was successfull, otherwise NULL.
 */
uip_ds6_reg_t* uip_ds6_reg_add(uip_ds6_addr_t* addr, uip_ds6_defrt_t* defrt, uint8_t state, uint16_t lifetime);

/**
 * \brief Removes a registration from the registrations list. It also
 * decreases the value of the number of registrations of
 * the corresponding default router.
 *
 * \param reg The registration to be deleted.
 */

void uip_ds6_reg_rm(uip_ds6_reg_t* reg);
/**
 * \brief Looks for a registration in the registrations list.
 * \param addr The address whose registration we are looking for.
 * \param defrt The default router with which the address is registered.
 *
 * \returns reg The registration matching the search.
 * NULL if there are no matches.
 */
uip_ds6_reg_t *uip_ds6_reg_lookup(uip_ds6_addr_t* addr, uip_ds6_defrt_t* defrt);

void uip_ds6_reg_update(uip_ds6_addr_t* addr, uip_ds6_defrt_t* defrt, uint16_t lifetime);

/**
 * \brief Removes all registrations with defrt from the registration
 * list.
 *
 * \param defrt The router whose registrations we want to remove.
 *
 */
void uip_ds6_reg_cleanup_defrt(uip_ds6_defrt_t* defrt);
/**
 * \brief Removes all resgitrations of address addr from the
 * registration list. If the registration is in REGISTERED
 * state, we can not just delete it, but we MUST first send
 * a NS with ARO lifetime = 0. As there may be more than one,
 * we mark it as TO_BE_UNREGISTERED so uip_ds6_periodic can
 * process them properly.
 *
 * \param addr The address whose registrationes we want to remove.
 *
 */
void uip_ds6_reg_cleanup_addr(uip_ds6_addr_t* addr);
/**
 * \brief Returns the number of addresses that are registered (or
 *                                                              pending to be registered) with a router.
 *
 * \param defrt The router whose number of registrations we want to check.
 *
 * \returns The number of addresses registered (or pending to be
 *                                                              registered) with defrt.
 */
uint8_t uip_ds6_get_registrations(uip_ds6_defrt_t *defrt);
/**
 * \brief Returns a default router that meets:
 *    - has the minimum number of registrations
 *    - addr is not registered with it
 * \param addr The address whose registrationes we want to chose.
 */
uip_ds6_defrt_t* uip_ds6_defrt_choose_min_reg(uip_ds6_addr_t* addr);

#endif /* UIP_DS6_REG_H_ */
/** @} */

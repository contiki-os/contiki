/**
 * \addtogroup uip6
 * @{
 */

/**
 * \file
 *         MLDv1 multicast registration handling (RFC 2710)
 * \author Phoebe Buckheister	<phoebe.buckheister@itwm.fhg.de> 
 */

/*
 * Copyright (c) 2014, Fraunhofer ITWM
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 
 * 1. Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 * 
 * 2. Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
 * IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 * PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 * TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef MLD_H_
#define MLD_H_

#include "contiki-conf.h"

#if UIP_CONF_MLD

#include <stdbool.h>

#include "net/uip.h"
#include "net/uip-ds6.h"

/** \name MLDv1 message processing and multicast listener reporting */
/** @{ */
/** \brief Report timeout for multicast adresses, in seconds.
 */
#define UIP_IP6_MLD_REPORT_INTERVAL 10

/**
 * \brief report the given address at the next possible point in time.
 */
void
uip_icmp6_mldv1_schedule_report(uip_ds6_maddr_t * addr);

/**
 * \brief Send an MLDv1 listener report for the specified maddr
 * \param addr Address to send a report for
 */
void
uip_icmp6_mldv1_report(uip_ip6addr_t * addr);

/**
 * \brief Send an MLDv1 listener done for the specified maddr
 * \param addr Address to send a "done" for
 */
void
uip_icmp6_mldv1_done(uip_ip6addr_t * addr);

/** \brief Process an MLDv1 query and send multicast listener reports for
 * suitable addresses.
 */
void
uip_icmp6_ml_query_input(void);

/** \brief Process an MLDv1 report and suppress listener reports for
 * suitable addresses.
 */
void
uip_icmp6_ml_report_input(void);

/**
 * \brief Checks whether MLD works has to be done, and if so, does it.
 */
void
uip_mld_periodic(void);

extern struct etimer uip_mld_timer_periodic;

/** @} */

#endif

#endif /*MLD_H_ */
/** @} */

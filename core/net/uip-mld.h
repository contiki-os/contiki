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
 * (c) Fraunhofer ITWM - Phoebe Buckheister <phoebe.buckheister@itwm.fhg.de>, 2012
 *
 * Contiki-MLD is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Contiki-MLD is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Contiki-MLD. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __MLD_H__
#define __MLD_H__

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
uip_icmp6_mldv1_schedule_report(uip_ds6_maddr_t *addr);

/**
 * \brief Send an MLDv1 listener report for the specified maddr
 * \param addr Address to send a report for
 */
void
uip_icmp6_mldv1_report(uip_ip6addr_t *addr);

/**
 * \brief Send an MLDv1 listener done for the specified maddr
 * \param addr Address to send a "done" for
 */
void
uip_icmp6_mldv1_done(uip_ip6addr_t *addr);

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

#endif /*__MLD_H__*/
/** @} */


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

#include "net/uip-mld.h"

#if UIP_CONF_MLD

#include "net/uip-ds6.h"
#include "net/uip-icmp6.h"
#include "net/tcpip.h"
#include "lib/random.h"

#define DEBUG 0
#if DEBUG
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#define PRINT6ADDR(addr) PRINTF(" %02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x ", ((u8_t *)addr)[0], ((u8_t *)addr)[1], ((u8_t *)addr)[2], ((u8_t *)addr)[3], ((u8_t *)addr)[4], ((u8_t *)addr)[5], ((u8_t *)addr)[6], ((u8_t *)addr)[7], ((u8_t *)addr)[8], ((u8_t *)addr)[9], ((u8_t *)addr)[10], ((u8_t *)addr)[11], ((u8_t *)addr)[12], ((u8_t *)addr)[13], ((u8_t *)addr)[14], ((u8_t *)addr)[15])
#define PRINTLLADDR(lladdr) PRINTF(" %02x:%02x:%02x:%02x:%02x:%02x ",lladdr->addr[0], lladdr->addr[1], lladdr->addr[2], lladdr->addr[3],lladdr->addr[4], lladdr->addr[5])
#else
#define PRINTF(...)
#define PRINT6ADDR(addr)
#endif

#define UIP_IP_BUF                ((struct uip_ip_hdr *)&uip_buf[UIP_LLH_LEN])
#define UIP_ICMP_BUF            ((struct uip_icmp_hdr *)&uip_buf[uip_l2_l3_hdr_len])
#define UIP_ICMP6_ERROR_BUF  ((struct uip_icmp6_error *)&uip_buf[uip_l2_l3_icmp_hdr_len])
#define UIP_ICMP6_MLD_BUF  ((struct uip_icmp6_mld1 *)&uip_buf[uip_l2_l3_icmp_hdr_len])

struct etimer uip_mld_timer_periodic;

static inline void mld_report_later(uip_ds6_maddr_t *addr, uint16_t timeout)
{
  int when = random_rand() % timeout;
  PRINTF("Report in %is:", when);
  PRINT6ADDR(&addr->ipaddr);
  PRINTF("\n");
  stimer_set(&addr->report_timeout, when);
}

/*---------------------------------------------------------------------------*/
static void
send_mldv1_packet(uip_ip6addr_t *maddr, uint8_t mld_type)
{
  /* IP header */
  /* MLD requires hoplimits to be 1 and source addresses to be link-local.
   * Since routers must send queries from link-local addresses, a link local
   * source be selected.
   * The destination IP must be the multicast group, though, and source address selection
   * will choose a routable address (if available) for multicast groups that are themselves
   * routable. Thus, select the source address before filling the destination.
   **/
  UIP_IP_BUF->ttl = 1;
  uip_ds6_select_src(&UIP_IP_BUF->srcipaddr, &UIP_IP_BUF->destipaddr);
  /* If the selected source is ::, the MLD packet would be invalid. */
  if (uip_is_addr_unspecified(&UIP_IP_BUF->destipaddr)) {
    return;
  }
  
  if (mld_type == ICMP6_ML_REPORT) {
    uip_ipaddr_copy(&UIP_IP_BUF->destipaddr, maddr);
  } else {
    uip_create_linklocal_allrouters_mcast(&UIP_IP_BUF->destipaddr);
  }

  UIP_IP_BUF->proto = UIP_PROTO_HBHO;
  uip_len = UIP_LLH_LEN + UIP_IPH_LEN;

  ((uip_hbho_hdr*) &uip_buf[uip_len])->next = UIP_PROTO_ICMP6;
  /* we need only pad with two bytes, so the PadN header is sufficient */
  /* also, len is in units of eight octets, excluding the first. */
  ((uip_hbho_hdr*) &uip_buf[uip_len])->len = (UIP_HBHO_LEN + UIP_RTR_ALERT_LEN + UIP_PADN_LEN) / 8 - 1;
  uip_len += UIP_HBHO_LEN;

  ((uip_ext_hdr_rtr_alert_tlv*) &uip_buf[uip_len])->tag = UIP_EXT_HDR_OPT_RTR_ALERT;
  ((uip_ext_hdr_rtr_alert_tlv*) &uip_buf[uip_len])->len = 2; /* data length of value field */
  ((uip_ext_hdr_rtr_alert_tlv*) &uip_buf[uip_len])->value = 0; /* MLD message */
  uip_len += UIP_RTR_ALERT_LEN;

  ((uip_ext_hdr_padn_tlv*) &uip_buf[uip_len])->tag = UIP_EXT_HDR_OPT_PADN;
  ((uip_ext_hdr_padn_tlv*) &uip_buf[uip_len])->len = 0; /* no data bytes following */
  uip_len += UIP_PADN_LEN;

  uip_len += UIP_ICMPH_LEN;

  uip_ext_len = UIP_HBHO_LEN + UIP_RTR_ALERT_LEN + UIP_PADN_LEN;
  uip_len += UIP_ICMP6_MLD1_LEN;

  UIP_IP_BUF->len[0] = ((uip_len - UIP_IPH_LEN) >> 8);
  UIP_IP_BUF->len[1] = ((uip_len - UIP_IPH_LEN) & 0xff);
  UIP_ICMP_BUF->type = mld_type;
  UIP_ICMP_BUF->icode = 0;

  UIP_ICMP6_MLD_BUF->maximum_delay = 0;
  UIP_ICMP6_MLD_BUF->reserved = 0;
  uip_ipaddr_copy(&UIP_ICMP6_MLD_BUF->address, maddr);

  UIP_ICMP_BUF->icmpchksum = 0;
  UIP_ICMP_BUF->icmpchksum = ~uip_icmp6chksum();
 
  tcpip_ipv6_output();
  UIP_STAT(++uip_stat.icmp.sent);
}

/*---------------------------------------------------------------------------*/
void
uip_icmp6_mldv1_schedule_report(uip_ds6_maddr_t *addr)
{
  addr->report_count = 3;
  stimer_set(&addr->report_timeout, 0);
  etimer_set(&uip_mld_timer_periodic, CLOCK_SECOND / 4);
}

/*---------------------------------------------------------------------------*/
void
uip_icmp6_mldv1_report(uip_ip6addr_t *addr)
{
  if (uip_is_addr_linklocal_allnodes_mcast(addr)) {
    PRINTF("Not sending MLDv1 report for FF02::1\n");
    return;
  }

  PRINTF("Sending MLDv1 report for");
  PRINT6ADDR(addr);
  PRINTF("\n");

  send_mldv1_packet(addr, ICMP6_ML_REPORT);
}

/*---------------------------------------------------------------------------*/
void
uip_icmp6_mldv1_done(uip_ip6addr_t *addr)
{
  if (uip_is_addr_linklocal_allnodes_mcast(addr)) {
    PRINTF("Not sending MLDv1 done for FF02::1\n");
    return;
  }

  PRINTF("Sending MLDv1 done for");
  PRINT6ADDR(addr);
  PRINTF("\n");

  send_mldv1_packet(addr, ICMP6_ML_DONE);
}

/*---------------------------------------------------------------------------*/
void
uip_icmp6_ml_query_input(void)
{
  uip_ds6_maddr_t *addr;
  uint8_t m;
  uint16_t max_delay;

  /*
   * Send an MLDv1 report packet for every multicast address known to be ours.
   */
  PRINTF("Received MLD query from");
  PRINT6ADDR(&UIP_IP_BUF->srcipaddr);
  PRINTF("for");
  PRINT6ADDR(&UIP_ICMP6_MLD_BUF->address);
  PRINTF("\n");

  max_delay = uip_ntohs(UIP_ICMP6_MLD_BUF->maximum_delay);

  if (uip_ext_len == 0) {
    PRINTF("MLD packet without hop-by-hop header received\n");
    return;
  } else {
    if (!uip_is_addr_linklocal_allnodes_mcast(&UIP_ICMP6_MLD_BUF->address)
        && uip_ds6_is_my_maddr(&UIP_ICMP6_MLD_BUF->address)) {
      addr = uip_ds6_maddr_lookup(&UIP_ICMP6_MLD_BUF->address);
      addr->report_count = 1;
      mld_report_later(addr, max_delay / 1000);
    } else if (uip_is_addr_unspecified(&UIP_ICMP6_MLD_BUF->address)) {
      for (m = 0; m < UIP_DS6_MADDR_NB; m++) {
        if (uip_ds6_if.maddr_list[m].isused) {
          uip_ds6_if.maddr_list[m].report_count = 1;
          mld_report_later(&uip_ds6_if.maddr_list[m], max_delay / 1000);
        }
      }
    }
  }

  etimer_set(&uip_mld_timer_periodic, CLOCK_SECOND / 4);
}

/*---------------------------------------------------------------------------*/
void
uip_icmp6_ml_report_input(void)
{
  uip_ds6_maddr_t *addr;

  PRINTF("Received MLD report from");
  PRINT6ADDR(&UIP_IP_BUF->srcipaddr);
  PRINTF("for");
  PRINT6ADDR(&UIP_IP_BUF->destipaddr);
  PRINTF("\n");

  if (uip_ext_len == 0) {
    PRINTF("MLD packet without hop-by-hop header received\n");
  } else if (uip_ds6_is_my_maddr(&UIP_ICMP6_MLD_BUF->address)) {
    addr = uip_ds6_maddr_lookup(&UIP_ICMP6_MLD_BUF->address);
    if (addr->report_count > 0)
      addr->report_count--;
  }
}

/*---------------------------------------------------------------------------*/
void
uip_mld_periodic(void)
{
  uint8_t m;
  uip_ds6_maddr_t *addr;
  bool more = false;

  for (m = 0; m < UIP_DS6_MADDR_NB; m++) {
    addr = &uip_ds6_if.maddr_list[m];
    if (addr->isused && addr->report_count) {
      if (stimer_expired(&addr->report_timeout)) {
        uip_icmp6_mldv1_report(&addr->ipaddr);
        if (--addr->report_count) {
          if (addr->report_timeout.interval == 0)
            mld_report_later(addr, UIP_IP6_MLD_REPORT_INTERVAL);
          stimer_restart(&addr->report_timeout);
        }
      }
      more = true;
    }
  }

  if (more)
    etimer_set(&uip_mld_timer_periodic, CLOCK_SECOND / 4);
}

/** @} */

#endif

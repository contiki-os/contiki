/**
 * \file
 *         Management of IPsec hosts metadata
 * \author
 *         Simon Duquennoy <simonduq@sics.se>
 */

#include "contiki-conf.h"

#if WITH_IPSEC

#include <string.h>
#include "ipsec.h"
#include "lib/list.h"
#include "lib/memb.h"

/*---------------------------------------------------------------------------*/
MEMB(host_memb, struct ipsec_host_metadata, IPSEC_HOSTS_NUM);
LIST(host_list);

/*---------------------------------------------------------------------------*/
struct ipsec_host_metadata*
ipsec_host_lookup(uip_ip6addr_t *remote_addr)
{
  struct ipsec_host_metadata *md;
  md = list_head(host_list);
  while(md) {
    if(uip_ipaddr_cmp(remote_addr, &md->remote_addr)) {
      return md;
    }
    md = list_item_next(md);
  }
  return NULL;
}
/*---------------------------------------------------------------------------*/
struct ipsec_host_metadata*
ipsec_host_insert(uip_ip6addr_t *remote_addr,
    uint32_t spi, enum ipsec_mode mode)
{
  struct ipsec_host_metadata *md;
  md = ipsec_host_lookup(remote_addr);
  if(md == NULL) {
    md = memb_alloc(&host_memb);
    if(md == NULL) {
      md = list_pop(host_list);
    }
    list_add(host_list, md);
    memcpy(&md->remote_addr, remote_addr, sizeof(uip_ip6addr_t));
    md->seqno = 0;
  }
  md->spi = spi;
  md->mode = mode;
  md->used = 1;
  return md;
}
/*---------------------------------------------------------------------------*/
void
ipsec_host_clear(uip_ip6addr_t *remote_addr)
{
  struct ipsec_host_metadata *md;
  md = ipsec_host_lookup(remote_addr);
  if(md != NULL) {
    md->used = 0;
  }
}
/*---------------------------------------------------------------------------*/

#endif /* WITH_IPSEC */

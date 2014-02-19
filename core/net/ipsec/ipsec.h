/**
 * \file
 *         IPsec constant values, headers, ...
 * \author
 *         Simon Duquennoy <simonduq@sics.se>
 */

#ifndef __IPSEC_H__
#define __IPSEC_H__

#if WITH_IPSEC

#include "net/uip.h"
#include "aes-moo.h"

#define IPSEC_KEYSIZE   16
#define IPSEC_MACSIZE   12
#define IPSEC_IVSIZE    8

#define UIP_PROTO_ESP   50
#define UIP_PROTO_AH    51

/* ESP header as defined in RFC 2406 */
struct uip_esp_header {
  uint32_t          spi;
  uint32_t          seq;
  unsigned char     iv[IPSEC_IVSIZE];
  unsigned char     data[0];
};

/* AH header as defined in RFC 4302 */
struct uip_ah_header {
  unsigned char     next;
  unsigned char     len;
  uint16_t          reserved;
  uint32_t          spi;
  uint32_t          seq;
  unsigned char     mac[IPSEC_MACSIZE];
};

/* The length of extension headers data coming after the payload */
extern u8_t uip_ext_end_len;

/* IPsec metadata for connected hosts */
#ifdef IPSEC_CONF_HOSTS_NUM
#define IPSEC_HOSTS_NUM IPSEC_CONF_HOSTS_NUM
#else
#define IPSEC_HOSTS_NUM 2
#endif

enum ipsec_mode { mode_esp, mode_ah };
struct ipsec_host_metadata {
  struct ipsec_host_metadata *next;
  uip_ip6addr_t               remote_addr;
  uint32_t                    spi;
  uint16_t                    seqno;
  uint8_t                     used;
  enum ipsec_mode             mode;
};

struct ipsec_host_metadata *ipsec_host_insert(uip_ip6addr_t *remote_addr,
    uint32_t spi, enum ipsec_mode mode);
struct ipsec_host_metadata *ipsec_host_lookup(uip_ip6addr_t *remote_addr);
void ipsec_host_clear(uip_ip6addr_t *remote_addr);

#endif /* WITH_IPSEC */

#endif /* __IPSEC_H__ */

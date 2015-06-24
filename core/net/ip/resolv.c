/*
 * Copyright (c) 2002-2003, Adam Dunkels.
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
 * 3. The name of the author may not be used to endorse or promote
 *    products derived from this software without specific prior
 *    written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * This file is part of the uIP TCP/IP stack.
 *
 *
 */

/**
 * \file
 *         DNS host name to IP address resolver.
 * \author Adam Dunkels <adam@dunkels.com>
 * \author Robert Quattlebaum <darco@deepdarc.com>
 *
 *         This file implements a DNS host name to IP address resolver,
 *         as well as an MDNS responder and resolver.
 */

/**
 * \addtogroup uip
 * @{
 */

/**
 * \defgroup uipdns uIP hostname resolver functions
 * @{
 *
 * The uIP DNS resolver functions are used to lookup a hostname and
 * map it to a numerical IP address. It maintains a list of resolved
 * hostnames that can be queried with the resolv_lookup()
 * function. New hostnames can be resolved using the resolv_query()
 * function.
 *
 * The event resolv_event_found is posted when a hostname has been
 * resolved. It is up to the receiving process to determine if the
 * correct hostname has been found by calling the resolv_lookup()
 * function with the hostname.
 */

#include "net/ip/tcpip.h"
#include "net/ip/resolv.h"
#include "net/ip/uip-udp-packet.h"
#include "net/ip/uip-nameserver.h"
#include "lib/random.h"

#ifndef DEBUG
#define DEBUG CONTIKI_TARGET_COOJA
#endif

#if UIP_UDP

#include <string.h>
#include <stdio.h>
#include <ctype.h>

#ifndef NULL
#define NULL (void *)0
#endif /* NULL */

#if !defined(__SDCC) && defined(SDCC_REVISION)
#define __SDCC 1
#endif

#if VERBOSE_DEBUG
#define DEBUG_PRINTF(...) printf(__VA_ARGS__)
#else
#define DEBUG_PRINTF(...) do { } while(0)
#endif

#if DEBUG || VERBOSE_DEBUG
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...) do { } while(0)
#endif

#ifdef __SDCC
static int
strncasecmp(const char *s1, const char *s2, size_t n)
{
  /* TODO: Add case support! */
  return strncmp(s1, s2, n);
}
static int
strcasecmp(const char *s1, const char *s2)
{
  /* TODO: Add case support! */
  return strcmp(s1, s2);
}
#endif /* __SDCC */

#define UIP_UDP_BUF ((struct uip_udpip_hdr *)&uip_buf[UIP_LLH_LEN])

/* If RESOLV_CONF_SUPPORTS_MDNS is set, then queries
 * for domain names in the local TLD will use mDNS as
 * described by draft-cheshire-dnsext-multicastdns.
 */
#ifndef RESOLV_CONF_SUPPORTS_MDNS
#define RESOLV_CONF_SUPPORTS_MDNS 1
#endif

#ifndef RESOLV_CONF_MDNS_INCLUDE_GLOBAL_V6_ADDRS
#define RESOLV_CONF_MDNS_INCLUDE_GLOBAL_V6_ADDRS 0
#endif

/** The maximum number of retries when asking for a name. */
#ifndef RESOLV_CONF_MAX_RETRIES
#define RESOLV_CONF_MAX_RETRIES 4
#endif

#ifndef RESOLV_CONF_MAX_MDNS_RETRIES
#define RESOLV_CONF_MAX_MDNS_RETRIES 3
#endif

#ifndef RESOLV_CONF_MAX_DOMAIN_NAME_SIZE
#define RESOLV_CONF_MAX_DOMAIN_NAME_SIZE 32
#endif

#ifdef RESOLV_CONF_AUTO_REMOVE_TRAILING_DOTS
#define RESOLV_AUTO_REMOVE_TRAILING_DOTS RESOLV_CONF_AUTO_REMOVE_TRAILING_DOTS
#else
#define RESOLV_AUTO_REMOVE_TRAILING_DOTS RESOLV_CONF_SUPPORTS_MDNS
#endif

#ifdef RESOLV_CONF_VERIFY_ANSWER_NAMES
#define RESOLV_VERIFY_ANSWER_NAMES RESOLV_CONF_VERIFY_ANSWER_NAMES
#else
#define RESOLV_VERIFY_ANSWER_NAMES RESOLV_CONF_SUPPORTS_MDNS
#endif

#ifdef RESOLV_CONF_SUPPORTS_RECORD_EXPIRATION
#define RESOLV_SUPPORTS_RECORD_EXPIRATION RESOLV_CONF_SUPPORTS_RECORD_EXPIRATION
#else
#define RESOLV_SUPPORTS_RECORD_EXPIRATION 1
#endif

#if RESOLV_CONF_SUPPORTS_MDNS && !RESOLV_VERIFY_ANSWER_NAMES
#error RESOLV_CONF_SUPPORTS_MDNS cannot be set without RESOLV_CONF_VERIFY_ANSWER_NAMES
#endif

#if !defined(CONTIKI_TARGET_NAME) && defined(BOARD)
#define stringy2(x) #x
#define stringy(x)  stringy2(x)
#define CONTIKI_TARGET_NAME stringy(BOARD)
#endif

#ifndef CONTIKI_CONF_DEFAULT_HOSTNAME
#ifdef CONTIKI_TARGET_NAME
#define CONTIKI_CONF_DEFAULT_HOSTNAME "contiki-"CONTIKI_TARGET_NAME
#else
#define CONTIKI_CONF_DEFAULT_HOSTNAME "contiki"
#endif
#endif

#define DNS_TYPE_A      1
#define DNS_TYPE_CNAME  5
#define DNS_TYPE_PTR   12
#define DNS_TYPE_MX    15
#define DNS_TYPE_TXT   16
#define DNS_TYPE_AAAA  28
#define DNS_TYPE_SRV   33
#define DNS_TYPE_ANY  255
#define DNS_TYPE_NSEC  47

#if NETSTACK_CONF_WITH_IPV6
#define NATIVE_DNS_TYPE DNS_TYPE_AAAA /* IPv6 */
#else
#define NATIVE_DNS_TYPE DNS_TYPE_A    /* IPv4 */
#endif

#define DNS_CLASS_IN    1
#define DNS_CLASS_ANY 255

#ifndef DNS_PORT
#define DNS_PORT 53
#endif

#ifndef MDNS_PORT
#define MDNS_PORT 5353
#endif

#ifndef MDNS_RESPONDER_PORT
#define MDNS_RESPONDER_PORT 5354
#endif

/** \internal The DNS message header. */
struct dns_hdr {
  uint16_t id;
  uint8_t flags1, flags2;
#define DNS_FLAG1_RESPONSE        0x80
#define DNS_FLAG1_OPCODE_STATUS   0x10
#define DNS_FLAG1_OPCODE_INVERSE  0x08
#define DNS_FLAG1_OPCODE_STANDARD 0x00
#define DNS_FLAG1_AUTHORATIVE     0x04
#define DNS_FLAG1_TRUNC           0x02
#define DNS_FLAG1_RD              0x01
#define DNS_FLAG2_RA              0x80
#define DNS_FLAG2_ERR_MASK        0x0f
#define DNS_FLAG2_ERR_NONE        0x00
#define DNS_FLAG2_ERR_NAME        0x03
  uint16_t numquestions;
  uint16_t numanswers;
  uint16_t numauthrr;
  uint16_t numextrarr;
};

/** \internal The DNS answer message structure. */
struct dns_answer {
  /* DNS answer record starts with either a domain name or a pointer
   * to a name already present somewhere in the packet. */
  uint16_t type;
  uint16_t class;
  uint16_t ttl[2];
  uint16_t len;
#if NETSTACK_CONF_WITH_IPV6
  uint8_t ipaddr[16];
#else
  uint8_t ipaddr[4];
#endif
};

struct namemap {
#define STATE_UNUSED 0
#define STATE_ERROR  1
#define STATE_NEW    2
#define STATE_ASKING 3
#define STATE_DONE   4
  uint8_t state;
  uint8_t tmr;
  uint16_t id;
  uint8_t retries;
  uint8_t seqno;
#if RESOLV_SUPPORTS_RECORD_EXPIRATION
  unsigned long expiration;
#endif /* RESOLV_SUPPORTS_RECORD_EXPIRATION */
  uip_ipaddr_t ipaddr;
  uint8_t err;
  uint8_t server;
#if RESOLV_CONF_SUPPORTS_MDNS
  int is_mdns:1, is_probe:1;
#endif
  char name[RESOLV_CONF_MAX_DOMAIN_NAME_SIZE + 1];
};

#ifndef UIP_CONF_RESOLV_ENTRIES
#define RESOLV_ENTRIES 4
#else /* UIP_CONF_RESOLV_ENTRIES */
#define RESOLV_ENTRIES UIP_CONF_RESOLV_ENTRIES
#endif /* UIP_CONF_RESOLV_ENTRIES */

static struct namemap names[RESOLV_ENTRIES];

static uint8_t seqno;

static struct uip_udp_conn *resolv_conn = NULL;

static struct etimer retry;

process_event_t resolv_event_found;

PROCESS(resolv_process, "DNS resolver");

static void resolv_found(char *name, uip_ipaddr_t * ipaddr);

/** \internal The DNS question message structure. */
struct dns_question {
  uint16_t type;
  uint16_t class;
};

#if RESOLV_CONF_SUPPORTS_MDNS
static char resolv_hostname[RESOLV_CONF_MAX_DOMAIN_NAME_SIZE + 1];

enum {
  MDNS_STATE_WAIT_BEFORE_PROBE,
  MDNS_STATE_PROBING,
  MDNS_STATE_READY,
};

static uint8_t mdns_state;

static const uip_ipaddr_t resolv_mdns_addr =
#if NETSTACK_CONF_WITH_IPV6
  { { 0xff, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xfb } };
#include "net/ipv6/uip-ds6.h"
#else  /* NETSTACK_CONF_WITH_IPV6 */
  { { 224, 0, 0, 251 } };
#endif /* NETSTACK_CONF_WITH_IPV6 */
static int mdns_needs_host_announce;

PROCESS(mdns_probe_process, "mDNS probe");
#endif /* RESOLV_CONF_SUPPORTS_MDNS */

/*---------------------------------------------------------------------------*/
#if RESOLV_VERIFY_ANSWER_NAMES || VERBOSE_DEBUG
/** \internal
 * \brief Decodes a DNS name from the DNS format into the given string.
 * \return 1 upon success, 0 if the size of the name would be too large.
 *
 * \note `dest` must point to a buffer with at least
 *       `RESOLV_CONF_MAX_DOMAIN_NAME_SIZE+1` bytes large.
 */
static uint8_t
decode_name(const unsigned char *query, char *dest,
            const unsigned char *packet)
{
  int len = RESOLV_CONF_MAX_DOMAIN_NAME_SIZE;

  unsigned char n = *query++;

  //DEBUG_PRINTF("resolver: decoding name: \"");

  while(len && n) {
    if(n & 0xc0) {
      const uint16_t offset = query[0] + ((n & ~0xC0) << 8);

      //DEBUG_PRINTF("<skip-to-%d>",offset);
      query = packet + offset;
      n = *query++;
    }

    if(!n)
      break;

    for(; n; --n) {
      //DEBUG_PRINTF("%c",*query);

      *dest++ = *query++;

      if(!--len) {
        *dest = 0;
        return 0;
      }
    }

    n = *query++;

    if(n) {
      //DEBUG_PRINTF(".");
      *dest++ = '.';
      --len;
    }
  }

  //DEBUG_PRINTF("\"\n");
  *dest = 0;
  return len != 0;
}
/*---------------------------------------------------------------------------*/
/** \internal
 */
static uint8_t
dns_name_isequal(const unsigned char *queryptr, const char *name,
                 const unsigned char *packet)
{
  unsigned char n = *queryptr++;

  if(*name == 0)
    return 0;

  while(n) {
    if(n & 0xc0) {
      queryptr = packet + queryptr[0] + ((n & ~0xC0) << 8);
      n = *queryptr++;
    }

    for(; n; --n) {
      if(!*name) {
        return 0;
      }

      if(tolower((unsigned int)*name++) != tolower((unsigned int)*queryptr++)) {
        return 0;
      }
    }

    n = *queryptr++;

    if((n != 0) && (*name++ != '.')) {
      return 0;
    }
  }

  if(*name == '.')
    ++name;

  return name[0] == 0;
}
#endif /* RESOLV_VERIFY_ANSWER_NAMES */
/*---------------------------------------------------------------------------*/
/** \internal
 */
static unsigned char *
skip_name(unsigned char *query)
{
  unsigned char n;

  DEBUG_PRINTF("resolver: skip name: ");

  do {
    n = *query;
    if(n & 0xc0) {
      DEBUG_PRINTF("<skip-to-%d>", query[0] + ((n & ~0xC0) << 8));
      ++query;
      break;
    }

    ++query;

    while(n > 0) {
      DEBUG_PRINTF("%c", *query);
      ++query;
      --n;
    };
    DEBUG_PRINTF(".");
  } while(*query != 0);
  DEBUG_PRINTF("\n");
  return query + 1;
}
/*---------------------------------------------------------------------------*/
/** \internal
 */
static unsigned char *
encode_name(unsigned char *query, const char *nameptr)
{
  char *nptr;

  --nameptr;
  /* Convert hostname into suitable query format. */
  do {
    uint8_t n = 0;

    ++nameptr;
    nptr = (char *)query;
    ++query;
    for(n = 0; *nameptr != '.' && *nameptr != 0; ++nameptr) {
      *query = *nameptr;
      ++query;
      ++n;
    }
    *nptr = n;
  } while(*nameptr != 0);

  /* End the the name. */
  *query++ = 0;

  return query;
}
/*---------------------------------------------------------------------------*/
#if RESOLV_CONF_SUPPORTS_MDNS
/** \internal
 */
static void
mdns_announce_requested(void)
{
  mdns_needs_host_announce = 1;
}
/*---------------------------------------------------------------------------*/
/** \internal
 */
static void
start_name_collision_check(clock_time_t after)
{
  process_exit(&mdns_probe_process);
  process_start(&mdns_probe_process, (void *)&after);
}
/*---------------------------------------------------------------------------*/
/** \internal
 */
static unsigned char *
mdns_write_announce_records(unsigned char *queryptr, uint8_t *count)
{
#if NETSTACK_CONF_WITH_IPV6
  uint8_t i;

  for(i = 0; i < UIP_DS6_ADDR_NB; ++i) {
    if(uip_ds6_if.addr_list[i].isused
#if !RESOLV_CONF_MDNS_INCLUDE_GLOBAL_V6_ADDRS
       && uip_is_addr_link_local(&uip_ds6_if.addr_list[i].ipaddr)
#endif
      ) {
      if(!*count) {
        queryptr = encode_name(queryptr, resolv_hostname);
      } else {
        /* Use name compression to refer back to the first name */
        *queryptr++ = 0xc0;
        *queryptr++ = sizeof(struct dns_hdr);
      }

      *queryptr++ = (uint8_t) ((NATIVE_DNS_TYPE) >> 8);
      *queryptr++ = (uint8_t) ((NATIVE_DNS_TYPE));

      *queryptr++ = (uint8_t) ((DNS_CLASS_IN | 0x8000) >> 8);
      *queryptr++ = (uint8_t) ((DNS_CLASS_IN | 0x8000));

      *queryptr++ = 0;
      *queryptr++ = 0;
      *queryptr++ = 0;
      *queryptr++ = 120;

      *queryptr++ = 0;
      *queryptr++ = sizeof(uip_ipaddr_t);

      uip_ipaddr_copy((uip_ipaddr_t*)queryptr, &uip_ds6_if.addr_list[i].ipaddr);
      queryptr += sizeof(uip_ipaddr_t);
      ++(*count);
    }
  }
#else /* NETSTACK_CONF_WITH_IPV6 */
  struct dns_answer *ans;

  queryptr = encode_name(queryptr, resolv_hostname);
  ans = (struct dns_answer *)queryptr;
  ans->type = UIP_HTONS(NATIVE_DNS_TYPE);
  ans->class = UIP_HTONS(DNS_CLASS_IN | 0x8000);
  ans->ttl[0] = 0;
  ans->ttl[1] = UIP_HTONS(120);
  ans->len = UIP_HTONS(sizeof(uip_ipaddr_t));
  uip_gethostaddr((uip_ipaddr_t *) ans->ipaddr);
  queryptr = (unsigned char *)ans + sizeof(*ans);
  ++(*count);
#endif /* NETSTACK_CONF_WITH_IPV6 */
  return queryptr;
}
/*---------------------------------------------------------------------------*/
/** \internal
 * Called when we need to announce ourselves
 */
static size_t
mdns_prep_host_announce_packet(void)
{
  static const struct {
    uint16_t type;
    uint16_t class;
    uint16_t ttl[2];
    uint16_t len;
    uint8_t data[8];

  } nsec_record = {
    UIP_HTONS(DNS_TYPE_NSEC),
    UIP_HTONS(DNS_CLASS_IN | 0x8000),
    { 0, UIP_HTONS(120) },
    UIP_HTONS(8),

    {
      0xc0,
      sizeof(struct dns_hdr), /* Name compression. Re-using the name of first record. */
      0x00,
      0x04,

#if NETSTACK_CONF_WITH_IPV6
      0x00,
      0x00,
      0x00,
      0x08,
#else /* NETSTACK_CONF_WITH_IPV6 */
      0x40,
      0x00,
      0x00,
      0x00,
#endif /* NETSTACK_CONF_WITH_IPV6 */
    }
  };

  unsigned char *queryptr;

  uint8_t total_answers = 0;

  /* Be aware that, unless `ARCH_DOESNT_NEED_ALIGNED_STRUCTS` is set,
   * writing directly to the uint16_t members of this struct is an error. */
  struct dns_hdr *hdr = (struct dns_hdr *)uip_appdata;

  /* Zero out the header */
  memset((void *)hdr, 0, sizeof(*hdr));

  hdr->flags1 |= DNS_FLAG1_RESPONSE | DNS_FLAG1_AUTHORATIVE;

  queryptr = (unsigned char *)uip_appdata + sizeof(*hdr);

  queryptr = mdns_write_announce_records(queryptr, &total_answers);

  /* We now need to add an NSEC record to indicate
   * that this is all there is.
   */
  if(!total_answers) {
    queryptr = encode_name(queryptr, resolv_hostname);
  } else {
    /* Name compression. Re-using the name of first record. */
    *queryptr++ = 0xc0;
    *queryptr++ = sizeof(*hdr);
  }

  memcpy((void *)queryptr, (void *)&nsec_record, sizeof(nsec_record));

  queryptr += sizeof(nsec_record);

  /* This platform might be picky about alignment. To avoid the possibility
   * of doing an unaligned write, we are going to do this manually. */
  ((uint8_t*)&hdr->numanswers)[1] = total_answers;
  ((uint8_t*)&hdr->numextrarr)[1] = 1;

  return (queryptr - (unsigned char *)uip_appdata);
}
#endif /* RESOLV_CONF_SUPPORTS_MDNS */
/*---------------------------------------------------------------------------*/
static char
try_next_server(struct namemap *namemapptr)
{
#if VERBOSE_DEBUG
  printf("server %d\n", namemapptr->server);
#endif
  namemapptr->server++;
  if(uip_nameserver_get(namemapptr->server) != NULL) {
    namemapptr->retries = 0;
    return 1;
  }
  namemapptr->server = 0;
  return 0;
}
/*---------------------------------------------------------------------------*/
/** \internal
 * Runs through the list of names to see if there are any that have
 * not yet been queried and, if so, sends out a query.
 */
static void
check_entries(void)
{
  volatile uint8_t i;

  uint8_t *query;

  register struct dns_hdr *hdr;

  register struct namemap *namemapptr;

  for(i = 0; i < RESOLV_ENTRIES; ++i) {
    namemapptr = &names[i];
    if(namemapptr->state == STATE_NEW || namemapptr->state == STATE_ASKING) {
      etimer_set(&retry, CLOCK_SECOND / 4);
      if(namemapptr->state == STATE_ASKING) {
        if(--namemapptr->tmr == 0) {
#if RESOLV_CONF_SUPPORTS_MDNS
          if(++namemapptr->retries ==
             (namemapptr->is_mdns ? RESOLV_CONF_MAX_MDNS_RETRIES :
              RESOLV_CONF_MAX_RETRIES))
#else /* RESOLV_CONF_SUPPORTS_MDNS */
          if(++namemapptr->retries == RESOLV_CONF_MAX_RETRIES)
#endif /* RESOLV_CONF_SUPPORTS_MDNS */
          {
            /* Try the next server (if possible) before failing. Otherwise
               simply mark the entry as failed. */
            if(try_next_server(namemapptr) == 0) {
              /* STATE_ERROR basically means "not found". */
              namemapptr->state = STATE_ERROR;

#if RESOLV_SUPPORTS_RECORD_EXPIRATION
              /* Keep the "not found" error valid for 30 seconds */
              namemapptr->expiration = clock_seconds() + 30;
#endif /* RESOLV_SUPPORTS_RECORD_EXPIRATION */

              resolv_found(namemapptr->name, NULL);
              continue;
            }
          }
          namemapptr->tmr = namemapptr->retries * namemapptr->retries * 3;

#if RESOLV_CONF_SUPPORTS_MDNS
          if(namemapptr->is_probe) {
            /* Probing retries are much more aggressive, 250ms */
            namemapptr->tmr = 2;
          }
#endif /* RESOLV_CONF_SUPPORTS_MDNS */
        } else {
          /* Its timer has not run out, so we move on to next
           * entry.
           */
          continue;
        }
      } else {
        namemapptr->state = STATE_ASKING;
        namemapptr->tmr = 1;
        namemapptr->retries = 0;
      }
      hdr = (struct dns_hdr *)uip_appdata;
      memset(hdr, 0, sizeof(struct dns_hdr));
      hdr->id = random_rand();
      namemapptr->id = hdr->id;
#if RESOLV_CONF_SUPPORTS_MDNS
      if(!namemapptr->is_mdns || namemapptr->is_probe) {
        hdr->flags1 = DNS_FLAG1_RD;
      }
      if(namemapptr->is_mdns) {
        hdr->id = 0;
      }
#else /* RESOLV_CONF_SUPPORTS_MDNS */
      hdr->flags1 = DNS_FLAG1_RD;
#endif /* RESOLV_CONF_SUPPORTS_MDNS */
      hdr->numquestions = UIP_HTONS(1);
      query = (unsigned char *)uip_appdata + sizeof(*hdr);
      query = encode_name(query, namemapptr->name);
#if RESOLV_CONF_SUPPORTS_MDNS
      if(namemapptr->is_probe) {
        *query++ = (uint8_t) ((DNS_TYPE_ANY) >> 8);
        *query++ = (uint8_t) ((DNS_TYPE_ANY));
      } else
#endif /* RESOLV_CONF_SUPPORTS_MDNS */
      {
        *query++ = (uint8_t) ((NATIVE_DNS_TYPE) >> 8);
        *query++ = (uint8_t) ((NATIVE_DNS_TYPE));
      }
      *query++ = (uint8_t) ((DNS_CLASS_IN) >> 8);
      *query++ = (uint8_t) ((DNS_CLASS_IN));
#if RESOLV_CONF_SUPPORTS_MDNS
      if(namemapptr->is_mdns) {
        if(namemapptr->is_probe) {
          /* This is our conflict detection request.
           * In order to be in compliance with the MDNS
           * spec, we need to add the records we are proposing
           * to the rrauth section.
           */
          uint8_t count = 0;

          query = mdns_write_announce_records(query, &count);
          hdr->numauthrr = UIP_HTONS(count);
        }
        uip_udp_packet_sendto(resolv_conn, uip_appdata,
                              (query - (uint8_t *) uip_appdata),
                              &resolv_mdns_addr, UIP_HTONS(MDNS_PORT));

        PRINTF("resolver: (i=%d) Sent MDNS %s for \"%s\".\n", i,
               namemapptr->is_probe?"probe":"request",namemapptr->name);
      } else {
        uip_udp_packet_sendto(resolv_conn, uip_appdata,
                              (query - (uint8_t *) uip_appdata),
                              (const uip_ipaddr_t *)
                                uip_nameserver_get(namemapptr->server), 
                              UIP_HTONS(DNS_PORT));

        PRINTF("resolver: (i=%d) Sent DNS request for \"%s\".\n", i,
               namemapptr->name);
      }
#else /* RESOLV_CONF_SUPPORTS_MDNS */
      uip_udp_packet_sendto(resolv_conn, uip_appdata,
                            (query - (uint8_t *) uip_appdata),
                            uip_nameserver_get(namemapptr->server), 
                            UIP_HTONS(DNS_PORT));
      PRINTF("resolver: (i=%d) Sent DNS request for \"%s\".\n", i,
             namemapptr->name);
#endif /* RESOLV_CONF_SUPPORTS_MDNS */
      break;
    }
  }
}
/*---------------------------------------------------------------------------*/
/** \internal
 * Called when new UDP data arrives.
 */
static void
newdata(void)
{
  static uint8_t nquestions, nanswers;

  static int8_t i;

  register struct namemap *namemapptr = NULL;

  struct dns_answer *ans;

  register struct dns_hdr const *hdr = (struct dns_hdr *)uip_appdata;

  unsigned char *queryptr = (unsigned char *)hdr + sizeof(*hdr);

  const uint8_t is_request = ((hdr->flags1 & ~1) == 0) && (hdr->flags2 == 0);

  /* We only care about the question(s) and the answers. The authrr
   * and the extrarr are simply discarded.
   */
  nquestions = (uint8_t) uip_ntohs(hdr->numquestions);
  nanswers = (uint8_t) uip_ntohs(hdr->numanswers);

  queryptr = (unsigned char *)hdr + sizeof(*hdr);
  i = 0;

  DEBUG_PRINTF
    ("resolver: flags1=0x%02X flags2=0x%02X nquestions=%d, nanswers=%d, nauthrr=%d, nextrarr=%d\n",
     hdr->flags1, hdr->flags2, (uint8_t) nquestions, (uint8_t) nanswers,
     (uint8_t) uip_ntohs(hdr->numauthrr),
     (uint8_t) uip_ntohs(hdr->numextrarr));

  if(is_request && (nquestions == 0)) {
    /* Skip requests with no questions. */
    DEBUG_PRINTF("resolver: Skipping request with no questions.\n");
    return;
  }

/** QUESTION HANDLING SECTION ************************************************/

  for(; nquestions > 0;
      queryptr = skip_name(queryptr) + sizeof(struct dns_question),
      --nquestions
  ) {
#if RESOLV_CONF_SUPPORTS_MDNS
    if(!is_request) {
      /* If this isn't a request, we don't need to bother
       * looking at the individual questions. For the most
       * part, this loop to just used to skip past them.
       */
      continue;
    }

    {
      struct dns_question *question = (struct dns_question *)skip_name(queryptr);

#if !ARCH_DOESNT_NEED_ALIGNED_STRUCTS
      static struct dns_question aligned;
      memcpy(&aligned, question, sizeof(aligned));
      question = &aligned;
#endif /* !ARCH_DOESNT_NEED_ALIGNED_STRUCTS */

      DEBUG_PRINTF("resolver: Question %d: type=%d class=%d\n", ++i,
                   uip_htons(question->type), uip_htons(question->class));

      if(((uip_ntohs(question->class) & 0x7FFF) != DNS_CLASS_IN) ||
         ((question->type != UIP_HTONS(DNS_TYPE_ANY)) &&
          (question->type != UIP_HTONS(NATIVE_DNS_TYPE)))) {
        /* Skip unrecognised records. */
        continue;
      }

      if(!dns_name_isequal(queryptr, resolv_hostname, uip_appdata)) {
        continue;
      }

      PRINTF("resolver: THIS IS A REQUEST FOR US!!!\n");

      if(mdns_state == MDNS_STATE_READY) {
        /* We only send immediately if this isn't an MDNS request.
         * Otherwise, we schedule ourselves to send later.
         */
        if(UIP_UDP_BUF->srcport == UIP_HTONS(MDNS_PORT)) {
          mdns_announce_requested();
        } else {
          uip_udp_packet_sendto(resolv_conn, uip_appdata,
                                mdns_prep_host_announce_packet(),
                                &UIP_UDP_BUF->srcipaddr,
                                UIP_UDP_BUF->srcport);
        }
        return;
      } else {
        static uint8_t nauthrr;
        PRINTF("resolver: But we are still probing. Waiting...\n");
        /* We are still probing. We need to do the mDNS
         * probe race condition check here and make sure
         * we don't need to delay probing for a second.
         */
        nauthrr = (uint8_t)uip_ntohs(hdr->numauthrr);

        /* For now, we will always restart the collision check if
         * there are *any* authority records present.
         * In the future we should follow the spec more closely,
         * but this should eventually converge to something reasonable.
         */
        if(nauthrr) {
          start_name_collision_check(CLOCK_SECOND);
        }
      }
    }
#endif /* RESOLV_CONF_SUPPORTS_MDNS */
  }

/** ANSWER HANDLING SECTION **************************************************/

  if(nanswers == 0) {
    /* Skip responses with no answers. */
    return;
  }

#if RESOLV_CONF_SUPPORTS_MDNS
  if(UIP_UDP_BUF->srcport == UIP_HTONS(MDNS_PORT) &&
     hdr->id == 0) {
    /* OK, this was from MDNS. Things get a little weird here,
     * because we can't use the `id` field. We will look up the
     * appropriate request in a later step. */

    i = -1;
    namemapptr = NULL;
  } else
#endif /* RESOLV_CONF_SUPPORTS_MDNS */
  {
    for(i = 0; i < RESOLV_ENTRIES; ++i) {
      namemapptr = &names[i];
      if(namemapptr->state == STATE_ASKING &&
         namemapptr->id == hdr->id) {
        break;
      }
    }

    if(i >= RESOLV_ENTRIES || i < 0 || namemapptr->state != STATE_ASKING) {
      PRINTF("resolver: DNS response has bad ID (%04X) \n", uip_ntohs(hdr->id));
      return;
    }

    PRINTF("resolver: Incoming response for \"%s\".\n", namemapptr->name);

    /* We'll change this to DONE when we find the record. */
    namemapptr->state = STATE_ERROR;

    namemapptr->err = hdr->flags2 & DNS_FLAG2_ERR_MASK;

#if RESOLV_SUPPORTS_RECORD_EXPIRATION
    /* If we remain in the error state, keep it cached for 30 seconds. */
    namemapptr->expiration = clock_seconds() + 30;
#endif /* RESOLV_SUPPORTS_RECORD_EXPIRATION */

    /* Check for error. If so, call callback to inform. */
    if(namemapptr->err != 0) {
      namemapptr->state = STATE_ERROR;
      resolv_found(namemapptr->name, NULL);
      return;
    }
  }

  i = 0;

  /* Answer parsing loop */
  while(nanswers > 0) {
    ans = (struct dns_answer *)skip_name(queryptr);

#if !ARCH_DOESNT_NEED_ALIGNED_STRUCTS
    {
      static struct dns_answer aligned;
      memcpy(&aligned, ans, sizeof(aligned));
      ans = &aligned;
    }
#endif /* !ARCH_DOESNT_NEED_ALIGNED_STRUCTS */

#if VERBOSE_DEBUG
    static char debug_name[40];
    decode_name(queryptr, debug_name, uip_appdata);
    DEBUG_PRINTF("resolver: Answer %d: \"%s\", type %d, class %d, ttl %d, length %d\n",
                 ++i, debug_name, uip_ntohs(ans->type),
                 uip_ntohs(ans->class) & 0x7FFF,
                 (int)((uint32_t) uip_ntohs(ans->ttl[0]) << 16) | (uint32_t)
                 uip_ntohs(ans->ttl[1]), uip_ntohs(ans->len));
#endif /* VERBOSE_DEBUG */

    /* Check the class and length of the answer to make sure
     * it matches what we are expecting
     */
    if(((uip_ntohs(ans->class) & 0x7FFF) != DNS_CLASS_IN) ||
       (ans->len != UIP_HTONS(sizeof(uip_ipaddr_t)))) {
      goto skip_to_next_answer;
    }

    if(ans->type != UIP_HTONS(NATIVE_DNS_TYPE)) {
      goto skip_to_next_answer;
    }

#if RESOLV_CONF_SUPPORTS_MDNS
    if(UIP_UDP_BUF->srcport == UIP_HTONS(MDNS_PORT) &&
       hdr->id == 0) {
      int8_t available_i = RESOLV_ENTRIES;

      DEBUG_PRINTF("resolver: MDNS query.\n");

      /* For MDNS, we need to actually look up the name we
       * are looking for.
       */
      for(i = 0; i < RESOLV_ENTRIES; ++i) {
        namemapptr = &names[i];
        if(dns_name_isequal(queryptr, namemapptr->name, uip_appdata)) {
          break;
        }
        if((namemapptr->state == STATE_UNUSED)
#if RESOLV_SUPPORTS_RECORD_EXPIRATION
          || (namemapptr->state == STATE_DONE && clock_seconds() > namemapptr->expiration)
#endif /* RESOLV_SUPPORTS_RECORD_EXPIRATION */
        ) {
          available_i = i;
        }
      }
      if(i == RESOLV_ENTRIES) {
        DEBUG_PRINTF("resolver: Unsolicited MDNS response.\n");
        i = available_i;
        namemapptr = &names[i];
        if(!decode_name(queryptr, namemapptr->name, uip_appdata)) {
          DEBUG_PRINTF("resolver: MDNS name too big to cache.\n");
          namemapptr = NULL;
          goto skip_to_next_answer;
        }
      }
      if(i == RESOLV_ENTRIES) {
        DEBUG_PRINTF
          ("resolver: Not enough room to keep track of unsolicited MDNS answer.\n");

        if(dns_name_isequal(queryptr, resolv_hostname, uip_appdata)) {
          /* Oh snap, they say they are us! We had better report them... */
          resolv_found(resolv_hostname, (uip_ipaddr_t *) ans->ipaddr);
        }
        namemapptr = NULL;
        goto skip_to_next_answer;
      }
      namemapptr = &names[i];

    } else
#endif /* RESOLV_CONF_SUPPORTS_MDNS */
    {
      /* This will force us to stop even if there are more answers. */
      nanswers = 1;
    }

/*  This is disabled for now, so that we don't fail on CNAME records.
#if RESOLV_VERIFY_ANSWER_NAMES
    if(namemapptr && !dns_name_isequal(queryptr, namemapptr->name, uip_appdata)) {
      DEBUG_PRINTF("resolver: Answer name doesn't match question...!\n");
      goto skip_to_next_answer;
    }
#endif
*/

    DEBUG_PRINTF("resolver: Answer for \"%s\" is usable.\n", namemapptr->name);

    namemapptr->state = STATE_DONE;
#if RESOLV_SUPPORTS_RECORD_EXPIRATION
    namemapptr->expiration = ans->ttl[1] + (ans->ttl[0] << 8);
    namemapptr->expiration += clock_seconds();
#endif /* RESOLV_SUPPORTS_RECORD_EXPIRATION */

    uip_ipaddr_copy(&namemapptr->ipaddr, (uip_ipaddr_t *) ans->ipaddr);

    resolv_found(namemapptr->name, &namemapptr->ipaddr);
    break;

  skip_to_next_answer:
    queryptr = (unsigned char *)skip_name(queryptr) + 10 + uip_htons(ans->len);
    --nanswers;
  }

  /* Got to this point there's no answer, try next nameserver if available
     since this one doesn't know the answer */
#if RESOLV_CONF_SUPPORTS_MDNS
  if(nanswers == 0 && UIP_UDP_BUF->srcport != UIP_HTONS(MDNS_PORT) 
      && hdr->id != 0)
#else
  if(nanswers == 0) 
#endif
  {
    if(try_next_server(namemapptr)) {
      namemapptr->state = STATE_ASKING;
      process_post(&resolv_process, PROCESS_EVENT_TIMER, NULL);
    }
  }

}
/*---------------------------------------------------------------------------*/
#if RESOLV_CONF_SUPPORTS_MDNS
/**
 * \brief           Changes the local hostname advertised by MDNS.
 * \param hostname  The new hostname to advertise.
 */
void
resolv_set_hostname(const char *hostname)
{
  strncpy(resolv_hostname, hostname, RESOLV_CONF_MAX_DOMAIN_NAME_SIZE);

  /* Add the .local suffix if it isn't already there */
  if(strlen(resolv_hostname) < 7 ||
     strcasecmp(resolv_hostname + strlen(resolv_hostname) - 6, ".local") != 0) {
    strncat(resolv_hostname, ".local", RESOLV_CONF_MAX_DOMAIN_NAME_SIZE);
  }

  PRINTF("resolver: hostname changed to \"%s\"\n", resolv_hostname);

  start_name_collision_check(0);
}
/*---------------------------------------------------------------------------*/
/**
 * \brief      Returns the local hostname being advertised via MDNS.
 * \return     C-string containing the local hostname.
 */
const char *
resolv_get_hostname(void)
{
  return resolv_hostname;
}
/*---------------------------------------------------------------------------*/
/** \internal
 * Process for probing for name conflicts.
 */
PROCESS_THREAD(mdns_probe_process, ev, data)
{
  static struct etimer delay;

  PROCESS_BEGIN();
  mdns_state = MDNS_STATE_WAIT_BEFORE_PROBE;

  PRINTF("mdns-probe: Process (re)started.\n");

  /* Wait extra time if specified in data */
  if(NULL != data) {
    PRINTF("mdns-probe: Probing will begin in %ld clocks.\n",
           (long)*(clock_time_t *) data);
    etimer_set(&delay, *(clock_time_t *) data);
    PROCESS_WAIT_EVENT_UNTIL(ev == PROCESS_EVENT_TIMER);
  }

  /* We need to wait a random (0-250ms) period of time before
   * probing to be in compliance with the MDNS spec. */
  etimer_set(&delay, CLOCK_SECOND * (random_rand() & 0xFF) / 1024);
  PROCESS_WAIT_EVENT_UNTIL(ev == PROCESS_EVENT_TIMER);

  /* Begin searching for our name. */
  mdns_state = MDNS_STATE_PROBING;
  resolv_query(resolv_hostname);

  do {
    PROCESS_WAIT_EVENT_UNTIL(ev == resolv_event_found);
  } while(strcasecmp(resolv_hostname, data) != 0);

  mdns_state = MDNS_STATE_READY;
  mdns_announce_requested();

  PRINTF("mdns-probe: Finished probing.\n");

  PROCESS_END();
}
#endif /* RESOLV_CONF_SUPPORTS_MDNS */
/*---------------------------------------------------------------------------*/
/** \internal
 * The main UDP function.
 */
PROCESS_THREAD(resolv_process, ev, data)
{
  PROCESS_BEGIN();

  memset(names, 0, sizeof(names));

  resolv_event_found = process_alloc_event();

  PRINTF("resolver: Process started.\n");

  resolv_conn = udp_new(NULL, 0, NULL);

#if RESOLV_CONF_SUPPORTS_MDNS
  PRINTF("resolver: Supports MDNS.\n");
  uip_udp_bind(resolv_conn, UIP_HTONS(MDNS_PORT));

#if NETSTACK_CONF_WITH_IPV6
  uip_ds6_maddr_add(&resolv_mdns_addr);
#else
  /* TODO: Is there anything we need to do here for IPv4 multicast? */
#endif

  resolv_set_hostname(CONTIKI_CONF_DEFAULT_HOSTNAME);
#endif /* RESOLV_CONF_SUPPORTS_MDNS */

  while(1) {
    PROCESS_WAIT_EVENT();

    if(ev == PROCESS_EVENT_TIMER) {
      tcpip_poll_udp(resolv_conn);
    } else if(ev == tcpip_event) {
      if(uip_udp_conn == resolv_conn) {
        if(uip_newdata()) {
          newdata();
        }
        if(uip_poll()) {
#if RESOLV_CONF_SUPPORTS_MDNS
          if(mdns_needs_host_announce) {
            size_t len;

            PRINTF("resolver: Announcing that we are \"%s\".\n",
                   resolv_hostname);

            memset(uip_appdata, 0, sizeof(struct dns_hdr));

            len = mdns_prep_host_announce_packet();

            uip_udp_packet_sendto(resolv_conn, uip_appdata,
                                  len, &resolv_mdns_addr, UIP_HTONS(MDNS_PORT));

            mdns_needs_host_announce = 0;

            /* Poll again in case this fired
             * at the same time the event timer did.
             */
            tcpip_poll_udp(resolv_conn);
          } else
#endif /* RESOLV_CONF_SUPPORTS_MDNS */
          {
            check_entries();
          }
        }
      }
    }

#if RESOLV_CONF_SUPPORTS_MDNS
    if(mdns_needs_host_announce) {
      tcpip_poll_udp(resolv_conn);
    }
#endif /* RESOLV_CONF_SUPPORTS_MDNS */
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
static void
init(void)
{
  static uint8_t initialized = 0;
  if(!initialized) {
    process_start(&resolv_process, NULL);
    initialized = 1;
  }
}
/*---------------------------------------------------------------------------*/
#if RESOLV_AUTO_REMOVE_TRAILING_DOTS
static const char *
remove_trailing_dots(const char *name) {
  static char dns_name_without_dots[RESOLV_CONF_MAX_DOMAIN_NAME_SIZE + 1];
  size_t len = strlen(name);

  if(name[len - 1] == '.') {
    strncpy(dns_name_without_dots, name, sizeof(dns_name_without_dots));
    while(len && (dns_name_without_dots[len - 1] == '.')) {
      dns_name_without_dots[--len] = 0;
    }
    name = dns_name_without_dots;
  }
  return name;
}
#else /* RESOLV_AUTO_REMOVE_TRAILING_DOTS */
#define remove_trailing_dots(x) (x)
#endif /* RESOLV_AUTO_REMOVE_TRAILING_DOTS */
/*---------------------------------------------------------------------------*/
/**
 * Queues a name so that a question for the name will be sent out.
 *
 * \param name The hostname that is to be queried.
 */
void
resolv_query(const char *name)
{
  static uint8_t i;

  static uint8_t lseq, lseqi;

  register struct namemap *nameptr = 0;

  init();
  
  lseq = lseqi = 0;

  /* Remove trailing dots, if present. */
  name = remove_trailing_dots(name);

  for(i = 0; i < RESOLV_ENTRIES; ++i) {
    nameptr = &names[i];
    if(0 == strcasecmp(nameptr->name, name)) {
      break;
    }
    if((nameptr->state == STATE_UNUSED)
#if RESOLV_SUPPORTS_RECORD_EXPIRATION
      || (nameptr->state == STATE_DONE && clock_seconds() > nameptr->expiration)
#endif /* RESOLV_SUPPORTS_RECORD_EXPIRATION */
    ) {
      lseqi = i;
      lseq = 255;
    } else if(seqno - nameptr->seqno > lseq) {
      lseq = seqno - nameptr->seqno;
      lseqi = i;
    }
  }

  if(i == RESOLV_ENTRIES) {
    i = lseqi;
    nameptr = &names[i];
  }

  PRINTF("resolver: Starting query for \"%s\".\n", name);

  memset(nameptr, 0, sizeof(*nameptr));

  strncpy(nameptr->name, name, sizeof(nameptr->name));
  nameptr->state = STATE_NEW;
  nameptr->seqno = seqno;
  ++seqno;

#if RESOLV_CONF_SUPPORTS_MDNS
  {
    size_t name_len = strlen(name);

    static const char local_suffix[] = "local";

    if((name_len > (sizeof(local_suffix) - 1)) &&
       (0 == strcasecmp(name + name_len - (sizeof(local_suffix) - 1), local_suffix))) {
      PRINTF("resolver: Using MDNS to look up \"%s\".\n", name);
      nameptr->is_mdns = 1;
    } else {
      nameptr->is_mdns = 0;
    }
  }
  nameptr->is_probe = (mdns_state == MDNS_STATE_PROBING) &&
                      (0 == strcmp(nameptr->name, resolv_hostname));
#endif /* RESOLV_CONF_SUPPORTS_MDNS */

  /* Force check_entires() to run on our process. */
  process_post(&resolv_process, PROCESS_EVENT_TIMER, 0);
}
/*---------------------------------------------------------------------------*/
/**
 * Look up a hostname in the array of known hostnames.
 *
 * \note This function only looks in the internal array of known
 * hostnames, it does not send out a query for the hostname if none
 * was found. The function resolv_query() can be used to send a query
 * for a hostname.
 *
 */
resolv_status_t
resolv_lookup(const char *name, uip_ipaddr_t ** ipaddr)
{
  resolv_status_t ret = RESOLV_STATUS_UNCACHED;

  static uint8_t i;

  struct namemap *nameptr;

  /* Remove trailing dots, if present. */
  name = remove_trailing_dots(name);

#if UIP_CONF_LOOPBACK_INTERFACE
  if(strcmp(name, "localhost")) {
    static uip_ipaddr_t loopback =
#if NETSTACK_CONF_WITH_IPV6
    { { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01 } };
#else /* NETSTACK_CONF_WITH_IPV6 */
    { { 127, 0, 0, 1 } };
#endif /* NETSTACK_CONF_WITH_IPV6 */
    if(ipaddr) {
      *ipaddr = &loopback;
    }
    ret = RESOLV_STATUS_CACHED;
  }
#endif /* UIP_CONF_LOOPBACK_INTERFACE */

  /* Walk through the list to see if the name is in there. */
  for(i = 0; i < RESOLV_ENTRIES; ++i) {
    nameptr = &names[i];

    if(strcasecmp(name, nameptr->name) == 0) {
      switch (nameptr->state) {
      case STATE_DONE:
        ret = RESOLV_STATUS_CACHED;
#if RESOLV_SUPPORTS_RECORD_EXPIRATION
        if(clock_seconds() > nameptr->expiration) {
          ret = RESOLV_STATUS_EXPIRED;
        }
#endif /* RESOLV_SUPPORTS_RECORD_EXPIRATION */
        break;
      case STATE_NEW:
      case STATE_ASKING:
        ret = RESOLV_STATUS_RESOLVING;
        break;
      /* Almost certainly a not-found error from server */
      case STATE_ERROR:
        ret = RESOLV_STATUS_NOT_FOUND;
#if RESOLV_SUPPORTS_RECORD_EXPIRATION
        if(clock_seconds() > nameptr->expiration) {
          ret = RESOLV_STATUS_UNCACHED;
        }
#endif /* RESOLV_SUPPORTS_RECORD_EXPIRATION */
        break;
      }

      if(ipaddr) {
        *ipaddr = &nameptr->ipaddr;
      }

      /* Break out of for loop. */
      break;
    }
  }

#if VERBOSE_DEBUG
  switch (ret) {
  case RESOLV_STATUS_CACHED:
    if(ipaddr) {
      PRINTF("resolver: Found \"%s\" in cache.\n", name);
      const uip_ipaddr_t *addr = *ipaddr;

      DEBUG_PRINTF
        ("resolver: %02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x \n",
         ((uint8_t *) addr)[0], ((uint8_t *) addr)[1], ((uint8_t *) addr)[2],
         ((uint8_t *) addr)[3], ((uint8_t *) addr)[4], ((uint8_t *) addr)[5],
         ((uint8_t *) addr)[6], ((uint8_t *) addr)[7], ((uint8_t *) addr)[8],
         ((uint8_t *) addr)[9], ((uint8_t *) addr)[10],
         ((uint8_t *) addr)[11], ((uint8_t *) addr)[12],
         ((uint8_t *) addr)[13], ((uint8_t *) addr)[14],
         ((uint8_t *) addr)[15]);
      break;
    }
  default:
    DEBUG_PRINTF("resolver: \"%s\" is NOT cached.\n", name);
    break;
  }
#endif /* VERBOSE_DEBUG */

  return ret;
}
/*---------------------------------------------------------------------------*/
/** \internal
 * Callback function which is called when a hostname is found.
 *
 */
static void
resolv_found(char *name, uip_ipaddr_t * ipaddr)
{
#if RESOLV_CONF_SUPPORTS_MDNS
  if(strncasecmp(resolv_hostname, name, strlen(resolv_hostname)) == 0 &&
     ipaddr
#if NETSTACK_CONF_WITH_IPV6
     && !uip_ds6_is_my_addr(ipaddr)
#else
     && uip_ipaddr_cmp(&uip_hostaddr, ipaddr) != 0
#endif
    ) {
    uint8_t i;

    if(mdns_state == MDNS_STATE_PROBING) {
      /* We found this new name while probing.
       * We must now rename ourselves.
       */
      PRINTF("resolver: Name collision detected for \"%s\".\n", name);

      /* Remove the ".local" suffix. */
      resolv_hostname[strlen(resolv_hostname) - 6] = 0;

      /* Append the last three hex parts of the link-level address. */
      for(i = 0; i < 3; ++i) {
        uint8_t val = uip_lladdr.addr[(UIP_LLADDR_LEN - 3) + i];

        char append_str[4] = "-XX";

        append_str[2] = (((val & 0xF) > 9) ? 'a' : '0') + (val & 0xF);
        val >>= 4;
        append_str[1] = (((val & 0xF) > 9) ? 'a' : '0') + (val & 0xF);
        strncat(resolv_hostname, append_str,
                sizeof(resolv_hostname) - strlen(resolv_hostname) - 1); /* -1 in order to fit the terminating null byte. */
      }

      /* Re-add the .local suffix */
      strncat(resolv_hostname, ".local", RESOLV_CONF_MAX_DOMAIN_NAME_SIZE);

      start_name_collision_check(CLOCK_SECOND * 5);
    } else if(mdns_state == MDNS_STATE_READY) {
      /* We found a collision after we had already asserted
       * that we owned this name. We need to immediately
       * and explicitly begin probing.
       */
      PRINTF("resolver: Possible name collision, probing...\n");
      start_name_collision_check(0);
    }

  } else
#endif /* RESOLV_CONF_SUPPORTS_MDNS */

#if VERBOSE_DEBUG
  if(ipaddr) {
    PRINTF("resolver: Found address for \"%s\".\n", name);
    PRINTF
      ("resolver: %02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x \n",
       ((uint8_t *) ipaddr)[0], ((uint8_t *) ipaddr)[1],
       ((uint8_t *) ipaddr)[2], ((uint8_t *) ipaddr)[3],
       ((uint8_t *) ipaddr)[4], ((uint8_t *) ipaddr)[5],
       ((uint8_t *) ipaddr)[6], ((uint8_t *) ipaddr)[7],
       ((uint8_t *) ipaddr)[8], ((uint8_t *) ipaddr)[9],
       ((uint8_t *) ipaddr)[10], ((uint8_t *) ipaddr)[11],
       ((uint8_t *) ipaddr)[12], ((uint8_t *) ipaddr)[13],
       ((uint8_t *) ipaddr)[14], ((uint8_t *) ipaddr)[15]);
  } else {
    PRINTF("resolver: Unable to retrieve address for \"%s\".\n", name);
  }
#endif /* VERBOSE_DEBUG */

  process_post(PROCESS_BROADCAST, resolv_event_found, name);
}
/*---------------------------------------------------------------------------*/
#endif /* UIP_UDP */

/** @} */
/** @} */

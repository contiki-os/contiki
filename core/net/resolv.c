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

/**
 * \file
 * DNS host name to IP address resolver.
 * \author Adam Dunkels <adam@dunkels.com>
 *
 * This file implements a DNS host name to IP address resolver.
 */

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

#include "net/tcpip.h"
#include "net/resolv.h"
#if UIP_UDP

#include <string.h>

#ifndef NULL
#define NULL (void *)0
#endif /* NULL */

#if UIP_CONF_IPV6

/* Currently this implementation only supports IPv4 DNS lookups.
   Until support for IPv6 is added, dummy functions are used to
   enable compilation with IPv6.
*/

process_event_t resolv_event_found;

PROCESS(resolv_process, "DNS resolver");

void resolv_conf(const uip_ipaddr_t *dnsserver) { }
uip_ipaddr_t *resolv_getserver(void) { return NULL; }
uip_ipaddr_t *resolv_lookup(const char *name) { return NULL; }
void resolv_query(const char *name) { }

PROCESS_THREAD(resolv_process, ev, data)
{
  PROCESS_BEGIN();
  resolv_event_found = process_alloc_event();
  PROCESS_END();
}

#else /* UIP_CONF_IPV6 */

/** \internal The maximum number of retries when asking for a name. */
#define MAX_RETRIES 8

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
     to a name already present somewhere in the packet. */
  uint16_t type;
  uint16_t class;
  uint16_t ttl[2];
  uint16_t len;
  uint8_t ipaddr[4];
};

struct namemap {
#define STATE_UNUSED 0
#define STATE_NEW    1
#define STATE_ASKING 2
#define STATE_DONE   3
#define STATE_ERROR  4
  uint8_t state;
  uint8_t tmr;
  uint8_t retries;
  uint8_t seqno;
  uint8_t err;
  char name[32];
  uip_ipaddr_t ipaddr;
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

static void resolv_found(char *name, uip_ipaddr_t *ipaddr);

enum {
  EVENT_NEW_SERVER=0
};

/*-----------------------------------------------------------------------------------*/
/** \internal
 * Walk through a compact encoded DNS name and return the end of it.
 *
 * \return The end of the name.
 */
/*-----------------------------------------------------------------------------------*/
static unsigned char *
parse_name(unsigned char *query)
{
  unsigned char n;

  do {
    n = *query++;
    
    while(n > 0) {
      /*      printf("%c", *query);*/
      ++query;
      --n;
    };
    /*    printf(".");*/
  } while(*query != 0);
  /*  printf("\n");*/
  return query + 1;
}
/*-----------------------------------------------------------------------------------*/
/** \internal
 * Runs through the list of names to see if there are any that have
 * not yet been queried and, if so, sends out a query.
 */
/*-----------------------------------------------------------------------------------*/
static void
check_entries(void)
{
  register struct dns_hdr *hdr;
  char *query, *nptr, *nameptr;
  uint8_t i;
  uint8_t n;
  register struct namemap *namemapptr;
  
  for(i = 0; i < RESOLV_ENTRIES; ++i) {
    namemapptr = &names[i];
    if(namemapptr->state == STATE_NEW ||
       namemapptr->state == STATE_ASKING) {
      etimer_set(&retry, CLOCK_SECOND);
      if(namemapptr->state == STATE_ASKING) {
	if(--namemapptr->tmr == 0) {
	  if(++namemapptr->retries == MAX_RETRIES) {
	    namemapptr->state = STATE_ERROR;
	    resolv_found(namemapptr->name, NULL);
	    continue;
	  }
	  namemapptr->tmr = namemapptr->retries;
	} else {
	  /*	  printf("Timer %d\n", namemapptr->tmr);*/
	  /* Its timer has not run out, so we move on to next
	     entry. */
	  continue;
	}
      } else {
	namemapptr->state = STATE_ASKING;
	namemapptr->tmr = 1;
	namemapptr->retries = 0;
      }
      hdr = (struct dns_hdr *)uip_appdata;
      memset(hdr, 0, sizeof(struct dns_hdr));
      hdr->id = uip_htons(i);
      hdr->flags1 = DNS_FLAG1_RD;
      hdr->numquestions = UIP_HTONS(1);
      query = (char *)uip_appdata + 12;
      nameptr = namemapptr->name;
      --nameptr;
      /* Convert hostname into suitable query format. */
      do {
	++nameptr;
	nptr = query;
	++query;
	for(n = 0; *nameptr != '.' && *nameptr != 0; ++nameptr) {
	  *query = *nameptr;
	  ++query;
	  ++n;
	}
	*nptr = n;
      } while(*nameptr != 0);
      {
	static unsigned char endquery[] =
	  {0,0,1,0,1};
	memcpy(query, endquery, 5);
      }
      uip_udp_send((unsigned char)(query + 5 - (char *)uip_appdata));
      break;
    }
  }
}
/*-----------------------------------------------------------------------------------*/
/** \internal
 * Called when new UDP data arrives.
 */
/*-----------------------------------------------------------------------------------*/
static void
newdata(void)
{
  unsigned char *nameptr;
  struct dns_answer *ans;
  struct dns_hdr *hdr;
  static uint8_t nquestions, nanswers;
  static uint8_t i;
  register struct namemap *namemapptr;
  
  hdr = (struct dns_hdr *)uip_appdata;
  /*  printf("ID %d\n", uip_htons(hdr->id));
  printf("Query %d\n", hdr->flags1 & DNS_FLAG1_RESPONSE);
  printf("Error %d\n", hdr->flags2 & DNS_FLAG2_ERR_MASK);
  printf("Num questions %d, answers %d, authrr %d, extrarr %d\n",
	 uip_htons(hdr->numquestions),
	 uip_htons(hdr->numanswers),
	 uip_htons(hdr->numauthrr),
	 uip_htons(hdr->numextrarr));
  */

  /* The ID in the DNS header should be our entry into the name
     table. */
  i = (uint8_t)uip_htons(hdr->id);
  namemapptr = &names[i];
  if(i < RESOLV_ENTRIES &&
     namemapptr->state == STATE_ASKING) {

    /* This entry is now finished. */
    namemapptr->state = STATE_DONE;
    namemapptr->err = hdr->flags2 & DNS_FLAG2_ERR_MASK;

    /* Check for error. If so, call callback to inform. */
    if(namemapptr->err != 0) {
      namemapptr->state = STATE_ERROR;
      resolv_found(namemapptr->name, NULL);
      return;
    }

    /* We only care about the question(s) and the answers. The authrr
       and the extrarr are simply discarded. */
    nquestions = (uint8_t)uip_htons(hdr->numquestions);
    nanswers = (uint8_t)uip_htons(hdr->numanswers);

    /* Skip the name in the question. XXX: This should really be
       checked agains the name in the question, to be sure that they
       match. */
    nameptr = parse_name((uint8_t *)uip_appdata + 12) + 4;

    while(nanswers > 0) {
      /* The first byte in the answer resource record determines if it
	 is a compressed record or a normal one. */
      if(*nameptr & 0xc0) {
	/* Compressed name. */
	nameptr +=2;
	/*	printf("Compressed anwser\n");*/
      } else {
	/* Not compressed name. */
	nameptr = parse_name((uint8_t *)nameptr);
      }

      ans = (struct dns_answer *)nameptr;
      /*      printf("Answer: type %x, class %x, ttl %x, length %x\n",
	     uip_htons(ans->type), uip_htons(ans->class), (uip_htons(ans->ttl[0])
	     << 16) | uip_htons(ans->ttl[1]), uip_htons(ans->len));*/

      /* Check for IP address type and Internet class. Others are
	 discarded. */
      if(ans->type == UIP_HTONS(1) &&
	 ans->class == UIP_HTONS(1) &&
	 ans->len == UIP_HTONS(4)) {
	/*	printf("IP address %d.%d.%d.%d\n",
	       ans->ipaddr[0],
	       ans->ipaddr[1],
	       ans->ipaddr[2],
	       ans->ipaddr[3]);*/
	/* XXX: we should really check that this IP address is the one
	   we want. */
        for(i = 0; i < 4; i++) {
          namemapptr->ipaddr.u8[i] = ans->ipaddr[i];
        }
	
	resolv_found(namemapptr->name, &namemapptr->ipaddr);
	return;
      } else {
	nameptr = nameptr + 10 + uip_htons(ans->len);
      }
      --nanswers;
    }
  }
}
/*-----------------------------------------------------------------------------------*/
/** \internal
 * The main UDP function.
 */
/*-----------------------------------------------------------------------------------*/
PROCESS_THREAD(resolv_process, ev, data)
{
  int i;
  
  PROCESS_BEGIN();

  for(i = 0; i < RESOLV_ENTRIES; ++i) {
    names[i].state = STATE_UNUSED;
  }
  resolv_conn = NULL;
  resolv_event_found = process_alloc_event();
  
  
  while(1) {
    PROCESS_WAIT_EVENT();
    
    if(ev == PROCESS_EVENT_TIMER) {
      if(resolv_conn != NULL) {
        tcpip_poll_udp(resolv_conn);
      }

    } else if(ev == EVENT_NEW_SERVER) {
      if(resolv_conn != NULL) {
	uip_udp_remove(resolv_conn);
      }
      resolv_conn = udp_new((uip_ipaddr_t *)data, UIP_HTONS(53), NULL);
      
    } else if(ev == tcpip_event) {
      if(uip_udp_conn->rport == UIP_HTONS(53)) {
	if(uip_poll()) {
	  check_entries();
	}
	if(uip_newdata()) {
	  newdata();
	}
      }
    }
  }
  
  PROCESS_END();
}
/*-----------------------------------------------------------------------------------*/
/**
 * Queues a name so that a question for the name will be sent out.
 *
 * \param name The hostname that is to be queried.
 */
/*-----------------------------------------------------------------------------------*/
void
resolv_query(const char *name)
{
  static uint8_t i;
  static uint8_t lseq, lseqi;
  register struct namemap *nameptr;
      
  lseq = lseqi = 0;
  nameptr = 0;                //compiler warning if not initialized
  
  for(i = 0; i < RESOLV_ENTRIES; ++i) {
    nameptr = &names[i];
    if(nameptr->state == STATE_UNUSED) {
      break;
    }
    if(seqno - nameptr->seqno > lseq) {
      lseq = seqno - nameptr->seqno;
      lseqi = i;
    }
  }

  if(i == RESOLV_ENTRIES) {
    i = lseqi;
    nameptr = &names[i];
  }

  strncpy(nameptr->name, name, sizeof(nameptr->name));
  nameptr->state = STATE_NEW;
  nameptr->seqno = seqno;
  ++seqno;

  if(resolv_conn != NULL) {
    tcpip_poll_udp(resolv_conn);
  }
}
/*-----------------------------------------------------------------------------------*/
/**
 * Look up a hostname in the array of known hostnames.
 *
 * \note This function only looks in the internal array of known
 * hostnames, it does not send out a query for the hostname if none
 * was found. The function resolv_query() can be used to send a query
 * for a hostname.
 *
 * \return A pointer to a 4-byte representation of the hostname's IP
 * address, or NULL if the hostname was not found in the array of
 * hostnames.
 */
/*-----------------------------------------------------------------------------------*/
uip_ipaddr_t *
resolv_lookup(const char *name)
{
  static uint8_t i;
  struct namemap *nameptr;
  
  /* Walk through the list to see if the name is in there. If it is
     not, we return NULL. */
  for(i = 0; i < RESOLV_ENTRIES; ++i) {
    nameptr = &names[i];
    if(nameptr->state == STATE_DONE &&
       strcmp(name, nameptr->name) == 0) {
      return &nameptr->ipaddr;
    }
  }
  return NULL;
}
/*-----------------------------------------------------------------------------------*/
/**
 * Obtain the currently configured DNS server.
 *
 * \return A pointer to a 4-byte representation of the IP address of
 * the currently configured DNS server or NULL if no DNS server has
 * been configured.
 */
/*-----------------------------------------------------------------------------------*/
uip_ipaddr_t *
resolv_getserver(void)
{
  if(resolv_conn == NULL) {
    return NULL;
  }
  return &resolv_conn->ripaddr;
}
/*-----------------------------------------------------------------------------------*/
/**
 * Configure a DNS server.
 *
 * \param dnsserver A pointer to a 4-byte representation of the IP
 * address of the DNS server to be configured.
 */
/*-----------------------------------------------------------------------------------*/
void
resolv_conf(const uip_ipaddr_t *dnsserver)
{
  static uip_ipaddr_t server;
  uip_ipaddr_copy(&server, dnsserver);
  process_post(&resolv_process, EVENT_NEW_SERVER, &server);
  
  /*  if(resolv_conn != NULL) {
    uip_udp_remove(resolv_conn);
  }
  
  resolv_conn = udp_new(dnsserver, 53, NULL);*/
}
/*-----------------------------------------------------------------------------------*/
/** \internal
 * Callback function which is called when a hostname is found.
 *
 */
/*-----------------------------------------------------------------------------------*/
static void
resolv_found(char *name, uip_ipaddr_t *ipaddr)
{
  process_post(PROCESS_BROADCAST, resolv_event_found, name);
}
/*-----------------------------------------------------------------------------------*/
#endif /* UIP_CONF_IPV6 */
#endif /* UIP_UDP */

/** @} */
/** @} */

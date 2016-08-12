/*
 * Copyright (c) 2014, Thingsquare, http://www.thingsquare.com/.
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
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#include "ip64.h"
#include "ip64-addr.h"
#include "ip64-dns64.h"

#include <stdio.h>

#define DEBUG 0

#if DEBUG
#undef PRINTF
#define PRINTF(...) printf(__VA_ARGS__)
#else /* DEBUG */
#define PRINTF(...)
#endif /* DEBUG */

struct dns_hdr {
  uint8_t id[2];
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
  uint8_t numquestions[2];
  uint8_t numanswers[2];
  uint8_t numauthrr[2];
  uint8_t numextrarr[2];
};

#define DNS_QUESTION_TYPE0  0
#define DNS_QUESTION_TYPE1  1
#define DNS_QUESTION_CLASS0 2
#define DNS_QUESTION_CLASS1 3
#define DNS_QUESTION_SIZE   4

struct dns_answer {
  /* DNS answer record starts with either a domain name or a pointer
   * to a name already present somewhere in the packet. */
  uint8_t type[2];
  uint8_t class[2];
  uint8_t ttl[4];
  uint8_t len[2];
  union {
    uint8_t ip6[16];
    uint8_t ip4[4];
  } addr;
};

#define DNS_TYPE_A      1
#define DNS_TYPE_AAAA  28

#define DNS_CLASS_IN    1
#define DNS_CLASS_ANY 255

/*---------------------------------------------------------------------------*/
void
ip64_dns64_6to4(const uint8_t *ipv6data, int ipv6datalen,
                uint8_t *ipv4data, int ipv4datalen)
{
  int i, j;
  int qlen;
  uint8_t *qdata;
  uint8_t *q;
  struct dns_hdr *hdr;

  hdr = (struct dns_hdr *)ipv4data;
  PRINTF("ip64_dns64_6to4 id: %02x%02x\n", hdr->id[0], hdr->id[1]);
  PRINTF("ip64_dns64_6to4 flags1: 0x%02x\n", hdr->flags1);
  PRINTF("ip64_dns64_6to4 flags2: 0x%02x\n", hdr->flags2);
  PRINTF("ip64_dns64_6to4 numquestions: 0x%02x\n", ((hdr->numquestions[0] << 8) + hdr->numquestions[1]));
  PRINTF("ip64_dns64_6to4 numanswers: 0x%02x\n", ((hdr->numanswers[0] << 8) + hdr->numanswers[1]));
  PRINTF("ip64_dns64_6to4 numauthrr: 0x%02x\n", ((hdr->numauthrr[0] << 8) + hdr->numauthrr[1]));
  PRINTF("ip64_dns64_6to4 numextrarr: 0x%02x\n", ((hdr->numextrarr[0] << 8) + hdr->numextrarr[1]));

  /* Find the DNS question header by scanning through the question
     labels. */
  qdata = ipv4data + sizeof(struct dns_hdr);
  for(i = 0; i < ((hdr->numquestions[0] << 8) + hdr->numquestions[1]); i++) {
    do {
      qlen = *qdata;
      qdata++;
      for(j = 0; j < qlen; j++) {
        qdata++;
        if(qdata > ipv4data + ipv4datalen) {
          PRINTF("ip64_dns64_6to4: packet ended while parsing\n");
          return;
        }
      }
    } while(qlen != 0);
    q = qdata;
    if(q[DNS_QUESTION_CLASS0] == 0 && q[DNS_QUESTION_CLASS1] == DNS_CLASS_IN &&
       q[DNS_QUESTION_TYPE0] == 0 && q[DNS_QUESTION_TYPE1] == DNS_TYPE_AAAA) {
      q[DNS_QUESTION_TYPE1] = DNS_TYPE_A;
    }

    qdata += DNS_QUESTION_SIZE;
  }
}
/*---------------------------------------------------------------------------*/
int
ip64_dns64_4to6(const uint8_t *ipv4data, int ipv4datalen,
                uint8_t *ipv6data, int ipv6datalen)
{
  uint8_t n;
  int i, j;
  int qlen, len;
  const uint8_t *qdata, *adata;
  uint8_t *qcopy, *acopy, *lenptr;
  uint8_t *q;
  struct dns_hdr *hdr;

  hdr = (struct dns_hdr *)ipv4data;
  PRINTF("ip64_dns64_4to6 id: %02x%02x\n", hdr->id[0], hdr->id[1]);
  PRINTF("ip64_dns64_4to6 flags1: 0x%02x\n", hdr->flags1);
  PRINTF("ip64_dns64_4to6 flags2: 0x%02x\n", hdr->flags2);
  PRINTF("ip64_dns64_4to6 numquestions: 0x%02x\n", ((hdr->numquestions[0] << 8) + hdr->numquestions[1]));
  PRINTF("ip64_dns64_4to6 numanswers: 0x%02x\n", ((hdr->numanswers[0] << 8) + hdr->numanswers[1]));
  PRINTF("ip64_dns64_4to6 numauthrr: 0x%02x\n", ((hdr->numauthrr[0] << 8) + hdr->numauthrr[1]));
  PRINTF("ip64_dns64_4to6 numextrarr: 0x%02x\n", ((hdr->numextrarr[0] << 8) + hdr->numextrarr[1]));

  /* Find the DNS answer header by scanning through the question
     labels. */
  qdata = ipv4data + sizeof(struct dns_hdr);
  qcopy = ipv6data + sizeof(struct dns_hdr);
  for(i = 0; i < ((hdr->numquestions[0] << 8) + hdr->numquestions[1]); i++) {
    do {
      qlen = *qdata;
      qdata++;
      qcopy++;
      for(j = 0; j < qlen; j++) {
        qdata++;
        qcopy++;
        if(qdata > ipv4data + ipv4datalen) {
          PRINTF("ip64_dns64_4to6: packet ended while parsing\n");
          return ipv6datalen;
        }
      }
    } while(qlen != 0);
    q = qcopy;
    if(q[DNS_QUESTION_CLASS0] == 0 && q[DNS_QUESTION_CLASS1] == DNS_CLASS_IN &&
       q[DNS_QUESTION_TYPE0] == 0 && q[DNS_QUESTION_TYPE1] == DNS_TYPE_AAAA) {
      q[DNS_QUESTION_TYPE1] = DNS_TYPE_AAAA;
    }

    qdata += DNS_QUESTION_SIZE;
    qcopy += DNS_QUESTION_SIZE;
  }

  adata = qdata;
  acopy = qcopy;

  /* Go through the answers section and update the answers. */
  for(i = 0; i < ((hdr->numanswers[0] << 8) + hdr->numanswers[1]); i++) {

    n = *adata;
    if(n & 0xc0) {
      /* Short-hand name format: 2 bytes */
      *acopy++ = *adata++;
      *acopy++ = *adata++;
    } else {
      /* Name spelled out */
      do {
        n = *adata;
        adata++;
        acopy++;
        for(j = 0; j < n; j++) {
          *acopy++ = *adata++;
        }
      } while(n != 0);
    }

    if(adata[0] == 0 && adata[1] == DNS_TYPE_A) {
      /* Update the type field from A to AAAA */
      *acopy = *adata;
      acopy++;
      adata++;
      *acopy = DNS_TYPE_AAAA;
      acopy++;
      adata++;

      /* Get the length of the address record. Should be 4. */
      lenptr = &acopy[6];
      len = (adata[6] << 8) + adata[7];

      /* Copy the class, the TTL, and the data length */
      memcpy(acopy, adata, 2 + 4 + 2);
      acopy += 8;
      adata += 8;

      if(len == 4) {
        uip_ip4addr_t addr;
        uip_ipaddr(&addr, adata[0], adata[1], adata[2], adata[3]);
        ip64_addr_4to6(&addr, (uip_ip6addr_t *)acopy);

        adata += len;
        acopy += 16;
        lenptr[0] = 0;
        lenptr[1] = 16;
        ipv6datalen += 12;

      } else {
        memcpy(acopy, adata, len);
        acopy += len;
        adata += len;
      }
    } else {
      len = (adata[8] << 8) + adata[9];

      /* Copy the type, class, the TTL, and the data length */
      memcpy(acopy, adata, 2 + 2 + 4 + 2);
      acopy += 10;
      adata += 10;

      /* Copy the data */
      memcpy(acopy, adata, len);
      acopy += len;
      adata += len;
    }
  }
  return ipv6datalen;
}
/*---------------------------------------------------------------------------*/

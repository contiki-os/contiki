/**
 * \addtogroup sicslowpan
 * @{
 */

/*
 * Copyright (c) 2008, Swedish Institute of Computer Science.
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
 * This file is part of the Contiki operating system.
 *
 * $Id: sicslowpan.h,v 1.5 2010/01/28 13:50:51 adamdunkels Exp $
 */
/**
 * \file
 *         Header file for the 6lowpan implementation
 *         (RFC4944 and draft-hui-6lowpan-hc-01)
 * \author Adam Dunkels <adam@sics.se>
 * \author Nicolas Tsiftes <nvt@sics.se>
 * \author Niclas Finne <nfi@sics.se>
 * \author Mathilde Durvy <mdurvy@cisco.com>
 * \author Julien Abeille <jabeille@cisco.com>
 */

#ifndef __SICSLOWPAN_H__
#define __SICSLOWPAN_H__
#include "net/uip.h"
#include "net/mac/mac.h"

/**
 * \name General sicslowpan defines
 * @{
 */
/* Min and Max compressible UDP ports */
#define SICSLOWPAN_UDP_PORT_MIN                     0xF0B0
#define SICSLOWPAN_UDP_PORT_MAX                     0xF0BF   /* F0B0 + 15 */
/** @} */


/**
 * \name 6lowpan dispatches
 * @{
 */
#define SICSLOWPAN_DISPATCH_IPV6                    0x41 /* 01000001 = 65 */
#define SICSLOWPAN_DISPATCH_HC1                     0x42 /* 01000010 = 66 */
#define SICSLOWPAN_DISPATCH_IPHC                    0x03 /* 00000011 = 3 */
#define SICSLOWPAN_DISPATCH_FRAG1                   0xc0 /* 11000xxx */
#define SICSLOWPAN_DISPATCH_FRAGN                   0xe0 /* 11100xxx */
/** @} */

/** \name HC1 encoding
 * @{
 */
#define SICSLOWPAN_HC1_NH_UDP                       0x02
#define SICSLOWPAN_HC1_NH_TCP                       0x06
#define SICSLOWPAN_HC1_NH_ICMP6                     0x04
/** @} */

/** \name HC_UDP encoding (works together with HC1)
 * @{
 */
#define SICSLOWPAN_HC_UDP_ALL_C                     0xE0
/** @} */

/**
 * \name IPHC encoding
 * @{
 */
/*
 * Values of fields within the IPHC encoding first byte
 * (C stands for compressed and I for inline)
 */
#define SICSLOWPAN_IPHC_TC_C                        0x80
#define SICSLOWPAN_IPHC_VF_C                        0x40
#define SICSLOWPAN_IPHC_NH_C                        0x20
#define SICSLOWPAN_IPHC_TTL_1                       0x08
#define SICSLOWPAN_IPHC_TTL_64                      0x10
#define SICSLOWPAN_IPHC_TTL_255                     0x18
#define SICSLOWPAN_IPHC_TTL_I                       0x00

/* Values of fields within the IPHC encoding second byte */
#define SICSLOWPAN_IPHC_SAM_I                       0x00
#define SICSLOWPAN_IPHC_SAM_64                      0x40
#define SICSLOWPAN_IPHC_SAM_16                      0x80
#define SICSLOWPAN_IPHC_SAM_0                       0xC0
#define SICSLOWPAN_IPHC_DAM_I                       0x00
#define SICSLOWPAN_IPHC_DAM_64                      0x04
#define SICSLOWPAN_IPHC_DAM_16                      0x08
#define SICSLOWPAN_IPHC_DAM_0                       0x0C

/* Link local context number */
#define SICSLOWPAN_IPHC_ADDR_CONTEXT_LL             0
/* 16-bit multicast addresses compression */
#define SICSLOWPAN_IPHC_MCAST_RANGE                 0xA0
/** @} */


/**
 * \name LOWPAN_UDP encoding (works together with IPHC)
 * @{
 */
#define SICSLOWPAN_NHC_UDP_ID                       0xF8
#define SICSLOWPAN_NHC_UDP_C                        0xFB
#define SICSLOWPAN_NHC_UDP_I                        0xF8
/** @} */


/**
 * \name The 6lowpan "headers" length
 * @{
 */

#define SICSLOWPAN_IPV6_HDR_LEN                     1    /*one byte*/
#define SICSLOWPAN_HC1_HDR_LEN                      3
#define SICSLOWPAN_HC1_HC_UDP_HDR_LEN               7
#define SICSLOWPAN_FRAG1_HDR_LEN                    4
#define SICSLOWPAN_FRAGN_HDR_LEN                    5
/** @} */

/**
 * \brief The header for fragments
 * \note We do not define different structures for FRAG1
 * and FRAGN headers, which are different. For FRAG1, the
 * offset field is just not used
 */
/* struct sicslowpan_frag_hdr { */
/*   u16_t dispatch_size; */
/*   u16_t tag; */
/*   u8_t offset; */
/* }; */

/**
 * \brief The HC1 header when HC_UDP is not used
 *
 * When all fields are compressed and HC_UDP is not used,
 * we use this structure. If HC_UDP is used, the ttl is
 * in another spot, and we use the sicslowpan_hc1_hc_udp
 * structure
 */
/* struct sicslowpan_hc1_hdr { */
/*   u8_t dispatch; */
/*   u8_t encoding; */
/*   u8_t ttl; */
/* }; */

/**
 * \brief HC1 followed by HC_UDP
 */
/* struct sicslowpan_hc1_hc_udp_hdr { */
/*   u8_t dispatch; */
/*   u8_t hc1_encoding; */
/*   u8_t hc_udp_encoding; */
/*   u8_t ttl; */
/*   u8_t ports; */
/*   u16_t udpchksum; */
/* }; */

/**
 * \brief IPHC dispatch and encoding
 * the rest (uncompressed fields) is variable
 */
struct sicslowpan_iphc_hdr {
  u8_t dispatch;
  u8_t encoding[2];
};

/* struct sicslowpan_nhc_udp_comp_hdr { */
/*   u8_t nhcid; */
/*   u8_t ports; */
/*   u16_t udpchksum; */
/* }; */

/**
 * \brief An address context for IPHC address compression
 */
struct sicslowpan_addr_context {
  u8_t used;
  u8_t number;
  u8_t prefix[8];
};

/**
 * \name Address compressibility test functions
 * @{
 */

/**
 * \brief check whether we can compress the IID in
 * address 'a' to 16 bits.
 * This is used for unicast addresses only, and is true
 * if first 49 bits of IID are 0
 */
#define sicslowpan_is_iid_16_bit_compressable(a) \
  ((((a)->u16[4]) == 0) &&                       \
   (((a)->u16[5]) == 0) &&                       \
   (((a)->u16[6]) == 0) &&                       \
   ((((a)->u8[14]) & 0x80) == 0))

/**
 * \brief check whether the 9-bit group-id of the
 * compressed multicast address is known. It is true
 * if the 9-bit group is the all nodes or all routers
 * group.
 * \param a is typed u8_t *
 */
#define sicslowpan_is_mcast_addr_decompressable(a) \
   (((*a & 0x01) == 0) &&                           \
    ((*(a + 1) == 0x01) || (*(a + 1) == 0x02)))

/**
 * \brief check whether the 112-bit group-id of the
 * multicast address is mappable to a 9-bit group-id
 * It is true if the group is the all nodes or all
 * routers group.
*/
#define sicslowpan_is_mcast_addr_compressable(a) \
  ((((a)->u16[1]) == 0) &&                       \
   (((a)->u16[2]) == 0) &&                       \
   (((a)->u16[3]) == 0) &&                       \
   (((a)->u16[4]) == 0) &&                       \
   (((a)->u16[5]) == 0) &&                       \
   (((a)->u16[6]) == 0) &&                       \
   (((a)->u8[14]) == 0) &&                       \
   ((((a)->u8[15]) == 1) || (((a)->u8[15]) == 2)))

/** @} */

/**
 * \brief 6lowpan init function
 * \param m is the MAC layer "driver"
 */
void sicslowpan_init(const struct mac_driver *m);

extern const struct mac_driver *sicslowpan_mac;
#endif /* __SICSLOWPAN_H__ */
/** @} */

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
 */
/**
 * \file
 *         Code managing id<->mac address<->IPv6 address mapping, and doing this
 *         for different deployment scenarios: Cooja, Nodes, Indriya or Twist testbeds
 *
 * \author Simon Duquennoy <simonduq@sics.se>
 */

#include "contiki-conf.h"
#include "deployment.h"
#include "sys/node-id.h"
#include "net/rpl/rpl.h"
#include "random.h"
#include "ds2411.h"
#include <string.h>

/* Our global IPv6 prefix */
static uip_ipaddr_t prefix;

/* ID<->MAC address mapping */
struct id_mac {
  uint16_t id;
  uint16_t mac;
};

/* List of ID<->MAC mapping used for different deployments */
static const struct id_mac id_mac_list[] = {
#if IN_TWIST
    {202, 0xfd30}, {187, 0xb32b}, {198,0xc823}, {199, 0x9b17}, {13, 0xf600}, {241, 0x3f0a}, {102, 0xc033}, {137, 0xfab2}, {190, 0x4b28}, {148, 0x35d9}, {95, 0x3e4e}, {93, 0xaa65}, {145, 0x97b1}, {139, 0x13a3}, {15, 0x2443}, {87, 0xc6ff}, {90, 0x167b}, {10, 0x0b63}, {82, 0x0649}, {262, 0x29a6}, {220, 0xa53c}, {100, 0x2058}, {228, 0xcc2b}, {230, 0x5a12}, {252, 0x94da}, {142, 0xf0a9}, {141, 0x1ec1}, {224, 0x5331}, {92, 0x9649}, {99, 0xafe6}, {222, 0x3311}, {205, 0xdf22}, {223, 0xfa35}, {189, 0x8622}, {138, 0xe1fc}, {83, 0x68c6}, {143, 0x34ab}, {221, 0x3c1a}, {80, 0x6fb0}, {195, 0x752a}, {153, 0x99cf}, {231, 0x63b1}, {103, 0x1ff4}, {250, 0x1ea9}, {212, 0xf939}, {211, 0x1812}, {12, 0xc7ee}, {101, 0x8268}, {185, 0xb0ad}, {11, 0x3c5b}, {272, 0x6671}, {208, 0x9733}, {88, 0x1100}, {152, 0xe8d4}, {97, 0x8165}, {186, 0x5914}, {214, 0xd621}, {144, 0xe29f}, {192, 0x9135}, {197, 0x93ae}, {200, 0x7410}, {218, 0x6d2d}, {96, 0x0e64}, {79, 0x825e}, {251, 0xc5a5}, {81, 0x3e5b}, {89, 0xf861}, {149, 0xc0b9}, {206, 0x9c2e}, {146, 0x2295}, {225, 0x5d32}, {207, 0x3b23}, {229, 0x8528}, {204, 0x8212}, {151, 0xf1c8}, {203, 0xd91c}, {213, 0x8f10}, {191, 0x4739}, {147, 0xb8e4}, {240, 0x68fa}, {140, 0x17f1}, {196, 0x5128}, {216, 0x3b16}, {150, 0xe987}, {209, 0x491a}, {249, 0x8c2e}, {84, 0x64ec}, {91, 0x796f}, {94, 0xc967}, {194, 0xe13d}, {154, 0xd782}, {85, 0x593a}, {86, 0x6903}, {215, 0x1b1f},
#elif IN_INDRIYA
    {1, 0xaeb3}, {2, 0x7e40}, {3, 0x11ed}, {4, 0xf3db}, {5, 0x3472}, {6, 0x16b9}, {7, 0x9887}, {8, 0x6558}, {9, 0x655f}, {10, 0xf756}, {11, 0x7677}, {12, 0xa699}, {13, 0x1b99}, {14, 0x4117}, {15, 0xd86a}, {16, 0x9188}, {17, 0xe611}, {18, 0x1160}, {19, 0x2190}, {20, 0x0041}, {21, 0xb6cc}, {22, 0x10c5}, {24, 0x14cc}, {25, 0x4a3f}, {26, 0x3fac}, {27, 0xf49d}, {28, 0xb2d8}, {30, 0xc07d}, {31, 0x0d5f}, {32, 0xb0a3}, {33, 0xb5d8}, {34, 0x5156}, {35, 0x63b0}, {36, 0x260c}, {37, 0x9586}, {38, 0x1b21}, {39, 0x7e48}, {40, 0x9161}, {41, 0x98e2}, {42, 0x53db}, {43, 0x9959}, {44, 0x5da1}, {45, 0x0856}, {46, 0xf7cf}, {47, 0x8f78}, {48, 0x0fbd}, {50, 0xa9c4}, {51, 0xfa5b}, {52, 0x65c2}, {53, 0x83cd}, {54, 0xd634}, {55, 0x3a54}, {56, 0x61b4}, {57, 0xdc77}, {58, 0xd393}, {59, 0x916b}, {60, 0xcd5d}, {63, 0x362a}, {64, 0x5916}, {65, 0x5396}, {66, 0x701c}, {67, 0xea46}, {68, 0x8b87}, {69, 0xbb9c}, {70, 0xe771}, {71, 0x01b6}, {72, 0x2e77}, {73, 0xa982}, {74, 0xc75d}, {75, 0x627e}, {76, 0xac09}, {77, 0x6d78}, {78, 0xfa5c}, {79, 0xb8c3}, {80, 0xf58a}, {81, 0x0840}, {82, 0x6c98}, {83, 0xd098}, {84, 0xc87d}, {85, 0x8c75}, {115, 0x9bb0}, {116, 0xc698}, {117, 0x40d1}, {118, 0xbde5}, {119, 0xb13b}, {120, 0xc5d3}, {121, 0xb54e}, {122, 0x6c18}, {124, 0x82cd}, {126, 0xd9f6}, {127, 0x4eab}, {128, 0xdc44}, {130, 0xabd9}, {131, 0x0653}, {133, 0x4e82}, {134, 0xea27}, {135, 0x2997}, {138, 0x38e7}, {139, 0x148d},
#elif IN_MOTES
    {1, 0x111f}, {2, 0x180b}, {3, 0x44b3},
#endif
    {0, 0x0000}
};

/* The total number of nodes in the deployment */
#if IN_COOJA
#define N_NODES 8
#else
#define N_NODES ((sizeof(id_mac_list)/sizeof(struct id_mac))-1)
#endif

/* Returns the node's node-id */
uint16_t
get_node_id()
{
  return node_id_from_rimeaddr((const rimeaddr_t *)&ds2411_id);
}

/* Returns the total number of nodes in the deployment */
uint16_t
get_n_nodes()
{
  return N_NODES;
}

/* Returns a node-id from a node's rimeaddr */
uint16_t
node_id_from_rimeaddr(const rimeaddr_t *addr)
{
#if IN_COOJA
  if(addr == NULL) return 0;
  else return addr->u8[7];
#else /* IN_COOJA */
  if(addr == NULL) return 0;
  uint16_t mymac = addr->u8[7] << 8 | addr->u8[6];
  const struct id_mac *curr = id_mac_list;
  while(curr->mac != 0) {
    if(curr->mac == mymac) {
      return curr->id;
    }
    curr++;
  }
  return 0;
#endif /* IN_COOJA */
}

/* Returns a node-id from a node's IPv6 address */
uint16_t
node_id_from_ipaddr(const uip_ipaddr_t *addr)
{
  return (addr->u8[14] << 8) + addr->u8[15];
}

/* Returns a node-id from a node's absolute index in the deployment */
uint16_t
get_node_id_from_index(uint16_t index)
{
#if IN_COOJA
  return 1 + (index % N_NODES);
#else
  return id_mac_list[index % N_NODES].id;
#endif
}

/* Sets the IID of an IPv6 from a link-layer address */
void
set_iid_from_rimeaddr(uip_ipaddr_t *ipaddr, const rimeaddr_t *lladdr)
{
  set_iid_from_id(ipaddr, node_id_from_rimeaddr((const rimeaddr_t*)lladdr));
}

/* Sets the IID of an IPv6 from a node-id */
void
set_iid_from_id(uip_ipaddr_t *ipaddr, uint16_t id)
{
  ipaddr->u8[8] = ipaddr->u8[10] = ipaddr->u8[12] = ipaddr->u8[14] = id >> 8;
  ipaddr->u8[9] = ipaddr->u8[11] = ipaddr->u8[13] = ipaddr->u8[15] = id;
}

/* Sets an IPv6 from a link-layer address */
void
set_ipaddr_from_rimeaddr(uip_ipaddr_t *ipaddr, const rimeaddr_t *lladdr) {
  set_ipaddr_from_id(ipaddr, node_id_from_rimeaddr((const rimeaddr_t*)lladdr));
}

/* Sets an IPv6 from a node-id */
void
set_ipaddr_from_id(uip_ipaddr_t *ipaddr, uint16_t id)
{
  memcpy(ipaddr, &prefix, 8);
  set_iid_from_id(ipaddr, id);
}

/* Initializes global IPv6 and creates DODAG */
void
deployment_init(uip_ipaddr_t *ipaddr) {
  uint16_t id = get_node_id();
  rpl_dag_t *dag;

  uip_ip6addr(&prefix, 0xaaaa, 0, 0, 0, 0, 0, 0, 0);
  set_ipaddr_from_id(ipaddr, id);
  uip_ds6_addr_add(ipaddr, 0, ADDR_AUTOCONF);

  if(node_id == ROOT_ID) {
    rpl_set_root(RPL_DEFAULT_INSTANCE, ipaddr);
    dag = rpl_get_any_dag();
    rpl_set_prefix(dag, &prefix, 64);
  }
}

#include "contiki.h"
#include "squatix-node.h"
#include "net/packetbuf.h"
#include "net/ipv6/uip-icmp6.h"
#include "net/rpl/rpl-private.h"

// #include "net/rime/rime.h" /* Needed for so-called rime-sniffer */

#define DEBUG DEBUG_PRINT
#include "net/ip/uip-debug.h"

/* A net-layer sniffer for packets sent and received */
// static void squatix_packet_received(void);
// static void squatix_packet_sent(int mac_status);
// RIME_SNIFFER(squatix_sniffer, squatix_packet_received, squatix_packet_sent);

/* The current RPL preferred parent's link-layer address */
linkaddr_t squatix_parent_linkaddr;


/* Set to one only after getting an ACK for a DAO sent to our preferred parent */
int squatix_parent_knows_us = 0;

/* The set of squatix rules in use */
const struct squatix_rule *all_rules[] = SQUATIX_RULES;
#define NUM_RULES (sizeof(all_rules) / sizeof(struct squatix_rule *))

/*---------------------------------------------------------------------------*/
// static void
// squatix_packet_received(void)
// {
// }
/*---------------------------------------------------------------------------*/
// static void
// squatix_packet_sent(int mac_status)
// {
//   /* Check if our parent just ACKed a DAO */
//   if(squatix_parent_knows_us == 0
//      && mac_status == MAC_TX_OK
//      && packetbuf_attr(PACKETBUF_ATTR_NETWORK_ID) == UIP_PROTO_ICMP6
//      && packetbuf_attr(PACKETBUF_ATTR_CHANNEL) == (ICMP6_RPL << 8 | RPL_CODE_DAO)) {
//     if(!linkaddr_cmp(&squatix_parent_linkaddr, &linkaddr_null)
//        && linkaddr_cmp(&squatix_parent_linkaddr, packetbuf_addr(PACKETBUF_ADDR_RECEIVER))) {
//       squatix_parent_knows_us = 1;
//     }
//   }
// }
/*---------------------------------------------------------------------------*/
void
squatix_callback_child_added(const linkaddr_t *addr)
{
  /* Notify all squatix rules that a child was added */
  int i;
  for(i = 0; i < NUM_RULES; i++) {
    if(all_rules[i]->child_added != NULL) {
      all_rules[i]->child_added(addr);
    }
  }
}
/*---------------------------------------------------------------------------*/
void
squatix_callback_child_removed(const linkaddr_t *addr)
{
  /* Notify all squatix rules that a child was removed */
  int i;
  for(i = 0; i < NUM_RULES; i++) {
    if(all_rules[i]->child_removed != NULL) {
      all_rules[i]->child_removed(addr);
    }
  }
}
/*---------------------------------------------------------------------------*/
void
squatix_callback_packet_ready(void)
{
  int i;
  /* By default, use any slotframe, any timeslot */
  uint16_t slotframe = 9;
  uint16_t timeslot = 0xffff;

  /* Loop over all rules until finding one able to handle the packet */
  for(i = 0; i < NUM_RULES; i++) {
    if(all_rules[i]->select_packet != NULL) {
      if(all_rules[i]->select_packet(&slotframe, &timeslot)) {
        break;
      }
    }
  }

#if TSCH_WITH_LINK_SELECTOR
  packetbuf_set_attr(PACKETBUF_ATTR_TSCH_SLOTFRAME, slotframe);
  packetbuf_set_attr(PACKETBUF_ATTR_TSCH_TIMESLOT, timeslot);
#endif
}
/*---------------------------------------------------------------------------*/
void
squatix_callback_new_time_source(const struct tsch_neighbor *old, const struct tsch_neighbor *new)
{
  /* squatix assumes that the time source is also the RPL parent.
   * This is the case if the following is set:
   * #define RPL_CALLBACK_PARENT_SWITCH tsch_rpl_callback_parent_switch
   * */

  int i;
  if(new != old) {
    squatix_parent_knows_us = 0;
  }
  for(i = 0; i < NUM_RULES; i++) {
    if(all_rules[i]->new_time_source != NULL) {
      all_rules[i]->new_time_source(old, new);
    }
  }
}
/*---------------------------------------------------------------------------*/
void
squatix_init(void)
{
  int i;
  /* Snoop on packet transmission to know if our parent knows about us
   * (i.e. has ACKed at one of our DAOs since we decided to use it as a parent) */
//   rime_sniffer_add(&squatix_sniffer);
  
  linkaddr_copy(&squatix_parent_linkaddr, &linkaddr_null);
  /* Initialize all squatix rules */
  for(i = 0; i < NUM_RULES; i++) {
    if(all_rules[i]->init != NULL) {
      PRINTF("squatix: initializing rule %u\n", i);
      all_rules[i]->init(i);
    }
  }
  PRINTF("squatix: initialization done\n");
}

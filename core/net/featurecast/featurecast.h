#ifndef FEATURECAST_H
#define FEATURECAST_H

#define LABEL_ROOM_1 1
#define LABEL_ROOM_2 2
#define LABEL_ROOM_3 4
#define LABEL_ROOM_4 8

#define LABEL_TYPE_TEMP 16
#define LABEL_TYPE_LIGHT 32

#define LABEL_FLOOR_1 64
#define LABEL_FLOOR_2 128

#define LABEL_ADV 0
#define LABEL_DISC 1


#define MAX_NEIGHBORS 3
#include "net/uip.h"
#include "net/uip-ds6.h"
#include "net/uip-udp-packet.h"
#include <stdio.h>
#include <string.h>
#include "net/rpl/rpl.h"

struct {
	uip_ip6addr_t addr;
	uint16_t labels;
} typedef routing_entry_t;

struct label_packet{
	int type;
	uip_ip6addr_t src;
	uint16_t labels;
	int data;
} typedef label_packet_t;

struct featurecast_conn{
	uip_ip6addr_t src;
	uint16_t lport;        /**< The local port number in network byte order. */
 	uint16_t rport;        /**< The remote port number in network byte order. */
  	uint8_t  ttl;
};

struct {
	routing_entry_t entries[MAX_NEIGHBORS];
} typedef routing_table_t;

extern  struct ctimer backoff_timer;
extern struct featurecast_conn featurecast_conn;
extern uip_ipaddr_t ipaddr;
void init_featurecast();

void init_addr_for_labels(uip_ip6addr_t* addr);
int is_featurecast_addr(uip_ip6addr_t* addr);
extern routing_table_t routing_table;

routing_entry_t* get_from_table(routing_table_t* table, uip_ip6addr_t* addr);

int put_in_table(routing_table_t* table, routing_entry_t* );

int delete_from_table(routing_table_t* table, uip_ip6addr_t* addr);

void print_table(routing_table_t* table);

void init_routing_table(routing_table_t* table);

void put_label_in_addr(int label, uip_ip6addr_t* addr);

int addr_contains_label(int label, uip_ip6addr_t* addr);

//void check_against_all_labels(uip_ip6addr_t* addr);

int bits_in_addr(uip_ip6addr_t* addr);

int bits_in_intersec(uip_ip6addr_t* addr1, uip_ip6addr_t* addr2);

 void featurecast_handler(void);

int send_featurecast(void* data, uint16_t datalen, int label, ...);

void get_merged_entry(routing_table_t* table, routing_entry_t* entry);

int forward_label_packet();

void test_children();

void featurecast_icmp_input();
#endif /* FEATURECAST_H */



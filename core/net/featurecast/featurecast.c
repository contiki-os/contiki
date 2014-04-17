#include "featurecast.h"
#define DEBUG DEBUG_NONE
#include "net/ip/uip-debug.h"
#include "net/ipv6/uip-ds6.h"
#include "net/ip/uip.h"
#include "net/ipv6/uip-nd6.h"
#include "net/ipv6/uip-ds6.h"
#include "net/ipv6/uip-icmp6.h"

#define UIP_IP_BUF   ((struct uip_ip_hdr *)&uip_buf[UIP_LLH_LEN])
#define UIP_ICMP_PAYLOAD ((label_packet_t *)&uip_buf[uip_l2_l3_icmp_hdr_len]) 


routing_table_t routing_table;
//struct uip_udp_conn* udp_conn;
uip_ipaddr_t ipaddr;
extern uint16_t uip_slen; 

struct featurecast_conn featurecast_conn;

struct ctimer backoff_timer;
int routing_pos = 0;

uip_buf_t backup;

void init_featurecast(){

//	udp_conn = udp_new(NULL, UIP_HTONS(8888), NULL);
}
routing_entry_t* get_from_table(routing_table_t* table, uip_ip6addr_t* addr){
	int i;
	for(i = 0; i < MAX_NEIGHBORS; i ++){
		if(uip_ip6addr_cmp(&table->entries[i].addr, addr)){
			return &table->entries[i];
		}	
	}
	return NULL;
}

int put_in_table(routing_table_t* table, routing_entry_t* entry){
	int i;
        for(i = 0; i < MAX_NEIGHBORS; i ++){
        	if(table->entries[i].addr.u16[0] == 0){
			memcpy(&table->entries[i], entry, sizeof(*entry));
			return 0;
		}
	}
        return -1;

}

int merge_into_table(routing_table_t* table, routing_entry_t* entry){
	
	routing_entry_t* old_entry = get_from_table(table, &entry->addr);
	if(old_entry == NULL){
		return put_in_table(table, entry);
	}else{
		old_entry->labels = entry->labels;
		return 0;
	}
	return -1;
}

int delete_from_table(routing_table_t* table, uip_ip6addr_t* addr){
	routing_entry_t* entry = get_from_table(table, addr);
	if(entry != NULL){
		memset(entry, 0, sizeof(*entry));
		return 0;
	}else
		return -1;
}

void print_table(routing_table_t* table){
	int i;
	printf("Printing routing table: \n");
        for(i = 0; i < MAX_NEIGHBORS; i ++){
		uip_debug_ipaddr_print(&table->entries[i].addr);
		printf(" -> %ui(", table->entries[i].labels);
		((LABEL_ROOM_1  &  table->entries[i].labels) == LABEL_ROOM_1) ? printf("LABEL_ROOM_1, ") : printf("!LABEL_ROOM_1, ");
		((LABEL_ROOM_2  &  table->entries[i].labels) == LABEL_ROOM_2) ? printf("LABEL_ROOM_2, ") : printf("!LABEL_ROOM_2, ");
		((LABEL_ROOM_3  &  table->entries[i].labels) == LABEL_ROOM_3) ? printf("LABEL_ROOM_3, ") : printf("!LABEL_ROOM_3, ");
		((LABEL_ROOM_4  &  table->entries[i].labels) == LABEL_ROOM_4) ? printf("LABEL_ROOM_4, ") : printf("!LABEL_ROOM_4, ");
		((LABEL_TYPE_TEMP  &  table->entries[i].labels) == LABEL_TYPE_TEMP) ? printf("LABEL_TYPE_TEMP, ") : printf("!LABEL_TYPE_TEMP, ");
		((LABEL_TYPE_LIGHT  &  table->entries[i].labels) == LABEL_TYPE_LIGHT) ? printf("LABEL_TYPE_LIGHT, ") : printf("!LABEL_TYPE_LIGHT, ");
		printf(")\n");
	}
}

void init_routing_table(routing_table_t* table){
	int i;
	for(i = 0; i < MAX_NEIGHBORS; i ++){
        	memset(&table->entries[i].addr, 0, sizeof(table->entries[i].addr));
		memset(&table->entries[i].labels, 0, sizeof(table->entries[i].labels));
	}

}

void init_addr_for_labels(uip_ip6addr_t* addr){

	addr->u8[0] = 0xFF;
	addr->u8[1] = 0xEF;
	addr->u16[6] = (unsigned int) 0;
}

int is_featurecast_addr(uip_ip6addr_t* addr){
	if((addr->u8[0] == 0xFF) && (addr->u8[1] == 0xEF)){
		return 1;
	}
	return 0;
}

void put_label_in_addr(int label, uip_ip6addr_t* addr){

	addr->u16[6] += (unsigned int) label;
}

int addr_contains_label(int label, uip_ip6addr_t* addr){

	
	PRINTF("label %d, addr %d, label&addr %d\n", label, addr->u16[6], label & addr->u16[6]);
	return ( (label &  addr->u16[6]) == label);
}


/*void check_against_all_labels(uip_ip6addr_t* addr){	
PRINTF("Checking: ");
PRINT6ADDR(addr);
PRINTF(":\n");
addr_contains_label(LABEL_ROOM_1, addr) ? PRINTF("contains label %d\n", LABEL_ROOM_1) : PRINTF("does not contain label %d \n", LABEL_ROOM_1);
addr_contains_label(LABEL_ROOM_2, addr) ? PRINTF("contains label %d\n", LABEL_ROOM_2) : PRINTF("does not contain label %d \n", LABEL_ROOM_2);
addr_contains_label(LABEL_ROOM_3, addr) ? PRINTF("contains label %d\n", LABEL_ROOM_3) : PRINTF("does not contain label %d \n", LABEL_ROOM_3);
addr_contains_label(LABEL_ROOM_4, addr) ? PRINTF("contains label %d\n", LABEL_ROOM_4) : PRINTF("does not contain label %d \n", LABEL_ROOM_4);

addr_contains_label(LABEL_TYPE_TEMP, addr) ? PRINTF("contains label %d\n", LABEL_TYPE_TEMP) : PRINTF("does not contain label %d \n", LABEL_TYPE_TEMP);
addr_contains_label(LABEL_TYPE_LIGHT, addr) ? PRINTF("contains label %d\n", LABEL_TYPE_LIGHT) : PRINTF("does not contain label %d \n", LABEL_TYPE_LIGHT);

}*/

int bits_in_addr(uip_ip6addr_t* addr){

uint16_t label = addr->u16[6];
unsigned int counter = 0;
for (counter = 0; label; label >>= 1)
{
  counter += label & 1;
}

return counter;

}

int bits_in_intersec(uip_ip6addr_t* addr1, uip_ip6addr_t* addr2){


PRINT6ADDR(addr1);
PRINTF(" vs ");
PRINT6ADDR(addr2);
PRINTF("\n");
check_against_all_labels(addr1);
check_against_all_labels(addr2);
uint16_t label = addr1->u16[6] & addr2->u16[6];
unsigned int counter = 0;
PRINTF("LABEL: %d\n", label);
for (counter = 0; label; label >>= 1)
{
  counter += label & 1;
}

PRINTF("Result %d\n", counter);

return counter;

}


void test_children(){
  //uip_ipaddr_t *nexthops[UIP_DS6_ROUTE_NB];

  
  PRINTF("Preferred parent: ");
  PRINT6ADDR(uip_ds6_defrt_choose());
  PRINTF("\n");
  PRINTF("Routes [%u max]\n",UIP_DS6_ROUTE_NB);
  

  uip_ds6_route_t *r;

  for (r = uip_ds6_route_head(); r != NULL; r = list_item_next(r)) {
    uip_debug_ipaddr_print(&r->ipaddr);
//    PRINTF(" -> next hop: ");
 //   uip_debug_ipaddr_print(&r->next->ipaddr);
    PRINTF("\n");
    
  }
}

void get_merged_entry(routing_table_t* table, routing_entry_t* entry){
	int i;
	//printf("merging: ");
	memcpy(&entry->addr, &ipaddr, sizeof(entry->addr)); 
	entry->labels = (unsigned int) ipaddr.u16[6];
        for(i = 0; i < MAX_NEIGHBORS; i ++){
                if(table->entries[i].addr.u16[0] != 0){
			//printf("merged_entry %u + %u = ",entry->labels, table->entries[i].labels); 
                        entry->labels |= table->entries[i].labels;
			//printf("%u\n", entry->labels);
                }
        }

}

int packet_for_me(label_packet_t* packet){
	if((packet->labels & ipaddr.u16[6]) == packet->labels){
		return 1;
	}
	return 0; 
}

int send_featurecast(void* data, uint16_t datalen, int labels_num, ...){

	init_addr_for_labels(&featurecast_conn.src);
	va_list ap;
        va_start(ap, labels_num);
	int val, i;
        for(i = 0; i < labels_num; i++) {
                val = va_arg(ap, int);
		printf("got label %u\n", val);
 		put_label_in_addr(val, &featurecast_conn.src);               
        }
        va_end(ap);

	featurecast_conn.lport = uip_htons(8888);
	featurecast_conn.rport = uip_htons(8888);


	if(data != NULL) {
    		uip_slen = datalen;
    		memcpy(&uip_buf[UIP_LLH_LEN + UIP_IPUDPH_LEN], data,
           		datalen > UIP_BUFSIZE - UIP_LLH_LEN - UIP_IPUDPH_LEN?
           		UIP_BUFSIZE - UIP_LLH_LEN - UIP_IPUDPH_LEN: datalen);
    		uip_process(FEATURECAST_SEND);
                forward_label_packet();
 	}
 	uip_slen = 0;
	return 0;
}

void featurecast_handler(void){
	label_packet_t* packet;
	printf("Handler!\n");
  	if(uip_newdata()) { 
    		printf("got udp (from:");
		uip_debug_ipaddr_print(&UIP_IP_BUF->srcipaddr);
		printf(") - ");
		memset(UIP_IP_BUF, '0', UIP_BUFSIZE);
    		packet = (label_packet_t*) uip_appdata;
			printf("\n");
  		//}else if(packet->type == LABEL_PACKET_DATA){
			packet->data++;
			if(packet_for_me(packet)) printf("data packet for me %d\n", packet->data);
			//forward_label_packet(packet);
		//}	
	}
}

int forward_label_packet(){
	printf("forwarding label packet  \n");
	int forwarded;
	forwarded = 0;
	int sent = 0;
	uip_ds6_nbr_t *nbr = NULL;
	uint16_t labels = UIP_IP_BUF->destipaddr.u16[6];

	if(routing_pos > 0){
		memcpy(UIP_IP_BUF, &backup, sizeof(backup) - UIP_LLH_LEN);
		uip_len = (UIP_IP_BUF->len[0] << 8) + UIP_IP_BUF->len[1] + UIP_IPH_LEN;
	}
        for(; routing_pos < MAX_NEIGHBORS; routing_pos++){
        	routing_entry_t* cur_entry;
                cur_entry = &routing_table.entries[routing_pos];
		printf("checking %d against (labels:%d): ", cur_entry->labels, labels);
		if((cur_entry->labels & labels) == labels){
			if(sent > 0 ){
				printf("sent > 0 rescheduling pos %d\n", routing_pos);
				sent = 0;
				memcpy(&backup, UIP_IP_BUF, sizeof(backup) - UIP_LLH_LEN);
				ctimer_set(&backoff_timer, CLOCK_SECOND, (void(*)(void *))forward_label_packet, NULL);
				return 0;
			}
			uip_debug_ipaddr_print(&cur_entry->addr);
			printf(" - OK -> sending\n");
			forwarded = 1;
                        //clock_wait(200);
			nbr = uip_ds6_nbr_lookup(&cur_entry->addr);
			printf("sent result: %d\n", tcpip_output(uip_ds6_nbr_get_ll(nbr)));
			//uip_len = 0;
			//uip_ext_len = 0;
			sent++;
		}else{
			printf(" - failed\n");
		}
      	}
	routing_pos = 0;
//	if(!forwarded && packet_for_me(packet))
//		printf("leaf cost %d\n", packet->data);
	return 0;
}

void featurecast_icmp_input(){


	label_packet_t* packet = UIP_ICMP_PAYLOAD;
	routing_entry_t* entry;
	
	printf("Got Featurecast ICMP6!\n");
    	if(packet->type == LABEL_ADV){
		printf("adv (labels: %u)", packet->labels);
		entry = (routing_entry_t*) &packet->src;
		packet->src.u16[0] = 0x80fe;
    		merge_into_table(&routing_table, entry);
  		print_table(&routing_table);
    		rpl_dag_t* dag;
    		dag = rpl_get_any_dag();
		memcpy(&packet->src, &ipaddr, sizeof(packet->src));
    		get_merged_entry(&routing_table, (routing_entry_t*) &packet->src);
    		if((dag != NULL) && (dag->preferred_parent != NULL )){
			printf(" - forwarding to ");
			uip_debug_ipaddr_print(rpl_get_parent_ipaddr(dag->preferred_parent));
			uip_icmp6_send(rpl_get_parent_ipaddr(dag->preferred_parent), ICMP6_FEATURECAST, LABEL_ADV, sizeof(*packet));
		}
	}else if(packet->type == LABEL_DISC){
		printf("Got disconnect msg\n");
		packet->src.u16[0] = 0x80fe;
		delete_from_table(&routing_table, &packet->src);
	}else{
		printf("got unknown ICMP6 packet - discard \n");	
	}
}

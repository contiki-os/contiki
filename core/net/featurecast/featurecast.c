#if WITH_FEATURECAST

#include "featurecast.h"
#define DEBUG DEBUG_NONE
#include "net/uip-debug.h"
#include "net/uip-ds6.h"
#include "net/uip.h"
#include "net/uip-nd6.h"
#include "net/uip-ds6.h"
#include "net/uip-icmp6.h"
#define UIP_IP_BUF   ((struct uip_ip_hdr *)&uip_buf[UIP_LLH_LEN])
#define UIP_ICMP_PAYLOAD ((label_packet_t *)&uip_buf[uip_l2_l3_icmp_hdr_len]) 


routing_table_t routing_table;
//uip_ipaddr_t ipaddr;
extern uint16_t uip_slen; 

struct featurecast_conn featurecast_conn;

struct ctimer backoff_timer;
int routing_pos = 0;

static unsigned int adv_received = 0;
static unsigned int disc_received = 0;

unsigned int adv_sent = 0;
unsigned int disc_sent = 0;

uip_buf_t backup;
uip_ipaddr_t featurecast_addr;

int fpow(int n){
	int i;
	int result = 1;
	for(i = 0; i < n; i++){
		result *= 2;
	}
	return result;
}
void init_featurecast(){

//	struct uip_udp_conn *udp_conn = udp_new(NULL, uip_htons(8888), NULL);
//	udp_bind(udp_conn, uip_htons(8888));
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
		int byte;
		for(byte = 0; byte < 6; byte++)
		old_entry->labels[byte] = entry->labels[byte];
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
	PRINTF("Printing routing table: \n");
        for(i = 0; i < MAX_NEIGHBORS; i ++){
		PRINT6ADDR(&table->entries[i].addr);
		/*PRINTF(" -> %ui(", table->entries[i].labels);
		((LABEL_ROOM_1  &  table->entries[i].labels) == LABEL_ROOM_1) ? PRINTF("LABEL_ROOM_1, ") : PRINTF("!LABEL_ROOM_1, ");
		((LABEL_ROOM_2  &  table->entries[i].labels) == LABEL_ROOM_2) ? PRINTF("LABEL_ROOM_2, ") : PRINTF("!LABEL_ROOM_2, ");
		((LABEL_ROOM_3  &  table->entries[i].labels) == LABEL_ROOM_3) ? PRINTF("LABEL_ROOM_3, ") : PRINTF("!LABEL_ROOM_3, ");
		((LABEL_ROOM_4  &  table->entries[i].labels) == LABEL_ROOM_4) ? PRINTF("LABEL_ROOM_4, ") : PRINTF("!LABEL_ROOM_4, ");
		((LABEL_TYPE_TEMP  &  table->entries[i].labels) == LABEL_TYPE_TEMP) ? PRINTF("LABEL_TYPE_TEMP, ") : PRINTF("!LABEL_TYPE_TEMP, ");
		((LABEL_TYPE_LIGHT  &  table->entries[i].labels) == LABEL_TYPE_LIGHT) ? PRINTF("LABEL_TYPE_LIGHT, ") : PRINTF("!LABEL_TYPE_LIGHT, ");*/
		PRINTF(")\n");
	}
}

void print_stats(){
	int i;
	unsigned int table_size = 0;
        for(i = 0; i < MAX_NEIGHBORS; i ++){
        	if(routing_table.entries[i].addr.u16[0] != 0){
			table_size += sizeof(routing_table.entries[i]);
		}
	}

	printf("Routing table size: %u\n Advertisement received/sent: %u/%u\n Disconnect received/sent: %u/%u\n", table_size, adv_received, adv_sent, disc_received, disc_sent);
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
	addr->u16[1] = (unsigned int) 0;
	addr->u16[2] = (unsigned int) 0;
	addr->u16[3] = (unsigned int) 0;
	addr->u16[4] = (unsigned int) 0;
	addr->u16[5] = (unsigned int) 0;
	addr->u16[6] = (unsigned int) 0;
}

int is_featurecast_addr(uip_ip6addr_t* addr){
	if((addr->u8[0] == 0xFF) && (addr->u8[1] == 0xEF)){
		return 1;
	}
	return 0;
}

void put_label_in_addr(int label, uip_ip6addr_t* addr){
	int byte = (label / 16) + 1;
	int pos = label % 16;
	addr->u16[byte] += (unsigned int) fpow(pos);
}

void put_label_in_packet(int label, label_packet_t* packet){
	int byte = (label / 16) ;
	int pos = label % 16;
	packet->labels[byte] += (unsigned int) fpow(pos);
}

int addr_contains_label(int label, uip_ip6addr_t* addr){
	int byte = (label / 16) + 1;
	int pos = label % 16;	

	PRINTF("label %d, label pow %d addr %d, label&addr %d\n", label, (int) fpow(pos), addr->u16[byte], fpow(pos) & addr->u16[byte]);
	return (((unsigned int)fpow(pos) &  addr->u16[byte]) == (unsigned int) fpow(pos));
}


void check_against_all_labels(uip_ip6addr_t* addr){	
/*PRINTF("Checking: ");
PRINT6ADDR(addr);
PRINTF(":\n");
addr_contains_label(LABEL_ROOM_1, addr) ? PRINTF("contains label %d\n", LABEL_ROOM_1) : PRINTF("does not contain label %d \n", LABEL_ROOM_1);
addr_contains_label(LABEL_ROOM_2, addr) ? PRINTF("contains label %d\n", LABEL_ROOM_2) : PRINTF("does not contain label %d \n", LABEL_ROOM_2);
addr_contains_label(LABEL_ROOM_3, addr) ? PRINTF("contains label %d\n", LABEL_ROOM_3) : PRINTF("does not contain label %d \n", LABEL_ROOM_3);
addr_contains_label(LABEL_ROOM_4, addr) ? PRINTF("contains label %d\n", LABEL_ROOM_4) : PRINTF("does not contain label %d \n", LABEL_ROOM_4);

addr_contains_label(LABEL_TYPE_TEMP, addr) ? PRINTF("contains label %d\n", LABEL_TYPE_TEMP) : PRINTF("does not contain label %d \n", LABEL_TYPE_TEMP);
addr_contains_label(LABEL_TYPE_LIGHT, addr) ? PRINTF("contains label %d\n", LABEL_TYPE_LIGHT) : PRINTF("does not contain label %d \n", LABEL_TYPE_LIGHT);
*/
}

int bits_in_addr(uip_ip6addr_t* addr){

int byte;
unsigned int counter = 0;
for(byte = 1; byte < 7; byte++){
	uint16_t label = addr->u16[byte];
	for (; label; label >>= 1){
	  counter += label & 1;
	}
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
int i;
unsigned int counter = 0;
for(i = 1; i < 7; i++){
	uint16_t label = addr1->u16[i] & addr2->u16[i];
	PRINTF("LABEL: %d\n", label);
	for (counter = 0; label; label >>= 1){
	  counter += label & 1;
	}
}

PRINTF("Result %d\n", counter);

return counter;

}


void test_children(){

  
  PRINTF("Preferred parent: ");
  PRINT6ADDR(uip_ds6_defrt_choose());
  PRINTF("\n");
  PRINTF("Routes [%u max]\n",UIP_DS6_ROUTE_NB);
  

  uip_ds6_route_t *r;

  for (r = uip_ds6_route_head(); r != NULL; r = list_item_next(r)) {
    PRINT6ADDR(&r->ipaddr);
    PRINTF(" -> next hop: ");
    PRINT6ADDR(&r->next->ipaddr);
    PRINTF("\n");
    ;
  }
}

void get_merged_entry(routing_table_t* table, routing_entry_t* entry){
	int i;
	PRINTF("merging: ");
	memcpy(&entry->addr, &featurecast_addr, sizeof(entry->addr));
	int byte;
	for(byte = 0; byte < 6; byte ++){
		entry->labels[byte] = (unsigned int) featurecast_addr.u16[byte + 1];
	        for(i = 0; i < MAX_NEIGHBORS; i ++){
	                if(table->entries[i].addr.u16[0] != 0){
				PRINTF("merged_entry %u + %u = ",entry->labels, table->entries[i].labels); 
	                        entry->labels[byte] |= table->entries[i].labels[byte];
				PRINTF("%u\n", entry->labels);
	                }
	        }
	}

}

int packet_for_me(label_packet_t* packet){
	int byte;
	for(byte = 0; byte < 7; byte++){
		if((packet->labels[byte] & featurecast_addr.u16[byte+1]) == packet->labels[byte]){
			return 1;
		}
	}
	return 0; 
}

int send_featurecast(void* data, uint16_t datalen, int labels_num, ...){

	init_addr_for_labels(&featurecast_conn.dst);
	//PRINTF("init:");
	PRINT6ADDR(&featurecast_conn.dst);
	PRINTF("\n");
	va_list ap;
        va_start(ap, labels_num);
	int val, i;
        for(i = 0; i < labels_num; i++) {
                val = va_arg(ap, int);
		PRINTF("got label %u\n", val);
		put_label_in_addr(val, &featurecast_conn.dst);               
	}
	va_end(ap);
	PRINTF("conn:");
	PRINT6ADDR(&featurecast_conn.dst);
	PRINTF("\n");
	featurecast_conn.lport = uip_htons(8888);
	featurecast_conn.rport = uip_htons(8888);


	if(data != NULL) {
    		uip_slen = datalen;
    		memcpy(&uip_buf[UIP_LLH_LEN + UIP_IPUDPH_LEN], data,
           		datalen > UIP_BUFSIZE - UIP_LLH_LEN - UIP_IPUDPH_LEN?
           		UIP_BUFSIZE - UIP_LLH_LEN - UIP_IPUDPH_LEN: datalen);
    		uip_process(FEATURECAST_SEND);
                forward_label_packet();
		PRINTF("after forwarding\n");
 	}
 	uip_slen = 0;
	return 0;
}

/*void featurecast_handler(void){
	label_packet_t* packet;
	PRINTF("Handler!\n");
  	if(uip_newdata()) { 
    		PRINTF("got udp (from:");
		PRINT6ADDR(&UIP_IP_BUF->srcipaddr);
		PRINTF(") - ");
		memset(UIP_IP_BUF, '0', UIP_BUFSIZE);
    		packet = (label_packet_t*) uip_appdata;
			PRINTF("\n");
  		}else if(packet->type == LABEL_PACKET_DATA){
			packet->data++;
			if(packet_for_me(packet)) PRINTF("data packet for me %d\n", packet->data);
			//forward_label_packet(packet);
		}	
	}
}*/

int forward_label_packet(){
	PRINTF("forwarding label packet  \n");
	print_table(&routing_table);
	int sent = 0;
	uint16_t labels[6];
	int byte;

	if(routing_pos > 0){
		memcpy(UIP_IP_BUF, &backup, sizeof(backup) - UIP_LLH_LEN);
		uip_len = (UIP_IP_BUF->len[0] << 8) + UIP_IP_BUF->len[1] + UIP_IPH_LEN;
	}else{
	}
	for(byte = 0; byte < 6; byte ++)
	 labels [byte] = UIP_IP_BUF->destipaddr.u16[byte + 1];

        for(; routing_pos < MAX_NEIGHBORS; routing_pos++){
        	routing_entry_t* cur_entry;
                cur_entry = &routing_table.entries[routing_pos];
		PRINTF("checking % against (labels:%d): ", cur_entry->labels, labels);
		int flag = 1;
		for(byte = 0; byte < 6; byte++){
			if((cur_entry->labels[byte] & labels[byte]) != labels[byte])
				flag = 0;
		}
		if(flag){
			if(sent > 0 ){
				PRINTF("sent > 0 rescheduling pos %d\n", routing_pos);
				sent = 0;
				memcpy(&backup, UIP_IP_BUF, sizeof(backup) - UIP_LLH_LEN);
				ctimer_set(&backoff_timer, CLOCK_SECOND, (void(*)(void *))forward_label_packet, NULL);
				//need to empty the buffer, or packet is send automatically without consulting routing tables
				//probably it's a bug in uIP stack				
				memset(UIP_IP_BUF, '0', sizeof(backup) - UIP_LLH_LEN);
				return 0;
			}
			PRINT6ADDR(&cur_entry->addr);
			PRINTF(" - OK -> sending\n");
                        int result = tcpip_output(uip_ds6_nbr_get_ll(uip_ds6_nbr_lookup(&cur_entry->addr)));
			PRINTF("sent result: %d\n", result);
			uip_len = 0;
			uip_ext_len = 0;
			sent++;
		}else{
			PRINTF(" - failed\n");
		}
      	}
	routing_pos = 0;
//	if(!forwarded && packet_for_me(packet))
//		PRINTF("leaf cost %d\n", packet->data);
	memset(UIP_IP_BUF, '0', sizeof(backup) - UIP_LLH_LEN);
//	PRINTF("zeroing buffer \n");
	return 0;
}

void featurecast_icmp_input(){


	label_packet_t* packet = UIP_ICMP_PAYLOAD;
	routing_entry_t* entry;
	
	PRINTF("Got Featurecast ICMP6!\n");
    	if(packet->type == LABEL_ADV){
		adv_received++;
		PRINTF("adv (labels: %d)", packet->labels);
		entry = (routing_entry_t*) &packet->src;
//		packet->src.u16[0] = 0x80fe;
		memcpy(&packet->src, &UIP_IP_BUF->srcipaddr, sizeof(packet->src));
    		merge_into_table(&routing_table, entry);
  		print_table(&routing_table);
    		rpl_dag_t* dag;
    		dag = rpl_get_any_dag();
		memcpy(&packet->src, &featurecast_addr, sizeof(packet->src));
    		get_merged_entry(&routing_table, (routing_entry_t*) &packet->src);
    		if((dag != NULL) && (dag->preferred_parent != NULL )){
			PRINTF(" - forwarding to ");
			PRINT6ADDR(rpl_get_parent_ipaddr(dag->preferred_parent));
			uip_icmp6_send(rpl_get_parent_ipaddr(dag->preferred_parent), ICMP6_FEATURECAST, LABEL_ADV, sizeof(*packet));
		}
	}else if(packet->type == LABEL_DISC){
		disc_received++;
		PRINTF("Got disconnect msg\n");
		packet->src.u16[0] = 0x80fe;
		delete_from_table(&routing_table, &packet->src);
	}else{
		PRINTF("got unknown ICMP6 packet - discard \n");	
	}
}

#endif /* WITH_FEATURECAST */

// ANDRE RIKER - ARIKER AT DEI.UC.PT
// UNIVERSITY OF COIMBRA

#include <stdio.h>
#include <string.h>
#include "net/rpl/rpl-mrhof.c"
#include <node-id.h>
#include "agg_functions.c"
#define MAX_N_PAYLOADS 40
#define LEN_SINGLE_PAYLOAD 2

unsigned num_output_payloads;

// DATA STRUCTURE
struct singlePayload{
	char strContent[LEN_SINGLE_PAYLOAD];
	int intContent;
	};

struct Payloads{
	int count_payloads;
	struct singlePayload singleP[MAX_N_PAYLOADS];
	};

struct Payloads rcv_payloads;

struct Payloads aggregated_payloads;

// FUNCTIONS
void reset_payloads(){
int i, k;
	for(i=0;i<rcv_payloads.count_payloads; i++){
		rcv_payloads.singleP[i].intContent=0;
		aggregated_payloads.singleP[i].intContent=0;

		for(k=0;k<LEN_SINGLE_PAYLOAD;k++){
			strcpy(rcv_payloads.singleP[i].strContent[k],"\0");
			strcpy(aggregated_payloads.singleP[i].strContent[k],"\0");
		}
	}
	rcv_payloads.count_payloads=0;
	aggregated_payloads.count_payloads=0;
	//printf("Parsing: Reseting aggregated payloads \n");
}

void add_payload(char* content){
	char temp1[2];
	
	strcpy(rcv_payloads.singleP[rcv_payloads.count_payloads].strContent,content);
		
	temp1[0] = (int) rcv_payloads.singleP[rcv_payloads.count_payloads].strContent[0];
	temp1[1] = (int) rcv_payloads.singleP[rcv_payloads.count_payloads].strContent[1];
	sprintf(rcv_payloads.singleP[rcv_payloads.count_payloads].intContent, "%d", temp1);// Convert from string to int
	
	rcv_payloads.singleP[rcv_payloads.count_payloads].intContent=atoi(temp1);
	//printf("New payload added: int %d string %s num pay %d \n",rcv_payloads.singleP[rcv_payloads.count_payloads].intContent, rcv_payloads.singleP[rcv_payloads.count_payloads].strContent, rcv_payloads.count_payloads);
	rcv_payloads.count_payloads=rcv_payloads.count_payloads+1;
}


int get_rcv_payloads(int payload_position){
	return rcv_payloads.singleP[payload_position].intContent;
}

int get_aggregated_payloads(int payload_position){
	return aggregated_payloads.singleP[payload_position].intContent;
}

int get_num_rcv_payloads(){
	//printf("Parsing: Number of agg message %d \n",rcv_payloads.count_payloads);
	return rcv_payloads.count_payloads;
}

int get_num_aggregated_payloads(){
	//printf("Parsing: Number of agg message %d \n",rcv_payloads.count_payloads);
	return aggregated_payloads.count_payloads;
}

int get_num_output_payloads(){
	return num_output_payloads;
}

  #if PLATFORM_HAS_AGGREGATION 
void aggregate_payloads(){
	unsigned max_output;
	double ratio, avg, min_energy_on_path;
	int i,j=0, sum=0, index_agg_function=0;
	float alpha;
	
	// route_energy goes from 0 to 1.
	min_energy_on_path = 1 - get_energy_on_route(); // after this inversion 0 means empty and 1 means full
	printf("1 - Energy on route is (0-100) %u \n", (unsigned) (min_energy_on_path * 100));
	
	index_agg_function = (unsigned) (min_energy_on_path * 100);
	

	alpha = poly[index_agg_function]; // polynomial
	//alpha = exponential[index_agg_function]; // exponential
	//alpha = min_energy_on_path; //linear

	
	// Function the determines how many payloads will be send. Allways more than 1
	num_output_payloads = 1 + ((unsigned) (alpha * get_num_rcv_payloads())); 
	
	//if(node_id == 3){num_output_payloads=1; // 3 is the node that injects traffic
	//}else{num_output_payloads=get_num_rcv_payloads();}
		
	printf("Alpha coeff is %u \n", (unsigned) (alpha * 100) );	
	printf("Num of rcv payloads is %d \n", get_num_rcv_payloads());
	
	max_output = get_num_rcv_payloads(); // cannot put in output more than rcv
	
	// Fixing an upper limit
	num_output_payloads = num_output_payloads >= max_output ? max_output : num_output_payloads;
	//num_output_payloads = 1;
	printf("Num of output payloads is %u \n", (unsigned) num_output_payloads);
	
	ratio = get_num_rcv_payloads() / num_output_payloads;
	printf("Ratio is %d \n", (unsigned) ratio);
	
	// Making the math operations to reduce the amount of rcv_payload

	aggregated_payloads.count_payloads = 0;
	for(i=0;i<get_num_rcv_payloads();i++){
		if(j<ratio){
			sum = sum + get_rcv_payloads(i);
			j++;
			if(j>=ratio){
				avg = sum / j;
				aggregated_payloads.singleP[aggregated_payloads.count_payloads].intContent = (int) avg;
				//printf("5 - Avg is %d Aggregated payload num %d has content = %d \n", (int) avg, aggregated_payloads.count_payloads, aggregated_payloads.singleP[aggregated_payloads.count_payloads].intContent);		
				aggregated_payloads.count_payloads++;
				j = 0;
				sum = 0;
			}
		}else{
			// Fine ajust in the end
			//Thinking about
			printf("Some rcv payloads are left and should be aggregated \n");
		}
	}
}


    #endif


// ANDRE RIKER - ARIKER AT DEI.UC.PT
// UNIVERSITY OF COIMBRA

struct Payloads;

void add_payload(char *);

int get_rcv_payloads(int);
int get_aggregated_payloads(int);

int get_num_rcv_payloads();
int get_num_aggregated_payloads();

void aggregate_payloads();
void reset_payloads();

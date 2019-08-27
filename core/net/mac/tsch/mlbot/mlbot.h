
#include "net/mac/tsch/tsch-slot-operation.h"
#include "lib/random.h"

/***** External Variables *****/

extern long unsigned next_eb_scheduled;

extern uint16_t mlbot_lb_interval; 

int lb_tx;
int lb_sch;

/* Macros y const */

#define LB_MAX_LEN 25

/* Every few TS we will send the LB
* if we do not define anything by default it will be the default length of a slotframe */
#ifdef MLBOT_CONF_DEFAULT_LB_SEND_INTERVAL
#define MLBOT_LB_DEFAULT_SEND_INTERVAL MLBOT_CONF_DEFAULT_LB_SEND_INTERVAL
#else 
//#define MLBOT_LB_DEFAULT_SEND_INTERVAL TSCH_SCHEDULE_DEFAULT_LENGTH
#define MLBOT_LB_DEFAULT_SEND_INTERVAL 1

#endif

/* This constant should preferably be a power of 2 (1, 2, 4, 8 ...)
* this constant represents the inverse, if we define 2, the probability is 0.5
* if we define 4, it is 0.25 ... the higher the number, the lower the probability
* and less likely shipping frequency and less likely collisions
*/
#ifdef MLBOT_CONF_LB_PROBABILITY
#define MLBOT_LB_PROBABILITY MLBOT_CONF_LB_PROBABILITY
#else
/* The probability of sending an LB at 0.25 */
#define MLBOT_LB_PROBABILITY 4
#endif

/* Initializes MLBOT */
#ifdef TSCH_MLBOT_ON_OFF
#define TSCH_MLBOT_ON TSCH_MLBOT_ON_OFF
#else
#define TSCH_MLBOT_ON 1
#endif

/*  Process */
/*---------------------------------------------------------------------------*/
/* This is the main process that orchestrates the sending of the LB.
* Must be invoked once the tsch_send_eb_process process begins
* to send EBs.
*/
PROCESS_NAME(mlbot_process);


/* funciones */

/* Function that sends the LB */
int mlbot_send_lb(void);

/* Function that updates the LB */
void mlbot_lb_update(void);

/* Check if the package received by radio is an LB */
int check_lb_packet (struct input_packet *packet, frame802154_t *lb_frame);
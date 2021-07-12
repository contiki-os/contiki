#include "net/nbr-table.h"
#include "net/rpl/rpl-private.h"

#define DEBUG DEBUG_PRINT
#include "net/uip-debug.h"
#include "net/rpl/FIS.h"
#include "net/rpl/battery.h"

static void reset(rpl_dag_t *);
static void neighbor_link_callback(rpl_parent_t *, int, int);
static rpl_parent_t *best_parent(rpl_parent_t *, rpl_parent_t *);
static rpl_dag_t *best_dag(rpl_dag_t *, rpl_dag_t *);
static rpl_rank_t calculate_rank(rpl_parent_t *, rpl_rank_t);
static void update_metric_container(rpl_instance_t *);

rpl_of_t rpl_fuzzy = {
  reset,
  neighbor_link_callback,
  best_parent,
  best_dag,
  calculate_rank,
  update_metric_container,
  3
};

/* Constants for the ETX moving average */
#define ETX_SCALE   100
#define ETX_ALPHA   90

/* Reject parents that have a higher link metric than the following. */
#define MAX_LINK_METRIC			10

/* Reject parents that have a higher path cost than the following. */
#define MAX_PATH_COST			100

/*
 * The rank must differ more than 1/PARENT_SWITCH_THRESHOLD_DIV in order
 * to switch preferred parent.
 */
#define PARENT_SWITCH_THRESHOLD_DIV	2

//energy_est is a 8 bit number so energy max is 255
#define ENERGY_MAX 255
#define HOPCOUNT_MAX 255

typedef uint16_t rpl_path_metric_t;

static rpl_path_metric_t
calculate_path_metric(rpl_parent_t *p)
{
  if(p == NULL) {
    return MAX_PATH_COST * RPL_DAG_MC_ETX_DIVISOR;
  }
  return p->mc.obj.etx + (uint16_t)p->link_metric;
}

static uint8_t
calculate_energy_metric(rpl_parent_t *p)
{
  battery_charge_set();
  uint8_t energy = ENERGY_MAX - (uint8_t)(battery_charge_value/0x01000000);
  PRINTF("BAT: %u\n", energy);
  // return ENERGY_MAX - (energy_consumed/ENERGY_MAX);
  return energy;
}

static rpl_path_metric_t
calculate_hopcount_metric(rpl_parent_t *p)
{

	if(p == NULL){
		return HOPCOUNT_MAX;
	}
	return p->mc.obj.hopcount + 1;
}

static void
reset(rpl_dag_t *sag)
{
  PRINTF("RPL: Reset MRHOF\n");
}

static void
neighbor_link_callback(rpl_parent_t *p, int status, int numtx)
{
  uint16_t recorded_etx = p->link_metric;
  uint16_t packet_etx = numtx * RPL_DAG_MC_ETX_DIVISOR;
  uint16_t new_etx;

  /* Do not penalize the ETX when collisions or transmission errors occur. */
  if(status == MAC_TX_OK || status == MAC_TX_NOACK) {
    if(status == MAC_TX_NOACK) {
      packet_etx = MAX_LINK_METRIC * RPL_DAG_MC_ETX_DIVISOR;
    }

    new_etx = ((uint32_t)recorded_etx * ETX_ALPHA +
               (uint32_t)packet_etx * (ETX_SCALE - ETX_ALPHA)) / ETX_SCALE;

    PRINTF("RPL: ETX changed from %u to %u (packet ETX = %u)\n",
        (unsigned)(recorded_etx / RPL_DAG_MC_ETX_DIVISOR),
        (unsigned)(new_etx  / RPL_DAG_MC_ETX_DIVISOR),
        (unsigned)(packet_etx / RPL_DAG_MC_ETX_DIVISOR));
    p->link_metric = new_etx;
  }
}

static rpl_rank_t
calculate_rank(rpl_parent_t *p, rpl_rank_t base_rank)
{
  rpl_rank_t new_rank;
  rpl_rank_t rank_increase;

  if(p == NULL) {
    if(base_rank == 0) {
      return INFINITE_RANK;
    }
    rank_increase = RPL_INIT_LINK_METRIC * RPL_DAG_MC_ETX_DIVISOR;
  } else {
    rank_increase = p->link_metric;
    if(base_rank == 0) {
      base_rank = p->rank;
    }
  }

  if(INFINITE_RANK - base_rank < rank_increase) {
    /* Reached the maximum rank. */
    new_rank = INFINITE_RANK;
  } else {
   /* Calculate the rank based on the new rank information from DIO or
      stored otherwise. */
    new_rank = base_rank + rank_increase;
  }

  return new_rank;
}

static rpl_dag_t *
best_dag(rpl_dag_t *d1, rpl_dag_t *d2)
{
  if(d1->grounded != d2->grounded) {
    return d1->grounded ? d1 : d2;
  }

  if(d1->preference != d2->preference) {
    return d1->preference > d2->preference ? d1 : d2;
  }

  return d1->rank < d2->rank ? d1 : d2;
}

static rpl_parent_t *
best_parent(rpl_parent_t *p1, rpl_parent_t *p2)
{
  rpl_dag_t *dag;
  rpl_path_metric_t min_diff;
  rpl_path_metric_t p1_metric;
  rpl_path_metric_t p2_metric;

  dag = p1->dag; /* Both parents are in the same DAG. */

  min_diff = RPL_DAG_MC_ETX_DIVISOR / PARENT_SWITCH_THRESHOLD_DIV;

  p1_metric = quality(qos(p1->mc.obj.etx/RPL_DAG_MC_ETX_DIVISOR, 2, p1->mc.obj.hopcount), p1->mc.obj.energy.energy_est);
  p2_metric = quality(qos(p2->mc.obj.etx/RPL_DAG_MC_ETX_DIVISOR, 2, p2->mc.obj.hopcount), p2->mc.obj.energy.energy_est);
  // p1_metric = calculate_path_metric(p1);
  // p2_metric = calculate_path_metric(p2);


  /* Maintain stability of the preferred parent in case of similar ranks. */
  if(p1 == dag->preferred_parent || p2 == dag->preferred_parent) {
    if(p1_metric < p2_metric + min_diff &&
       p1_metric > p2_metric - min_diff) {
      PRINTF("RPL: MRHOF hysteresis: %u <= %u <= %u\n",
             p2_metric - min_diff,
             p1_metric,
             p2_metric + min_diff);
      return dag->preferred_parent;
    }
  }

  return p1_metric < p2_metric ? p1 : p2;
}

#if RPL_DAG_MC == RPL_DAG_MC_NONE
static void
update_metric_container(rpl_instance_t *instance)
{
  instance->mc.type = RPL_DAG_MC;
}
#else
static void
update_metric_container(rpl_instance_t *instance)
{
  rpl_path_metric_t path_metric, hopcount;
  uint8_t en;
  rpl_dag_t *dag;
  uint8_t type;

  instance->mc.type = RPL_DAG_MC;
  instance->mc.flags = RPL_DAG_MC_FLAG_P;
  instance->mc.aggr = RPL_DAG_MC_AGGR_ADDITIVE;
  instance->mc.prec = 0;

  dag = instance->current_dag;

  if (!dag->joined) {
    PRINTF("RPL: Cannot update the metric container when not joined\n");
    return;
  }

  instance->mc.length = 4;

  if(dag->rank == ROOT_RANK(instance)) {
    type = RPL_DAG_MC_ENERGY_TYPE_MAINS;
    path_metric = 0;
    en = 255;
    hopcount = 0;
  } else {
    type = RPL_DAG_MC_ENERGY_TYPE_BATTERY;
    path_metric = calculate_path_metric(dag->preferred_parent);
    en = calculate_energy_metric(dag->preferred_parent);
    hopcount = calculate_hopcount_metric(dag->preferred_parent);
  }
  instance->mc.obj.etx = path_metric;
  instance->mc.obj.energy.flags = type << RPL_DAG_MC_ENERGY_TYPE;
  instance->mc.obj.energy.energy_est = en;
  instance->mc.obj.hopcount = hopcount;
  PRINTF("RPL: etx:%u energy:%u hc:%u Quality:%u\n",
          instance->mc.obj.etx,
          instance->mc.obj.energy.energy_est,
          instance->mc.obj.hopcount,
          quality(qos(path_metric, 2, hopcount),en));

}
#endif /* RPL_DAG_MC == RPL_DAG_MC_NONE */

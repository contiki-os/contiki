/*
 * Copyright (c) 2014, SICS Swedish ICT.
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
 *         Interaction between TSCH and RPL
 *
 * \author Simon Duquennoy <simonduq@sics.se>
 */

#if UIP_CONF_IPV6_RPL

#include "contiki.h"
#include "net/rpl/rpl.h"
#include "net/rpl/rpl-private.h"
#include "net/mac/tsch/tsch.h"
#include "net/mac/tsch/tsch-private.h"
#include "net/mac/tsch/tsch-schedule.h"
#include "net/mac/tsch/tsch-log.h"
#include "tsch-rpl.h"

#if TSCH_LOG_LEVEL >= 1
#define DEBUG DEBUG_PRINT
#else /* TSCH_LOG_LEVEL */
#define DEBUG DEBUG_NONE
#endif /* TSCH_LOG_LEVEL */
#include "net/net-debug.h"

/*---------------------------------------------------------------------------*/
/* To use, set #define TSCH_CALLBACK_JOINING_NETWORK tsch_rpl_callback_joining_network */
void
tsch_rpl_callback_joining_network(void)
{
}
/*---------------------------------------------------------------------------*/
/* Upon leaving a TSCH network, perform a local repair
 * (cleanup neighbor state, reset Trickle timer etc)
 * To use, set #define TSCH_CALLBACK_LEAVING_NETWORK tsch_rpl_callback_leaving_network */
void
tsch_rpl_callback_leaving_network(void)
{
  rpl_dag_t *dag = rpl_get_any_dag();
  if(dag != NULL) {
    rpl_local_repair(dag->instance);
  }
}
/*---------------------------------------------------------------------------*/
/* Set TSCH EB period based on current RPL DIO period.
 * To use, set #define RPL_CALLBACK_NEW_DIO_INTERVAL tsch_rpl_callback_new_dio_interval */
void
tsch_rpl_callback_new_dio_interval(uint8_t dio_interval)
{
  /* Transmit EBs only if we have a valid rank as per 6TiSCH minimal */
  rpl_dag_t *dag = rpl_get_any_dag();
  if(dag != NULL && dag->rank != INFINITE_RANK) {
    /* If we are root set TSCH as coordinator */
    if(dag->rank == ROOT_RANK(dag->instance)) {
      tsch_set_coordinator(1);
    }
    /* Set EB period */
    tsch_set_eb_period((CLOCK_SECOND * 1UL << dag->instance->dio_intcurrent) / 1000);
    /* Set join priority based on RPL rank */
    tsch_set_join_priority(DAG_RANK(dag->rank, dag->instance) - 1);
  } else {
    tsch_set_eb_period(0);
  }
}
/*---------------------------------------------------------------------------*/
/* Set TSCH time source based on current RPL preferred parent.
 * To use, set #define RPL_CALLBACK_PARENT_SWITCH tsch_rpl_callback_parent_switch */
void
tsch_rpl_callback_parent_switch(rpl_parent_t *old, rpl_parent_t *new)
{
  if(tsch_is_associated == 1) {
    tsch_queue_update_time_source(
      (const linkaddr_t *)uip_ds6_nbr_lladdr_from_ipaddr(
        rpl_get_parent_ipaddr(new)));
  }
}
#endif /* UIP_CONF_IPV6_RPL */

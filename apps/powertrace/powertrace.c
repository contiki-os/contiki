/*
 * Copyright (c) 2010, Swedish Institute of Computer Science.
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
 * This file is part of the Contiki operating system.
 *
 */

/**
 * \file
 *         Powertrace and KIBAM battery
 * \author
 *         Code changed by André Riker <ariker@dei.uc.pt>
 */

#include "contiki.h"
#include "contiki-lib.h"
#include "sys/compower.h"
#include "powertrace.h"
#include "net/rime/rime.h"
#include <math.h>
#include <all-in-one.c>

#include <stdio.h>
#include <string.h>

struct powertrace_sniff_stats {
  struct powertrace_sniff_stats *next;
  unsigned long num_input, num_output;
  unsigned long input_txtime, input_rxtime;
  unsigned long output_txtime, output_rxtime;
#if NETSTACK_CONF_WITH_IPV6
  uint16_t proto; /* includes proto + possibly flags */
#endif
  uint16_t channel;
  unsigned long last_input_txtime, last_input_rxtime;
  unsigned long last_output_txtime, last_output_rxtime;
};

#define INPUT  1
#define OUTPUT 0

#define MAX_NUM_STATS  16

MEMB(stats_memb, struct powertrace_sniff_stats, MAX_NUM_STATS);
LIST(stats_list);




PROCESS(powertrace_process, "Periodic power output");



//####################################################################
//############# Piece of Code Added By André Riker ###################
//############# KIBAM BATTERY FOR CONTIKI ############################
//####################################################################


//RENATO - DISABLE HARVESTING.
int harvest = 1;

void setHarvestingOff(){
  harvest = 0;
}




unsigned timer_couter=0;
unsigned btt_interval_sec;
unsigned btt_interval_min;
double total_consumption=0;
double periodic_consumption=0;
unsigned solar_array_couter=0;
unsigned btt_interval_sec, btt_interval_min;
unsigned day_profile=0; // Change it if you want to start with a different day
short new_day=1;


struct battery{
  // Maximum amount of charge in battery (microAh) - static
   double qmax;
  // Available charge at beginning of time step (microAh)
   long double q1_0;
  // Bound charge at beginning of time step (microAh)
   long double q2_0;
  // Battery rate constant (rate at which chemically bound charge becomes available)
   double k;
  // Battery capacity ratio (fraction of total charge that is available) 
   double c;
  // Charge (-) or discharge (+) current of battery (A)
   double i;
  // Length of time step (micro hours)
   double dt;
  // parameter of the Kinetic model that is the result of exp(-k * dt)
   double r;
}; struct battery batt; // Parameters of the Battery 

struct stats{
unsigned long cpu;
unsigned long lpm;
unsigned long transmit;
unsigned long listen;
unsigned long idle_transmit;
unsigned long idle_listen;
}; struct stats stats_com;

struct energy_states{
	double active; //8Mhz
	double low_power;
	double tx;
	double rx;
}; 

// Intiate the struct with the eletrical current values in microA 
// for the following states: active, low power CPU, Tx and Rx.
// Current values for Wismote (microA)
struct energy_states i_energyStt = {0.001200*1000000, 0.000090*1000000, 0.0336*1000000, 0.0185*1000000};

//###############################################################################################
//###############################################################################################
//###############################################################################################


/*---------------------------------------------------------------------------*/
void
powertrace_print(char *str)
{

	
  static unsigned long last_cpu, last_lpm, last_transmit, last_listen;
  static unsigned long last_idle_transmit, last_idle_listen;

  unsigned long cpu, lpm, transmit, listen;
  unsigned long all_cpu, all_lpm, all_transmit, all_listen;
  unsigned long idle_transmit, idle_listen;
  unsigned long all_idle_transmit, all_idle_listen;

  static unsigned long seqno;

  unsigned long time, all_time, radio, all_radio;
  
  struct powertrace_sniff_stats *s;

  energest_flush();	

  all_cpu = energest_type_time(ENERGEST_TYPE_CPU);
  all_lpm = energest_type_time(ENERGEST_TYPE_LPM);
  all_transmit = energest_type_time(ENERGEST_TYPE_TRANSMIT);
  all_listen = energest_type_time(ENERGEST_TYPE_LISTEN);
  all_idle_transmit = compower_idle_activity.transmit;
  all_idle_listen = compower_idle_activity.listen;

  cpu = all_cpu - last_cpu;
  lpm = all_lpm - last_lpm;
  transmit = all_transmit - last_transmit;
  listen = all_listen - last_listen;
  idle_transmit = compower_idle_activity.transmit - last_idle_transmit;
  idle_listen = compower_idle_activity.listen - last_idle_listen;

  last_cpu = energest_type_time(ENERGEST_TYPE_CPU);
  last_lpm = energest_type_time(ENERGEST_TYPE_LPM);
  last_transmit = energest_type_time(ENERGEST_TYPE_TRANSMIT);
  last_listen = energest_type_time(ENERGEST_TYPE_LISTEN);
  last_idle_listen = compower_idle_activity.listen;
  last_idle_transmit = compower_idle_activity.transmit;

// ARIKER> KIBAM VARIALBES
  stats_com.cpu=cpu;
  printf("Defining CPU %lu\n", cpu);
  stats_com.lpm=lpm;
  stats_com.transmit=transmit;
  stats_com.listen=listen;
  stats_com.idle_transmit=idle_transmit;
  stats_com.idle_listen=idle_listen;

  radio = transmit + listen;
  time = cpu + lpm;
  all_time = all_cpu + all_lpm;
  all_radio = energest_type_time(ENERGEST_TYPE_LISTEN) + energest_type_time(ENERGEST_TYPE_TRANSMIT);

/*zprintf("Printing POWERTRACE stats: \n %s clock_time: %lu P %d.%d seqno %lu all_cpu %lu all_lpm %lu all_transmit %lu all_listen %lu all_idle_transmit %lu all_idle_listen %lu cpu %lu lpm %lu transmit %lu listen %lu idle_transmit %lu idle_listen %lu \n (radio %d.%02d%% / %d.%02d%% tx %d.%02d%% / %d.%02d%% listen %d.%02d%% / %d.%02d%%)\n\n",
         str,
         clock_time(), linkaddr_node_addr.u8[0], linkaddr_node_addr.u8[1], seqno,
         all_cpu, all_lpm, all_transmit, all_listen, all_idle_transmit, all_idle_listen,
         cpu, lpm, transmit, listen, idle_transmit, idle_listen,
         (int)((100L * (all_transmit + all_listen)) / all_time),
         (int)((10000L * (all_transmit + all_listen) / all_time) - (100L * (all_transmit + all_listen) / all_time) * 100),
         (int)((100L * (transmit + listen)) / time),
         (int)((10000L * (transmit + listen) / time) - (100L * (transmit + listen) / time) * 100),
         (int)((100L * all_transmit) / all_time),
         (int)((10000L * all_transmit) / all_time - (100L * all_transmit / all_time) * 100),
         (int)((100L * transmit) / time),
         (int)((10000L * transmit) / time - (100L * transmit / time) * 100),
         (int)((100L * all_listen) / all_time),
         (int)((10000L * all_listen) / all_time - (100L * all_listen / all_time) * 100),
         (int)((100L * listen) / time),
         (int)((10000L * listen) / time - (100L * listen / time) * 100));

printf("-------------------------------------------\n");
*/

  for(s = list_head(stats_list); s != NULL; s = list_item_next(s)) {

#if ! NETSTACK_CONF_WITH_IPV6
    printf("%s %lu SP %d.%d %lu %u %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu (channel %d radio %d.%02d%% / %d.%02d%%)\n",
           str, clock_time(), linkaddr_node_addr.u8[0], linkaddr_node_addr.u8[1], seqno,
           s->channel,
           s->num_input, s->input_txtime, s->input_rxtime,
           s->input_txtime - s->last_input_txtime,
           s->input_rxtime - s->last_input_rxtime,
           s->num_output, s->output_txtime, s->output_rxtime,
           s->output_txtime - s->last_output_txtime,
           s->output_rxtime - s->last_output_rxtime,
           s->channel,
           (int)((100L * (s->input_rxtime + s->input_txtime + s->output_rxtime + s->output_txtime)) / all_radio),
           (int)((10000L * (s->input_rxtime + s->input_txtime + s->output_rxtime + s->output_txtime)) / all_radio),
           (int)((100L * (s->input_rxtime + s->input_txtime +
                          s->output_rxtime + s->output_txtime -
                          (s->last_input_rxtime + s->last_input_txtime +
                           s->last_output_rxtime + s->last_output_txtime))) /
                 radio),
           (int)((10000L * (s->input_rxtime + s->input_txtime +
                          s->output_rxtime + s->output_txtime -
                          (s->last_input_rxtime + s->last_input_txtime +
                           s->last_output_rxtime + s->last_output_txtime))) /
                 radio));
#else
    printf("%s %lu SP %d.%d %lu %u %u %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu (proto %u(%u) radio %d.%02d%% / %d.%02d%%)\n",
           str, clock_time(), linkaddr_node_addr.u8[0], linkaddr_node_addr.u8[1], seqno,
           s->proto, s->channel,
           s->num_input, s->input_txtime, s->input_rxtime,
           s->input_txtime - s->last_input_txtime,
           s->input_rxtime - s->last_input_rxtime,
           s->num_output, s->output_txtime, s->output_rxtime,
           s->output_txtime - s->last_output_txtime,
           s->output_rxtime - s->last_output_rxtime,
           s->proto, s->channel,
           (int)((100L * (s->input_rxtime + s->input_txtime + s->output_rxtime + s->output_txtime)) / all_radio),
           (int)((10000L * (s->input_rxtime + s->input_txtime + s->output_rxtime + s->output_txtime)) / all_radio),
           (int)((100L * (s->input_rxtime + s->input_txtime +
                          s->output_rxtime + s->output_txtime -
                          (s->last_input_rxtime + s->last_input_txtime +
                           s->last_output_rxtime + s->last_output_txtime))) /
                 radio),
           (int)((10000L * (s->input_rxtime + s->input_txtime +
                          s->output_rxtime + s->output_txtime -
                          (s->last_input_rxtime + s->last_input_txtime +
                           s->last_output_rxtime + s->last_output_txtime))) /
                 radio));
#endif
    s->last_input_txtime = s->input_txtime;
    s->last_input_rxtime = s->input_rxtime;
    s->last_output_txtime = s->output_txtime;
    s->last_output_rxtime = s->output_rxtime;
    
  }
  seqno++;
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(powertrace_process, ev, data)
{
  static struct etimer periodic;
  clock_time_t *period;
  PROCESS_BEGIN();

  period = data;

  if(period == NULL) {
    PROCESS_EXIT();
  }
  etimer_set(&periodic, *period);

  while(1) {
    PROCESS_WAIT_UNTIL(etimer_expired(&periodic));
    etimer_reset(&periodic);
    // Ariker> Kibam Code
    update_battery();
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/

/*---------------------------------- RIKER ----------------------------------*/
//############################################################################
//####################### This Function has been Changed #####################
//####################### to enable Kibam Battery module #####################
//############################################################################
/*---------------------------------------------------------------------------*/
void
powertrace_start(clock_time_t period, unsigned seconds, double fixed_perc_energy, unsigned variation)
{
	double random_variable;
	random_variable = fixed_perc_energy + ((rand() % variation)*0.01);

  printf("Code 1\n");

	printf("-------------------------------------------\n");
	printf("KIBAM Battery has been started \n");
	
	printf("KIBAM Battery: rand() is %u and Random variable is %u \n",(unsigned) rand()%variation, (unsigned) (random_variable*100));
	
	batt.qmax = 1*1000000;
	batt.q1_0 = 1*1000000 * random_variable;
	batt.q2_0 = batt.qmax * (1 - random_variable) ; // q1 and q2 should start balanced
	batt.k = 0.1;
	batt.c = batt.q1_0/batt.qmax;
	batt.i = 0; 
	batt.dt = 0;
	batt.r = 0;


	printf("KIBAM Battery: qmax = %lu | q1 = %lu | q2 = %lu \n",(long unsigned) batt.qmax, (long unsigned) batt.q1_0, (long unsigned) batt.q2_0);
	printf("-------------------------------------------\n");
	
	btt_interval_sec=seconds;//gives it in seconds
	btt_interval_min=seconds/60;


  process_start(&powertrace_process, (void *)&period);
}
//###############################################################################
//###############################################################################
//###############################################################################
//###############################################################################
/*---------------------------------------------------------------------------*/
void
powertrace_stop(void)
{
  process_exit(&powertrace_process);
}
/*---------------------------------------------------------------------------*/
static void
add_stats(struct powertrace_sniff_stats *s, int input_or_output)
{
  if(input_or_output == INPUT) {
    s->num_input++;
    s->input_txtime += packetbuf_attr(PACKETBUF_ATTR_TRANSMIT_TIME);
    s->input_rxtime += packetbuf_attr(PACKETBUF_ATTR_LISTEN_TIME);
  } else if(input_or_output == OUTPUT) {
    s->num_output++;
    s->output_txtime += packetbuf_attr(PACKETBUF_ATTR_TRANSMIT_TIME);
    s->output_rxtime += packetbuf_attr(PACKETBUF_ATTR_LISTEN_TIME);
  }
}
/*---------------------------------------------------------------------------*/
static void
add_packet_stats(int input_or_output)
{
  struct powertrace_sniff_stats *s;

  /* Go through the list of stats to find one that matches the channel
     of the packet. If we don't find one, we allocate a new one and
     put it on the list. */
  for(s = list_head(stats_list); s != NULL; s = list_item_next(s)) {
    if(s->channel == packetbuf_attr(PACKETBUF_ATTR_CHANNEL)
#if NETSTACK_CONF_WITH_IPV6
       && s->proto == packetbuf_attr(PACKETBUF_ATTR_NETWORK_ID)
#endif
       ) {
      add_stats(s, input_or_output);
      break;
    }
  }
  if(s == NULL) {
    s = memb_alloc(&stats_memb);
    if(s != NULL) {
      memset(s, 0, sizeof(struct powertrace_sniff_stats));
      s->channel = packetbuf_attr(PACKETBUF_ATTR_CHANNEL);
#if NETSTACK_CONF_WITH_IPV6
      s->proto = packetbuf_attr(PACKETBUF_ATTR_NETWORK_ID);
#endif
      list_add(stats_list, s);
      add_stats(s, input_or_output);
    }
  }
}
/*---------------------------------------------------------------------------*/
static void
input_sniffer(void)
{
  add_packet_stats(INPUT);
}
/*---------------------------------------------------------------------------*/
static void
output_sniffer(int mac_status)
{
  add_packet_stats(OUTPUT);
}
/*---------------------------------------------------------------------------*/
#if NETSTACK_CONF_WITH_RIME
static void
sniffprint(char *prefix, int seqno)
{
  const linkaddr_t *esender;
  esender = packetbuf_addr(PACKETBUF_ADDR_ESENDER);

  printf("%lu %s %d %u %d %d %d.%d %u %u\n",
         clock_time(),
         prefix,
         linkaddr_node_addr.u8[0], seqno,
         packetbuf_attr(PACKETBUF_ATTR_CHANNEL),
         packetbuf_attr(PACKETBUF_ATTR_PACKET_TYPE),
         esender->u8[0], esender->u8[1],
         packetbuf_attr(PACKETBUF_ATTR_TRANSMIT_TIME),
         packetbuf_attr(PACKETBUF_ATTR_LISTEN_TIME));
}
/*---------------------------------------------------------------------------*/
static void
input_printsniffer(void)
{
  static int seqno = 0; 
  sniffprint("I", seqno++);

  if(packetbuf_attr(PACKETBUF_ATTR_CHANNEL) == 0) {
    int i;
    uint8_t *dataptr;

    printf("x %d ", packetbuf_totlen());
    dataptr = packetbuf_hdrptr();
    printf("%02x ", dataptr[0]);
    for(i = 1; i < packetbuf_totlen(); ++i) {
      printf("%02x ", dataptr[i]);
    }
    printf("\n");
  }
}
/*---------------------------------------------------------------------------*/
static void
output_printsniffer(int mac_status)
{
  static int seqno = 0;
  sniffprint("O", seqno++);
}
/*---------------------------------------------------------------------------*/
RIME_SNIFFER(printsniff, input_printsniffer, output_printsniffer);
/*---------------------------------------------------------------------------*/
void
powertrace_printsniff(powertrace_onoff_t onoff)
{
  switch(onoff) {
  case POWERTRACE_ON:
    rime_sniffer_add(&printsniff);
    break;
  case POWERTRACE_OFF:
    rime_sniffer_remove(&printsniff);
    break;
  }
}
#endif /* NETSTACK_CONF_WITH_RIME */
/*---------------------------------------------------------------------------*/
RIME_SNIFFER(powersniff, input_sniffer, output_sniffer);
/*---------------------------------------------------------------------------*/
void
powertrace_sniff(powertrace_onoff_t onoff)
{
  switch(onoff) {
  case POWERTRACE_ON:
    rime_sniffer_add(&powersniff);
    break;
  case POWERTRACE_OFF:
    rime_sniffer_remove(&powersniff);
    break;
  }
}


//####################################################################
//############# Piece of Code Added By André Riker ###################
//############# KIBAM BATTERY FOR CONTIKI ############################
//####################################################################


void kinetic_model(){
double q0, q1, q2, id_max, ic_max;
	q0 = batt.q1_0 + batt.q2_0;
    
    // Calculate maximum discharge and charging currents
    // id : current discharging
    // ic : current charging
    id_max = (batt.k * batt.q1_0 * batt.r + q0 * batt.k * batt.c * (1 - batt.r)) / (1 - batt.r + batt.c * (batt.k * batt.dt - 1 + batt.r));
    ic_max = (-batt.k * batt.c * batt.qmax + batt.k * batt.q1_0 * batt.r + q0 * batt.k * batt.c * (1 - batt.r)) / (1 - batt.r + batt.c * (batt.k * batt.dt - 1 + batt.r));
    
    // Check if battery current is within maximum bounds
    // If not, set battery current to limits
    if (batt.i > id_max) batt.i = id_max;
    if (batt.i < ic_max) batt.i = ic_max;
    
    // Update the available and bound charge
    q1 =  batt.q1_0 * batt.r + ((q0 * batt.k * batt.c - batt.i) * (1 - batt.r) - batt.i * batt.c * (batt.k * batt.dt - 1 + batt.r)) / batt.k ;
    q2 =  batt.q2_0 * batt.r + q0 * (1 - batt.c) * (1 - batt.r) - batt.i * (1 - batt.c) * (batt.k * batt.dt - 1 + batt.r) / batt.k ;
    
    batt.q1_0 = q1;
    batt.q2_0 = q2;

	}


void update_battery(){
  double convert_tick2sec=0.000030517578125;//A single time tick
  double convert_sec2hour=0.0002777777777778;// 1/3600
  unsigned long convert_tick2nano=30517;// a nanoseconds for a single time tick
  double voltage=0.2;//it is 1/5v
  double efficiency_painel=0.2;
  double convert2micro=1000000;
  double painel_area=210;//in cm2. 
  double current_draw[5];// in micro A
  unsigned long time_each_stt[5];
  double solar_charging;
  int stt_couter, num_states=5;
  long unsigned sim_time_minutes;
  unsigned start_sunlight=8, end_sunlight=start_sunlight+13;// in hours, considering 24h --- Trace is conf to run 13, which means between 8h to 21h
  unsigned duration_solartrace_value=1;// number of timer_counter's units which the solar value longs 

  
//-------- selects another day profile in the begining of a new day -----------------
  if(new_day==1){
  //day_profile=(unsigned)(random_rand()*100000L)*0.0001;//random_rand() returns 0 - 65375. So, this line makes day_profile a number between 0-6
  if(day_profile<6)
  day_profile++;
  else day_profile=0;
  printf("Battery: Day is %u \n",day_profile);
  new_day=0;
  }

//-------- Controls the time -----------------
  timer_couter=timer_couter+btt_interval_min;
  sim_time_minutes=timer_couter; // assuming the battery interval is 60 seconds - 1 minute
  printf("Battery: Timer couter is %u and Time is %u \n",timer_couter, (unsigned) sim_time_minutes);
  
//-------- Computes the current charging -----------------
  if((sim_time_minutes>=start_sunlight*60) && (sim_time_minutes<=end_sunlight*60)){// it is in the sun light time
	solar_charging=(unsigned long)(solar_array[day_profile][solar_array_couter]* efficiency_painel* voltage* painel_area);//read the array with solar current values (in micro Watt)
	printf("Battery: 1 Solar charging is %lu \n",(unsigned long)(solar_charging));
	solar_charging=-1*solar_charging;//the current should be negative in case of charging. Kinetic model requires it.
	if(sim_time_minutes % duration_solartrace_value == 0) solar_array_couter++; // For this solar trace, it is 5 min
    }
  else{ // there is no sun light, so it does not compute solar charging
	solar_charging=0;
	num_states=4;
	}
  if(solar_array_couter>=size_of_solar_array)solar_array_couter=0;//it should be an error
  if(sim_time_minutes>=(24*60)){// end of a day
	timer_couter=0;
	new_day=1;
	solar_array_couter=0;
	printf("Battery: End of day \n");
	}// reset for a new day

  //TURNING HARVESTING OFF TO CORRECT MEASUREMENT

  if(!harvest)
    solar_charging = 0;

  powertrace_print("");// In this fuction time in state is updated


//-------- Set the current spend/collect in every state -----------------

  current_draw[0] = i_energyStt.active;
  current_draw[1] = i_energyStt.low_power;
  current_draw[2] = i_energyStt.tx;
  current_draw[3] = i_energyStt.rx;
  current_draw[4] = solar_charging;//solar charging current. It should be negative
  
//-------- Set the time spent in every state -----------------
  //-------- Set the time spent in every state -----------------
  //powertrace_print("");// In this fuction time in state is updated
  time_each_stt[1] = stats_com.lpm;
  time_each_stt[2] = stats_com.transmit + stats_com.idle_transmit;
  time_each_stt[3] = stats_com.listen + stats_com.idle_listen;
  time_each_stt[4] = stats_com.cpu + stats_com.lpm; // solar charging time (in ticks) 
  

  printf("TESTE AQUI  Ó %lu", stats_com.cpu);

//-------- call the Kinetic model -----------------
  for(stt_couter=0;stt_couter<num_states;stt_couter++){
	// --------------------------------------- Variable Battery parameters ------------------------------
	// Define the charging or discharging current (microAh)
	batt.i=current_draw[stt_couter];
	
	if(stt_couter<4){ // this is because harvesting does not count for energy consumption
	total_consumption=total_consumption + (current_draw[stt_couter]*time_each_stt[stt_couter]*convert_tick2sec*convert_sec2hour);
	periodic_consumption = (current_draw[stt_couter]*time_each_stt[stt_couter]*convert_tick2sec*convert_sec2hour);
	}
	// Define the time step (hour)
	batt.dt=time_each_stt[stt_couter]*convert_tick2sec*convert_sec2hour; // convert from seconds to hours
	
  
	// Computing r
	batt.r=expf(-batt.k * batt.dt);
  
	// Call kinetic model
	kinetic_model();
 
	// prints
	printf("Battery: Available charge at next time interval is %lu (microAh) \n",(unsigned long) (batt.q1_0));
	printf("Battery: Bound charge at next time interval is %lu (microAh) \n",(unsigned long) (batt.q2_0));
	//PRINTF("Battery: Time of CPU in state %d in nanoseconds is %lu \n",j,(unsigned long)time_each_stt[j]*convert_tick2nano);
	printf("Battery: Number of CPU ticks in state %d is %lu \n",stt_couter,time_each_stt[stt_couter]);
  }
  
    // Print
	printf("Log: Residual (microAh); %lu; \n",(unsigned long) (batt.q1_0));
	//PRINTF("Battery: Bound charge at next time interval is %lu (microAh) \n",(unsigned long) (batt.q2_0));
	printf("Log: Energy Total consumption (microA); %lu;  \n",(unsigned long) (total_consumption));
	printf("Log: Energy Periodic consumption (microA); %d;  \n",(int) (periodic_consumption));
	printf("Energy: CPU ticks %lu, LPM %lu, Tx %lu, Rx %lu \n", stats_com.cpu, stats_com.lpm, stats_com.transmit, stats_com.listen);

}

//###############################################################################################
//###############################################################################################
//###############################################################################################

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


#include "contiki.h"
#include "contiki-lib.h"
#include "sys/compower.h"
#include "powertrace.h"
#include "net/rime/rime.h"

#include <stdio.h>
#include <string.h>




#define INPUT  1
#define OUTPUT 0

#define MAX_NUM_STATS  16



/**
 Powertrace tracks the duration of activities of a node being in each power state. In other words, the
outputs show the fraction of time that a node remains in a particular power state. In the above
example, there are 6 defined power states: CPU, LPM, TRANSMIT, LISTEN, IDLE_TRANSMIT,
IDLE_LISTEN.
Power states are measured with a hardware timer. The clock frequency is defined in
RTIMER_SECOND 
 */

/*---------------------------------------------------------------------------*/
float *powertrace_print1(char *str,int interval,float v_in)
{
  static unsigned long last_cpu, last_lpm, last_transmit, last_listen;
  static unsigned long last_idle_transmit, last_idle_listen;
  static float array[6];
  unsigned long cpu, lpm, transmit, listen;
  unsigned long all_cpu, all_lpm, all_transmit, all_listen;
  unsigned long idle_transmit, idle_listen;
  unsigned long all_idle_transmit, all_idle_listen;
  float cpu_sleep = 2.15, cpu_on = 4.5, transmit_power= 10.1, r = 14.4;
  static unsigned long seqno;

  unsigned long time, all_time, radio, all_radio;
  
  

  energest_flush();

 /* accummulated number of ticks*/ 
  all_cpu = energest_type_time(ENERGEST_TYPE_CPU);
  all_lpm = energest_type_time(ENERGEST_TYPE_LPM);
  all_transmit = energest_type_time(ENERGEST_TYPE_TRANSMIT);
  all_listen = energest_type_time(ENERGEST_TYPE_LISTEN);
  all_idle_transmit = compower_idle_activity.transmit;
  all_idle_listen = compower_idle_activity.listen;
  /*difference between the number of ticks in two time intervals*/
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

  

  radio = transmit + listen;
  time = cpu + lpm;
  all_time = all_cpu + all_lpm;
  all_radio = energest_type_time(ENERGEST_TYPE_LISTEN) +
    energest_type_time(ENERGEST_TYPE_TRANSMIT);

 /* printf("%s %lu P %d.%d %lu all_cpu %lu all_lpm %lu all_transmit %lu all_listen %lu all_idle_transmit %lu all_idle_listen %lu cpu %lu lpm %lu transmit %lu listen %lu  idle_transmit %lu idle_listen %lu  s\n",
         str,
         clock_time(), linkaddr_node_addr.u8[0], linkaddr_node_addr.u8[1], seqno,
         all_cpu, all_lpm, all_transmit, all_listen, all_idle_transmit, all_idle_listen,
         cpu, lpm, transmit, listen, idle_transmit, idle_listen
        );*/
        
  float energy_cpu = cal_power_consumption(cpu, cpu_on, interval, v_in);
  float energy_lpm = cal_power_consumption(lpm, cpu_sleep,interval, v_in);
  float energy_transmit = cal_power_consumption(transmit, transmit_power,interval, v_in);
  float energy_listen = cal_power_consumption(listen, r,interval, v_in);
//printf("cpu:%.3f mA lpm:%.3f mA transmit:%.3f mA listen:%.3f mA %.3f\n",energy_cpu,energy_lpm,energy_transmit,energy_listen,v_in);
   seqno++;
   array[0]= energy_cpu;
   array[1]= energy_lpm;
   array[2]= energy_transmit;
   array[3]= energy_listen;
	 
   //printf("cpu time:%d mA radio time:%.4f mA timer %d\n ", all_radio,(float) all_radio/RTIMER_SECOND,RTIMER_SECOND);	
  //printf("cpu:%.3f mA lpm:%.3f mA transmit:%.3f mA listen:%.3f mA \n",array[0],array[1],array[2],array[3]);
return array;
}

/*---------------------------------------------------------------------------*/
float
cal_power_consumption(unsigned long task, float current,int runtime,float voltage)
{
  
 return ((task*current*voltage)/(RTIMER_SECOND*runtime));
    
} 


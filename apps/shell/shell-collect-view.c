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
 *         Collect view client code
 * \author
 *         Adam Dunkels <adam@sics.se>
 */

#include "contiki.h"
#include "shell.h"
#include "collect-view.h"
#include "net/rime/broadcast-announcement.h"

/*---------------------------------------------------------------------------*/
PROCESS(collect_view_data_process, "collect-view-data");
SHELL_COMMAND(collect_view_data_command,
	      "collect-view-data",
	      "collect-view-data: sensor data, power consumption, network stats",
	      &collect_view_data_process);
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(collect_view_data_process, ev, data)
{
  struct collect_view_data_msg msg;
  struct collect_neighbor *n;
  uint16_t parent_etx;
  uint16_t num_neighbors;
  uint16_t beacon_interval;
  
  PROCESS_BEGIN();

  n = collect_neighbor_list_find(&shell_collect_conn.neighbor_list,
                                 &shell_collect_conn.parent);
  if(n != NULL) {
    parent_etx = collect_neighbor_link_estimate(n);
  } else {
    parent_etx = 0;
  }
  num_neighbors = collect_neighbor_list_num(&shell_collect_conn.neighbor_list);
  beacon_interval = broadcast_announcement_beacon_interval() / CLOCK_SECOND;

  collect_view_construct_message(&msg, &shell_collect_conn.parent,
                                 parent_etx, shell_collect_conn.rtmetric,
                                 num_neighbors, beacon_interval);
  shell_output(&collect_view_data_command, &msg, sizeof(msg), "", 0);

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
void
shell_collect_view_init(void)
{
  shell_register_command(&collect_view_data_command);
}
/*---------------------------------------------------------------------------*/

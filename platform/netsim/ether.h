/*
 * Copyright (c) 2004, Swedish Institute of Computer Science.
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
 * Author: Adam Dunkels <adam@sics.se>
 *
 * $Id: ether.h,v 1.9 2008/05/14 19:22:58 adamdunkels Exp $
 */
#ifndef __ETHER_H__
#define __ETHER_H__

#include "contiki.h"
#include "sensor.h"

struct ether_packet {
  struct ether_packet *next;
  char data[1500];
  int len;
  int x, y;
  int destx, desty;
};


void ether_send_done(void);

u8_t ether_send(char *data, int len);
void ether_set_leds(int leds);
void ether_set_text(char *text);

void ether_set_radio_status(int onoroff);
void ether_send_serial(char *text);


void ether_poll(void);
void ether_server_init(void);
void ether_client_init(int port);
void ether_tick(void);


u16_t ether_client_read(u8_t *buf, int len);
void  ether_server_poll(void);

void ether_put(char *packet, int len, int src_x, int src_y);

void ether_send_sensor_data(struct sensor_data *d, int srcx, int srcy, int strength);


int ether_client_poll(void);

struct ether_packet * ether_packets(void);

clock_time_t ether_time(void);

#define ETHER_PORT 4999
/*#define ETHER_STRENGTH 24*/
int ether_strength(void);
void ether_set_strength(int s);
void ether_set_collisions(int c);
void ether_set_drop_probability(double p);

void ether_print_stats(void);

void ether_set_line(int x, int y);


#endif /* __ETHER_H__ */

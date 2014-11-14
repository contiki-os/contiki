/*
 * Copyright (c) 2005, Swedish Institute of Computer Science
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
 *
 * \file
 * Code propagation and storage.
 * \author
 * Adam Dunkels <adam@sics.se>
 *
 * This file implements a simple form of code propagation, which
 * allows a binary program to be downloaded and propagated throughout
 * a network of devices.
 *
 * Features:
 *
 *    Commands: load code, start code
 *    Point-to-point download over TCP
 *    Point-to-multipoint delivery over UDP broadcasts
 *    Versioning of code modules
 *
 * Procedure:
 *
 *    1. Receive code over TCP
 *    2. Send code packets over UDP
 *
 *    When a code packet is deemed to be missed, a NACK is sent. If a
 *    NACK is received, the sending restarts at the point in the
 *    binary where the NACK pointed to. (This is *not* very efficient,
 *    but simple to implement...)
 *
 * States:
 *
 *  Receiving code header -> receiving code -> sending code
 *
 */

#include <stdio.h>

#include "contiki-net.h"
#include "cfs/cfs.h"
#include "codeprop-otf.h"
#include "loader/elfloader-otf.h"
#include <string.h>

static const char *err_msgs[] =
  {"OK\r\n", "Bad ELF header\r\n", "No symtab\r\n", "No strtab\r\n",
   "No text\r\n", "Symbol not found\r\n", "Segment not found\r\n",
   "No startpoint\r\n", "Unhandled relocation\r\n",
   "Relocation out of range\r\n", "Relocations not sorted\r\n",
   "Input error\r\n" , "Ouput error\r\n" };

#define CODEPROP_DATA_PORT 6510

/*static int random_rand(void) { return 1; }*/

#if 0
#define PRINTF(x) printf x
#else
#define PRINTF(x)
#endif

#define START_TIMEOUT 12 * CLOCK_SECOND
#define MISS_NACK_TIMEOUT (CLOCK_SECOND / 8) * (random_rand() % 8)
#define HIT_NACK_TIMEOUT (CLOCK_SECOND / 8) * (8 + random_rand() % 16)
#define NACK_REXMIT_TIMEOUT CLOCK_SECOND * (4 + random_rand() % 4)

#define WAITING_TIME CLOCK_SECOND * 10

#define NUM_SEND_DUPLICATES 2

#define UDPHEADERSIZE 8
#define UDPDATASIZE   32

struct codeprop_udphdr {
  uint16_t id;
  uint16_t type;
#define TYPE_DATA 0x0001
#define TYPE_NACK 0x0002
  uint16_t addr;
  uint16_t len;
  uint8_t data[UDPDATASIZE];
};

struct codeprop_tcphdr {
  uint16_t len;
  uint16_t pad;
};

static void uipcall(void *state);

PROCESS(codeprop_process, "Code propagator");

struct codeprop_state {
  uint8_t state;
#define STATE_NONE              0
#define STATE_RECEIVING_TCPDATA 1
#define STATE_RECEIVING_UDPDATA 2
#define STATE_SENDING_UDPDATA   3
  uint16_t count;
  uint16_t addr;
  uint16_t len;
  uint16_t id;
  struct etimer sendtimer;
  struct timer nacktimer, timer, starttimer;
  uint8_t received;
  uint8_t send_counter;
  struct pt tcpthread_pt;
  struct pt udpthread_pt;
  struct pt recv_udpthread_pt;
};

static int fd;

static struct uip_udp_conn *udp_conn;

static struct codeprop_state s;

void system_log(char *msg);

static clock_time_t send_time;

#define CONNECTION_TIMEOUT (30 * CLOCK_SECOND)

/*---------------------------------------------------------------------*/
void
codeprop_set_rate(clock_time_t time)
{
  send_time = time;
}
/*---------------------------------------------------------------------*/
PROCESS_THREAD(codeprop_process, ev, data)
{
  PROCESS_BEGIN();

  elfloader_init();

  s.id = 0/*random_rand()*/;

  send_time = CLOCK_SECOND/4;

  PT_INIT(&s.udpthread_pt);
  PT_INIT(&s.recv_udpthread_pt);

  tcp_listen(UIP_HTONS(CODEPROP_DATA_PORT));

  udp_conn = udp_broadcast_new(UIP_HTONS(CODEPROP_DATA_PORT), NULL);

  s.state = STATE_NONE;
  s.received = 0;
  s.addr = 0;
  s.len = 0;

  fd = cfs_open("codeprop-image", CFS_READ | CFS_WRITE);

  while(1) {

    PROCESS_YIELD();

    if(ev == tcpip_event) {
      uipcall(data);
    } else if(ev == PROCESS_EVENT_TIMER) {
      tcpip_poll_udp(udp_conn);
    }
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------*/
static uint16_t
send_udpdata(struct codeprop_udphdr *uh)
{
  uint16_t len;

  uh->type = UIP_HTONS(TYPE_DATA);
  uh->addr = uip_htons(s.addr);
  uh->id = uip_htons(s.id);

  if(s.len - s.addr > UDPDATASIZE) {
    len = UDPDATASIZE;
  } else {
    len = s.len - s.addr;
  }

  cfs_seek(fd, s.addr, CFS_SEEK_SET);
  cfs_read(fd, (char*)&uh->data[0], len);
  /*  eeprom_read(EEPROMFS_ADDR_CODEPROP + s.addr,
      &uh->data[0], len);*/

  uh->len = uip_htons(s.len);

  PRINTF(("codeprop: sending packet from address 0x%04x\n", s.addr));
  uip_udp_send(len + UDPHEADERSIZE);

  return len;
}
/*---------------------------------------------------------------------*/
static
PT_THREAD(send_udpthread(struct pt *pt))
{
  int len;
  struct codeprop_udphdr *uh = (struct codeprop_udphdr *)uip_appdata;


  PT_BEGIN(pt);

  while(1) {
    PT_WAIT_UNTIL(pt, s.state == STATE_SENDING_UDPDATA);

    for(s.addr = 0; s.addr < s.len; ) {
      len = send_udpdata(uh);
      s.addr += len;

      etimer_set(&s.sendtimer, CLOCK_SECOND/4);
      do {
	PT_WAIT_UNTIL(pt, uip_newdata() || etimer_expired(&s.sendtimer));

	if(uip_newdata()) {
	  if(uh->type == UIP_HTONS(TYPE_NACK)) {
	    PRINTF(("send_udpthread: got NACK for address 0x%x (now 0x%x)\n",
		    uip_htons(uh->addr), s.addr));
	    /* Only accept a NACK if it points to a lower byte. */
	    if(uip_htons(uh->addr) <= s.addr) {
	      /*	      beep();*/
	      s.addr = uip_htons(uh->addr);
	    }
	  }
	  PT_YIELD(pt);
	}
      } while(!etimer_expired(&s.sendtimer));
    }

    s.state = STATE_NONE;

/*     process_post(PROCESS_BROADCAST, codeprop_event_quit, (process_data_t)NULL); */
  }
  PT_END(pt);
}
/*---------------------------------------------------------------------*/
static void
send_nack(struct codeprop_udphdr *uh, unsigned short addr)
{
  uh->type = UIP_HTONS(TYPE_NACK);
  uh->addr = uip_htons(addr);
  uip_udp_send(UDPHEADERSIZE);
}
/*---------------------------------------------------------------------*/
static
PT_THREAD(recv_udpthread(struct pt *pt))
{
  int len;
  struct codeprop_udphdr *uh = (struct codeprop_udphdr *)uip_appdata;

  /*  if(uip_newdata()) {
    PRINTF(("recv_udpthread: id %d uh->id %d\n", s.id, uip_htons(uh->id)));
    }*/

  PT_BEGIN(pt);

  while(1) {

    do {
      PT_WAIT_UNTIL(pt, uip_newdata() &&
		    uh->type == UIP_HTONS(TYPE_DATA) &&
		    uip_htons(uh->id) > s.id);

      if(uip_htons(uh->addr) != 0) {
	s.addr = 0;
	send_nack(uh, 0);
      }

    } while(uip_htons(uh->addr) != 0);

    /*    leds_on(LEDS_YELLOW);
	  beep_down(10000);*/

    s.addr = 0;
    s.id = uip_htons(uh->id);
    s.len = uip_htons(uh->len);

    timer_set(&s.timer, CONNECTION_TIMEOUT);
/*     process_post(PROCESS_BROADCAST, codeprop_event_quit, (process_data_t)NULL); */

    while(s.addr < s.len) {

      if(uip_htons(uh->addr) == s.addr) {
	/*	leds_blink();*/
	len = uip_datalen() - UDPHEADERSIZE;
	if(len > 0) {
	  /*	  eeprom_write(EEPROMFS_ADDR_CODEPROP + s.addr,
		  &uh->data[0], len);*/
	  cfs_seek(fd, s.addr, CFS_SEEK_SET);
	  cfs_write(fd, (char*)&uh->data[0], len);

	  /*	  beep();*/
	  PRINTF(("Saved %d bytes at address %d, %d bytes left\n",
		  uip_datalen() - UDPHEADERSIZE, s.addr,
		  s.len - s.addr));

	  s.addr += len;
	}

      } else if(uip_htons(uh->addr) > s.addr) {
	PRINTF(("sending nack since 0x%x != 0x%x\n", uip_htons(uh->addr), s.addr));
	send_nack(uh, s.addr);
      }

      if(s.addr < s.len) {

	/*	timer_set(&s.nacktimer, NACK_TIMEOUT);*/

	do {
	  timer_set(&s.nacktimer, HIT_NACK_TIMEOUT);
	  PT_YIELD_UNTIL(pt, timer_expired(&s.nacktimer) ||
			 (uip_newdata() &&
			  uh->type == UIP_HTONS(TYPE_DATA) &&
			  uip_htons(uh->id) == s.id));
	  if(timer_expired(&s.nacktimer)) {
	    send_nack(uh, s.addr);
	  }
	} while(timer_expired(&s.nacktimer));
      }

    }

    /*    leds_off(LEDS_YELLOW);
	  beep_quick(2);*/
    /*    printf("Received entire bunary over udr\n");*/
    codeprop_start_program();
    PT_EXIT(pt);
  }

  PT_END(pt);
}
/*---------------------------------------------------------------------*/

#define CODEPROP_TCPHDR_SIZE sizeof(struct codeprop_tcphdr)

static
PT_THREAD(recv_tcpthread(struct pt *pt))
{
  struct codeprop_tcphdr *th;
  int datalen = uip_datalen();
  PT_BEGIN(pt);

  while(1) {

    PT_WAIT_UNTIL(pt, uip_connected());

    codeprop_exit_program();
    
    s.state = STATE_RECEIVING_TCPDATA;

    s.addr = 0;
    s.count = 0;

    /* Read the header. */
    PT_WAIT_UNTIL(pt, uip_newdata() && uip_datalen() > 0);

    if(uip_datalen() < CODEPROP_TCPHDR_SIZE) {
      PRINTF(("codeprop: header not found in first tcp segment\n"));
      uip_abort();
    }
    th = (struct codeprop_tcphdr *)uip_appdata;
    s.len = uip_htons(th->len);
    s.addr = 0;
    uip_appdata += CODEPROP_TCPHDR_SIZE;
    datalen -= CODEPROP_TCPHDR_SIZE;
    
    /* Read the rest of the data. */
    do {
      if(datalen > 0) {
	/* printf("Got %d bytes\n", datalen); */

	if (cfs_seek(fd, s.addr, CFS_SEEK_SET) != s.addr) {
	   PRINTF(("codeprop: seek in buffer file failed\n"));
	   uip_abort();
	}
					     
	if (cfs_write(fd, uip_appdata, datalen) != datalen) {
	  PRINTF(("codeprop: write to buffer file failed\n"));
	  uip_abort();
	}
	s.addr += datalen;
      }
      if(s.addr < s.len) {
	PT_YIELD_UNTIL(pt, uip_newdata());
      }
    } while(s.addr < s.len);
#if 1
    
    {
      static int err;
      
      err = codeprop_start_program();
      
      /* Print out the "OK"/error message. */
      do {
	if (err >= 0 && err < sizeof(err_msgs)/sizeof(char*)) {
	  uip_send(err_msgs[err], strlen(err_msgs[err]));
	} else {
	  uip_send("Unknown error\r\n", 15);
	}
	PT_WAIT_UNTIL(pt, uip_acked() || uip_rexmit() || uip_closed());
      } while(uip_rexmit());
      
      /* Close the connection. */
      uip_close();
    }
#endif
    ++s.id;
    s.state = STATE_SENDING_UDPDATA;
    tcpip_poll_udp(udp_conn);

    PT_WAIT_UNTIL(pt, s.state != STATE_SENDING_UDPDATA);
    /*    printf("recv_tcpthread: unblocked\n");*/
  }

  PT_END(pt);
}
/*---------------------------------------------------------------------*/
void
codeprop_start_broadcast(unsigned int len)
{
  s.addr = 0;
  s.len = len;
  ++s.id;
  s.state = STATE_SENDING_UDPDATA;
  tcpip_poll_udp(udp_conn);
}
/*---------------------------------------------------------------------*/
void
codeprop_exit_program(void)
{
  if(elfloader_autostart_processes != NULL) {
    autostart_exit(elfloader_autostart_processes);
  }
}
/*---------------------------------------------------------------------*/
int
codeprop_start_program(void)
{
  int err;

  codeprop_exit_program();

  err = elfloader_load(fd, codeprop_output);
  if(err == ELFLOADER_OK) {
    PRINTF(("codeprop: starting %s\n",
	    elfloader_autostart_processes[0]->name));
    autostart_start(elfloader_autostart_processes);
  }
  return err;
}
/*---------------------------------------------------------------------*/
static void
uipcall(void *state)
{
  if(uip_udpconnection()) {
    recv_udpthread(&s.recv_udpthread_pt);
    send_udpthread(&s.udpthread_pt);
  } else {
    if(uip_conn->lport == UIP_HTONS(CODEPROP_DATA_PORT)) {
      if(uip_connected()) {

	if(state == NULL) {
	  s.addr = 0;
	  s.count = 0;
	  PT_INIT(&s.tcpthread_pt);
	  process_poll(&codeprop_process);
	  tcp_markconn(uip_conn, &s);
/* 	  process_post(PROCESS_BROADCAST, codeprop_event_quit, */
/* 		       (process_data_t)NULL); */
	} else {
	  PRINTF(("codeprop: uip_connected() and state != NULL\n"));
	  uip_abort();
	}
      }
      recv_tcpthread(&s.tcpthread_pt);


      if(uip_closed() || uip_aborted() || uip_timedout()) {
	PRINTF(("codeprop: connection down\n"));
	tcp_markconn(uip_conn, NULL);
      }
    }
  }
}
/*---------------------------------------------------------------------*/

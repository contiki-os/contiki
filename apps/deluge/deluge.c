/*
 * Copyright (c) 2007, Swedish Institute of Computer Science
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
 * $Id: deluge.c,v 1.10 2010/06/14 18:58:45 adamdunkels Exp $
 */

/**
 * \file
 *	An implementation of the Deluge protocol.
 *      (Hui and Culler: The dynamic behavior of a data 
 *      dissemination protocol for network programming at scale,
 *      ACM SenSys 2004)
 * \author
 * 	Nicolas Tsiftes <nvt@sics.se>
 */

#include "contiki.h"
#include "net/rime.h"
#include "cfs/cfs.h"
#include "loader/elfloader.h"
#include "lib/crc16.h"
#include "lib/random.h"
#include "node-id.h"
#include "deluge.h"

#if NETSIM
#include "ether.h"
#include <stdio.h>
#endif

#include "dev/leds.h"
#include <stdlib.h>
#include <string.h>

#define DEBUG	0
#if DEBUG
#include <stdio.h>
#define PRINTF(...)	printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif

/* Implementation-specific variables. */
static struct broadcast_conn deluge_broadcast;
static struct unicast_conn deluge_uc;
static struct deluge_object current_object;
static process_event_t deluge_event;

/* Deluge variables. */
static int deluge_state;
static int old_summary;
static int neighbor_inconsistency;
static unsigned r_interval;
static unsigned recv_adv;
static int broadcast_profile;

/* Deluge timers. */
static struct ctimer rx_timer;
static struct ctimer tx_timer;
static struct ctimer summary_timer;
static struct ctimer profile_timer;

/* Deluge objects will get an ID that defaults to the current value of
   the next_object_id parameter. */
static deluge_object_id_t next_object_id;

/* Rime callbacks. */
static void broadcast_recv(struct broadcast_conn *, const rimeaddr_t *);
static void unicast_recv(struct unicast_conn *, const rimeaddr_t *);

static const struct broadcast_callbacks broadcast_call = {broadcast_recv, NULL};
static const struct unicast_callbacks unicast_call = {unicast_recv, NULL};

/* The Deluge process manages the main Deluge timer. */
PROCESS(deluge_process, "Deluge");

static void
transition(int state)
{
  if(state != deluge_state) {
    switch(deluge_state) {
    case DELUGE_STATE_MAINTAIN:
      ctimer_stop(&summary_timer);
      ctimer_stop(&profile_timer);
      break;
    case DELUGE_STATE_RX:
      ctimer_stop(&rx_timer);
      break;
    case DELUGE_STATE_TX:
      ctimer_stop(&tx_timer);
      break;
    }
    deluge_state = state;
  }
}

static int
write_page(struct deluge_object *obj, unsigned pagenum, unsigned char *data)
{
  cfs_offset_t offset;

  offset = pagenum * S_PAGE;

  if(cfs_seek(obj->cfs_fd, offset, CFS_SEEK_SET) != offset) {
    return -1;
  }
  return cfs_write(obj->cfs_fd, (char *)data, S_PAGE);
}

static int
read_page(struct deluge_object *obj, unsigned pagenum, unsigned char *buf)
{
  cfs_offset_t offset;

  offset = pagenum * S_PAGE;

  if(cfs_seek(obj->cfs_fd, offset, CFS_SEEK_SET) != offset) {
    return -1;
  }
  return cfs_read(obj->cfs_fd, (char *)buf, S_PAGE);
}

static void
init_page(struct deluge_object *obj, int pagenum, int have)
{
  struct deluge_page *page;
  unsigned char buf[S_PAGE];

  page = &obj->pages[pagenum];

  page->flags = 0;
  page->last_request = 0;
  page->last_data = 0;

  if(have) {
    page->version = obj->version;
    page->packet_set = ALL_PACKETS;
    page->flags |= PAGE_COMPLETE;
    read_page(obj, pagenum, buf);
    page->crc = crc16_data(buf, S_PAGE, 0);
  } else {
    page->version = 0;
    page->packet_set = 0;
  }
}

static cfs_offset_t
file_size(const char *file)
{
  int fd;
  cfs_offset_t size;

  fd = cfs_open(file, CFS_READ);
  if(fd < 0) {
    return (cfs_offset_t)-1;
  }

  size = cfs_seek(fd, 0, CFS_SEEK_END);
  cfs_close(fd);

  return size;
}

static int
init_object(struct deluge_object *obj, char *filename, unsigned version)
{
  static struct deluge_page *page;
  int i;

  obj->cfs_fd = cfs_open(filename, CFS_READ | CFS_WRITE);
  if(obj->cfs_fd < 0) {
    return -1;
  }

  obj->filename = filename;
  obj->object_id = next_object_id++;
  obj->size = file_size(filename);
  obj->version = obj->update_version = version;
  obj->current_rx_page = 0;
  obj->nrequests = 0;
  obj->tx_set = 0;

  obj->pages = malloc(OBJECT_PAGE_COUNT(*obj) * sizeof(*obj->pages));
  if(obj->pages == NULL) {
    cfs_close(obj->cfs_fd);
    return -1; 
  }

  for(i = 0; i < OBJECT_PAGE_COUNT(current_object); i++) {
    page = &current_object.pages[i];
    init_page(&current_object, i, 1);
  }

  memset(obj->current_page, 0, sizeof(obj->current_page));

  return 0;
}

static int
highest_available_page(struct deluge_object *obj)
{
  int i;

  for(i = 0; i < OBJECT_PAGE_COUNT(*obj); i++) {
    if(!(obj->pages[i].flags & PAGE_COMPLETE)) {
      break;
    }
  }

  return i;
}

static void
send_request(void *arg)
{
  struct deluge_object *obj;
  struct deluge_msg_request request;

  obj = (struct deluge_object *)arg;

  request.cmd = DELUGE_CMD_REQUEST;
  request.pagenum = obj->current_rx_page;
  request.version = obj->pages[request.pagenum].version;
  request.request_set = ~obj->pages[obj->current_rx_page].packet_set;
  request.object_id = obj->object_id;

  PRINTF("Sending request for page %d, version %u, request_set %u\n", 
	request.pagenum, request.version, request.request_set);
  packetbuf_copyfrom(&request, sizeof(request));
  unicast_send(&deluge_uc, &obj->summary_from);

  /* Deluge R.2 */
  if(++obj->nrequests == CONST_LAMBDA) {
    /* XXX check rate here too. */
    obj->nrequests = 0;
    transition(DELUGE_STATE_MAINTAIN);
  } else {
    ctimer_reset(&rx_timer);
  }
}

static void
advertise_summary(struct deluge_object *obj)
{
  struct deluge_msg_summary summary;

  if(recv_adv >= CONST_K) {
    ctimer_stop(&summary_timer);
    return;
  }

  summary.cmd = DELUGE_CMD_SUMMARY;
  summary.version = obj->update_version;
  summary.highest_available = highest_available_page(obj);
  summary.object_id = obj->object_id;

  PRINTF("Advertising summary for object id %u: version=%u, available=%u\n",
	(unsigned)obj->object_id, summary.version, summary.highest_available);

  packetbuf_copyfrom(&summary, sizeof(summary));
  broadcast_send(&deluge_broadcast);
}

static void
handle_summary(struct deluge_msg_summary *msg, const rimeaddr_t *sender)
{
  int highest_available, i;
  clock_time_t oldest_request, oldest_data, now;
  struct deluge_page *page;

  highest_available = highest_available_page(&current_object);

  if(msg->version != current_object.version ||
      msg->highest_available != highest_available) {
    neighbor_inconsistency = 1;
  } else {
    recv_adv++;
  }

  if(msg->version < current_object.version) {
    old_summary = 1;
    broadcast_profile = 1;
  }

  /* Deluge M.5 */
  if(msg->version == current_object.update_version &&
     msg->highest_available > highest_available) {
    if(msg->highest_available > OBJECT_PAGE_COUNT(current_object)) {
      PRINTF("Error: highest available is above object page count!\n");
      return;
    }

    oldest_request = oldest_data = now = clock_time();
    for(i = 0; i < msg->highest_available; i++) {
      page = &current_object.pages[i];
      if(page->last_request < oldest_request) {
	oldest_request = page->last_request;
      }
      if(page->last_request < oldest_data) {
	oldest_data = page->last_data;
      }
    }

    if(((now - oldest_request) / CLOCK_SECOND) <= 2 * r_interval ||
	((now - oldest_data) / CLOCK_SECOND) <= r_interval) {
      return;
    }

    rimeaddr_copy(&current_object.summary_from, sender);
    transition(DELUGE_STATE_RX);

    if(ctimer_expired(&rx_timer)) {
      ctimer_set(&rx_timer,
	CONST_OMEGA * ESTIMATED_TX_TIME + ((unsigned)random_rand() % T_R),
	send_request, &current_object);
    }
  }
}

static void
send_page(struct deluge_object *obj, unsigned pagenum)
{
  unsigned char buf[S_PAGE];
  struct deluge_msg_packet pkt;
  unsigned char *cp;

  pkt.cmd = DELUGE_CMD_PACKET;
  pkt.pagenum = pagenum;
  pkt.version = obj->pages[pagenum].version;
  pkt.packetnum = 0;
  pkt.object_id = obj->object_id;
  pkt.crc = 0;

  read_page(obj, pagenum, buf);

  /* Divide the page into packets and send them one at a time. */
  for(cp = buf; cp + S_PKT <= (unsigned char *)&buf[S_PAGE]; cp += S_PKT) {
    if(obj->tx_set & (1 << pkt.packetnum)) {
      pkt.crc = crc16_data(cp, S_PKT, 0);
      memcpy(pkt.payload, cp, S_PKT);
      packetbuf_copyfrom(&pkt, sizeof(pkt));
      broadcast_send(&deluge_broadcast);
    }
    pkt.packetnum++;
  }
  obj->tx_set = 0;
}

static void
tx_callback(void *arg)
{
  struct deluge_object *obj;

  obj = (struct deluge_object *)arg;
  if(obj->current_tx_page >= 0 && obj->tx_set) {
    send_page(obj, obj->current_tx_page);
    /* Deluge T.2. */
    if(obj->tx_set) {
      packetbuf_set_attr(PACKETBUF_ATTR_PACKET_TYPE,
			 PACKETBUF_ATTR_PACKET_TYPE_STREAM);
      ctimer_reset(&tx_timer);
    } else {
      packetbuf_set_attr(PACKETBUF_ATTR_PACKET_TYPE,
			 PACKETBUF_ATTR_PACKET_TYPE_STREAM_END);
      obj->current_tx_page = -1;
      transition(DELUGE_STATE_MAINTAIN);
    }
  }
}

static void
handle_request(struct deluge_msg_request *msg)
{
  int highest_available;

  if(msg->pagenum >= OBJECT_PAGE_COUNT(current_object)) {
    return;
  }

  if(msg->version != current_object.version) {
    neighbor_inconsistency = 1;
  }

  highest_available = highest_available_page(&current_object);

  /* Deluge M.6 */
  if(msg->version == current_object.version &&
      msg->pagenum <= highest_available) {
    current_object.pages[msg->pagenum].last_request = clock_time();

    /* Deluge T.1 */
    if(msg->pagenum == current_object.current_tx_page) {
      current_object.tx_set |= msg->request_set;
    } else {
      current_object.current_tx_page = msg->pagenum;
      current_object.tx_set = msg->request_set;
    }

    transition(DELUGE_STATE_TX);
    ctimer_set(&tx_timer, CLOCK_SECOND, tx_callback, &current_object);
  }
}

static void
handle_packet(struct deluge_msg_packet *msg)
{
  struct deluge_page *page;
  uint16_t crc;
  struct deluge_msg_packet packet;

  memcpy(&packet, msg, sizeof(packet));

  PRINTF("Incoming packet for object id %u, version %u, page %u, packet num %u!\n",
	(unsigned)packet.object_id, (unsigned)packet.version,
	(unsigned)packet.pagenum, (unsigned)packet.packetnum);

  if(packet.pagenum != current_object.current_rx_page) {
    return;
  }

  if(packet.version != current_object.version) {
    neighbor_inconsistency = 1;
  }

  page = &current_object.pages[packet.pagenum];
  if(packet.version == page->version && !(page->flags & PAGE_COMPLETE)) {
    memcpy(&current_object.current_page[S_PKT * packet.packetnum],
	packet.payload, S_PKT);

    crc = crc16_data(packet.payload, S_PKT, 0);
    if(packet.crc != crc) {
      PRINTF("packet crc: %hu, calculated crc: %hu\n", packet.crc, crc);
      return;
    }

    page->last_data = clock_time();
    page->packet_set |= (1 << packet.packetnum);

    if(page->packet_set == ALL_PACKETS) {
      /* This is the last packet of the requested page; stop streaming. */
      packetbuf_set_attr(PACKETBUF_ATTR_PACKET_TYPE,
			 PACKETBUF_ATTR_PACKET_TYPE_STREAM_END);

      write_page(&current_object, packet.pagenum, current_object.current_page);
      page->version = packet.version;
      page->flags = PAGE_COMPLETE;
      PRINTF("Page %u completed\n", packet.pagenum);

      current_object.current_rx_page++;

      if(packet.pagenum == OBJECT_PAGE_COUNT(current_object) - 1) {
	current_object.version = current_object.update_version;
	leds_on(LEDS_RED);
	PRINTF("Update completed for object %u, version %u\n", 
	       (unsigned)current_object.object_id, packet.version);
      } else if(current_object.current_rx_page < OBJECT_PAGE_COUNT(current_object)) {
        if(ctimer_expired(&rx_timer)) {
	  ctimer_set(&rx_timer,
		CONST_OMEGA * ESTIMATED_TX_TIME + (random_rand() % T_R),
		send_request, &current_object);
	}
      }
      /* Deluge R.3 */
      transition(DELUGE_STATE_MAINTAIN);
    } else {
      /* More packets to come. Put lower layers in streaming mode. */
      packetbuf_set_attr(PACKETBUF_ATTR_PACKET_TYPE,
			 PACKETBUF_ATTR_PACKET_TYPE_STREAM);
    }
  }
}

static void
send_profile(struct deluge_object *obj)
{
  struct deluge_msg_profile *msg;
  unsigned char buf[sizeof(*msg) + OBJECT_PAGE_COUNT(*obj)];
  int i;

  if(broadcast_profile && recv_adv < CONST_K) {
    broadcast_profile = 0;

    msg = (struct deluge_msg_profile *)buf;
    msg->cmd = DELUGE_CMD_PROFILE;
    msg->version = obj->version;
    msg->npages = OBJECT_PAGE_COUNT(*obj);
    msg->object_id = obj->object_id;
    for(i = 0; i < msg->npages; i++) {
      msg->version_vector[i] = obj->pages[i].version;
    }

    packetbuf_copyfrom(buf, sizeof(buf));
    broadcast_send(&deluge_broadcast);
  }
}

static void
handle_profile(struct deluge_msg_profile *msg)
{
  int i;
  int npages;
  struct deluge_object *obj;
  char *p;

  obj = &current_object;
  if(msg->version <= current_object.update_version) {
    return;
  }

  PRINTF("Received profile of version %u with a vector of %u pages.\n",
	msg->version, msg->npages);

  leds_off(LEDS_RED);
  current_object.tx_set = 0;

  npages = OBJECT_PAGE_COUNT(*obj);
  obj->size = msg->npages * S_PAGE;

  p = malloc(OBJECT_PAGE_COUNT(*obj) * sizeof(*obj->pages));
  if(p == NULL) {
    PRINTF("Failed to reallocate memory for pages!\n");
    return;
  }

  memcpy(p, obj->pages, npages * sizeof(*obj->pages));
  free(obj->pages);
  obj->pages = (struct deluge_page *)p;

  if(msg->npages < npages) {
    npages = msg->npages;
  }

  for(i = 0; i < npages; i++) {
    if(msg->version_vector[i] > obj->pages[i].version) {
      obj->pages[i].packet_set = 0;
      obj->pages[i].flags &= ~PAGE_COMPLETE;
      obj->pages[i].version = msg->version_vector[i];
    }
  }

  for(; i < msg->npages; i++) {
    init_page(obj, i, 0);
  }

  obj->current_rx_page = highest_available_page(obj);
  obj->update_version = msg->version;

  transition(DELUGE_STATE_RX);

  ctimer_set(&rx_timer,
	CONST_OMEGA * ESTIMATED_TX_TIME + ((unsigned)random_rand() % T_R),
	send_request, obj);
}

static void
command_dispatcher(const rimeaddr_t *sender)
{
  char *msg;
  int len;
  struct deluge_msg_profile *profile;

  msg = packetbuf_dataptr();  
  len = packetbuf_datalen();
  if(len < 1)
    return;

  switch(msg[0]) {
  case DELUGE_CMD_SUMMARY:
    if(len >= sizeof(struct deluge_msg_summary))
      handle_summary((struct deluge_msg_summary *)msg, sender);
    break;
  case DELUGE_CMD_REQUEST:
    if(len >= sizeof(struct deluge_msg_request))
      handle_request((struct deluge_msg_request *)msg);
    break;
  case DELUGE_CMD_PACKET:
    if(len >= sizeof(struct deluge_msg_packet))
      handle_packet((struct deluge_msg_packet *)msg);
    break;
  case DELUGE_CMD_PROFILE:
    profile = (struct deluge_msg_profile *)msg;
    if(len >= sizeof(*profile) &&
       len >= sizeof(*profile) + profile->npages * profile->version_vector[0])
      handle_profile((struct deluge_msg_profile *)msg);
    break;
  default:
    PRINTF("Incoming packet with unknown command: %d\n", msg[0]);
  }
}

static void
unicast_recv(struct unicast_conn *c, const rimeaddr_t *sender)
{
  command_dispatcher(sender);
}

static void
broadcast_recv(struct broadcast_conn *c, const rimeaddr_t *sender)
{
  command_dispatcher(sender);
}

int
deluge_disseminate(char *file, unsigned version)
{
  /* This implementation disseminates at most one object. */
  if(next_object_id > 0 || init_object(&current_object, file, version) < 0) {
    return -1;
  }
  process_start(&deluge_process, file);

  return 0;
}

PROCESS_THREAD(deluge_process, ev, data)
{
  static struct etimer et;
  static unsigned time_counter;
  static unsigned r_rand;

  PROCESS_EXITHANDLER(goto exit);

  PROCESS_BEGIN();

  deluge_event = process_alloc_event();

  broadcast_open(&deluge_broadcast, DELUGE_BROADCAST_CHANNEL, &broadcast_call);
  unicast_open(&deluge_uc, DELUGE_UNICAST_CHANNEL, &unicast_call);
  r_interval = T_LOW;

  PRINTF("Maintaining state for object %s of %d pages\n",
	current_object.filename, OBJECT_PAGE_COUNT(current_object));

  deluge_state = DELUGE_STATE_MAINTAIN;

  for(r_interval = T_LOW;;) {
    if(neighbor_inconsistency) {
      /* Deluge M.2 */
      r_interval = T_LOW;
      neighbor_inconsistency = 0;
    } else {
      /* Deluge M.3 */
      r_interval = (2 * r_interval >= T_HIGH) ? T_HIGH : 2 * r_interval;
    }

    r_rand = r_interval / 2 + ((unsigned)random_rand() % (r_interval / 2));
    recv_adv = 0;
    old_summary = 0;

    /* Deluge M.1 */
    ctimer_set(&summary_timer, r_rand * CLOCK_SECOND,
	(void *)(void *)advertise_summary, &current_object);

    /* Deluge M.4 */
    ctimer_set(&profile_timer, r_rand * CLOCK_SECOND,
	(void *)(void *)send_profile, &current_object);

    LONG_TIMER(et, time_counter, r_interval);
  }

exit:
  unicast_close(&deluge_uc);
  broadcast_close(&deluge_broadcast);
  if(current_object.cfs_fd >= 0) {
    cfs_close(current_object.cfs_fd);
  }
  if(current_object.pages != NULL) {
    free(current_object.pages);
  }

  PROCESS_END();
}

/**
 * \addtogroup rime
 * @{
 */

/**
 * \defgroup packetqueue Packet queue
 * @{
 *
 * The packetqueue module handles a list of queued packets.
 *
 */

/*
 * Copyright (c) 2009, Swedish Institute of Computer Science.
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
 *         Header file for the packetqueue module
 * \author
 *         Adam Dunkels <adam@sics.se>
 */

#ifndef PACKETQUEUE_H_
#define PACKETQUEUE_H_

#include "lib/list.h"
#include "lib/memb.h"

#include "sys/ctimer.h"

#include "net/packetbuf.h"
#include "net/queuebuf.h"

/**
 * \brief      Representation of a packet queue.
 *
 *             This structure holds the state of a packet queue. It is
 *             an opaque structure with no user-visible elements.
 */
struct packetqueue {
  list_t *list;
  struct memb *memb;
};

/**
 * \brief      Representation of an item in a packet queue.
 *
 *             This structure holds the state of a packet queue. It is
 *             an opaque structure with no user-visible elements. The
 *             function packetqueue_queuebuf() is used to extract a
 *             \ref queuebuf "queubuf" from the item. The function
 *             packetqueue_ptr() is used to extract the opaque pointer
 *             that was registered with the
 *             packetqueue_enqueue_packetbuf() function.
 */
struct packetqueue_item {
  struct packetqueue_item *next;
  struct queuebuf *buf;
  struct packetqueue *queue;
  struct ctimer lifetimer;
  void *ptr;
};


/**
 * \brief      Define a packet queue.
 * \param name The variable name of the packet queue
 * \param size The maximum size of the packet queue
 *
 *             This statement defines a packet queue. A packet queue
 *             is defined on a per-module basis.
 *
 */
#define PACKETQUEUE(name, size) LIST(name##_list); \
                                MEMB(name##_memb, struct packetqueue_item, size); \
				static struct packetqueue name = { &name##_list, \
								   &name##_memb }

/**
 * \name Packet queue functions.
 * @{
 */
/**
 * \brief      Initialize a packet queue.
 * \param q    A pointer to a struct packetqueue that was defined with PACKETQUEUE().
 *
 *             This function initializes a packetqueue that has
 *             previously been defined with PACKETQUEUE().
 *
 */
void packetqueue_init(struct packetqueue *q);


/**
 * \brief      Enqueue a packetbuf on a packet queue.
 * \param q    A pointer to a struct packetqueue.
 * \param lifetime The maximum time that the packet should stay in the packet queue, or zero if the packet should stay on the packet queue indefinitely.
 * \param ptr  An opaque, user-defined pointer that can be used to identify the packet when it later is dequeued.
 * \retval Zero   If memory could not be allocated for the packet.
 * \retval Non-zero If the packet was successfully enqueued.
 *
 *
 *             This function enqueues the \ref packetbuf "packetbuf"
 *             to the packet queue pointed to by the q parameter. The
 *             packet queue must previously have been defined with
 *             PACKETQUEUE() and initialized with packetqueue_init().
 *
 *             Each packet queue item has a maximum lifetime. When the
 *             lifetime expires, the packet queue item is
 *             automatically removed from the packet queue. If the
 *             lifetime parameter is given as zero, the packet never
 *             times out from the packet queue.
 *
 *             Each packet queue item is tagged with a user-defined
 *             pointer. This pointer can be used to identify packets
 *             as they later are dequeued from the queue. This is
 *             useful if two modules is using the same packet queue:
 *             the modules can use the pointer to distinguish to which
 *             module a dequeued packet belongs.
 *
 */
int packetqueue_enqueue_packetbuf(struct packetqueue *q, clock_time_t lifetime,
				  void *ptr);

/**
 * \brief      Access the first item on the packet buffer.
 * \param q    A pointer to a struct packetqueue.
 * \return     A pointer to the first item on the packet queue.
 *
 *             This function returns the first item on the packet
 *             queue. The packet queue is unchanged by this
 *             function. To dequeue the first item on the list, use
 *             the packetqueue_dequeue() function.
 *
 */
struct packetqueue_item *packetqueue_first(struct packetqueue *q);

/**
 * \brief      Remove the first item on the packet buffer.
 * \param q    A pointer to a struct packetqueue.
 *
 *             This function removes the first item on the packet
 *             queue. The function does not return the first item: to
 *             access the first item, the packetqueue_first() function
 *             must have been used prior to calling
 *             packetqueue_dequeue().
 *
 */
void packetqueue_dequeue(struct packetqueue *q);

/**
 * \brief      Get the length of the packet queue
 * \param q    A pointer to a struct packetqueue.
 * \return     The number of packets queued on the packet queue
 *
 *             This function returns the number of packets that are
 *             queued on the packet queue.
 *
 */
int packetqueue_len(struct packetqueue *q);

/**
 * @}
 */

/**
 * \name Packet queue item functions
 * @{
 */

/**
 * \brief      Access the queuebuf in a packet queue item.
 * \param i    A packet queue item, obtained with packetqueue_first().
 * \return     A pointer to the queuebuf in the packet queue item.
 */
struct queuebuf *packetqueue_queuebuf(struct packetqueue_item *i);
/**
 * \brief      Access the user-defined pointer in a packet queue item.
 * \param i    A packet queue item, obtained with packetqueue_first().
 * \return     A pointer to the user-defined pointer in the packet queue item.
 */

void *packetqueue_ptr(struct packetqueue_item *i);
/**
 * @}
 */


#endif /* PACKETQUEUE_H_ */

/** @} */
/** @} */

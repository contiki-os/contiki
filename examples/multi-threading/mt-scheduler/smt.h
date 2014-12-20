/*
 * Copyright (c) 2014, marcas756@gmail.com.
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
 * Author: marcas756 <marcas756@gmail.com>
 *
 */

/**
 * \file
 *         Scheduled multi threading library
 *
 *         - Non preemtive mt-scheduler (context switch during irq not
 *           allowed for this implementation)
 *         - sending events from a contiki-process to a smt-thread works by
 *           using smt_post in the contiki-process
 *         - sending events from a smt-thread to a smt-thread works by
 *           using smt_post source smt_thread
 *         - sending events from a smt-thread to a contiki-process works by
 *           using process_post/process_post_synch in the smt-thread
 *         - smt-thread can be sent to bed with smt_sleep. This does only block
 *           the smt-thread calling sleep, not the whole system.
 *         - an smt-thread has to exit on its own. Killing the thread from outside
 *           is not possible
 *         - A broadcast of an event to all smt-threads can be achieved by setting the
 *           target thread to SMT_BROADCAST when invoking smt_post.
 *         - Don't forget to call mt_init() and smt_init() before using this lib.
 *         - polling threads from irq or main thread by using smt_poll()
 *
 * \author
 *         marcas756 <marcas756@gmail.com>
 */
#ifndef SMT_H_
#define SMT_H_

#include "contiki.h"
#include "sys/mt.h"
#include "memb.h"



#define SMT_ERR_FULL        PROCESS_ERR_FULL
#define SMT_ERR_OK          PROCESS_ERR_OK
#define SMT_MAX_EVENTS      PROCESS_CONF_NUMEVENTS
#define SMT_BROADCAST       PROCESS_BROADCAST
#define SMT_EVENT_POLL      PROCESS_EVENT_POLL
#define SMT_EVENT_CONTINUE  PROCESS_EVENT_CONTINUE

/*
 * Access macros to smt event data and smt event id
 * In comparison to contiki processes the void pointer provided to mt_threads
 * will be the same as long as the process does not exit. Thus we need access
 * to global event data and event id (see smt.c).
 */
#define SMT_DATA()          smt_data
#define SMT_EVENT()         smt_ev


/** \brief  Extended smt event structure
 *
 *  \details
 *  A contiki event delivered to porcesses consists of an event ID
 *  and a pointer to data associated with the event. Also a pointer
 *  to the process to be serviced with the event has to be stored.
 *
 *  As there is no support for mthread events, the contiki process event
 *  has to be extended by an mthread pointer.
 *
 *  So first the smt process will receive the process event and extract
 *  The target mthread pointer and the event data pointer from process event
 *  data pointer.
 *
 **/
struct smt_event_ext_t {
    mt_thread *target;  /*!< Target thread to send the event to */
    void *data;         /*!< Data associated with the event */
};

extern process_event_t smt_ev;     /*!< Any mthread may access this event id */
extern process_data_t smt_data;    /*!< Any mthread may access this event data */

/**
 * Wait for an event within a mt_thread function
 *
 * Must only be called within a mt_thread function!
 *
 * Waits for any event within the mt_thread function.
 * The only way to reschedule an mt_thread is to send
 * an event to the smt scheduler process. This can be done
 * via smt_post by mt_threads or contiki processes.
 */
#define smt_wait_event() \
    mt_yield()

/**
 * Wait for an event within a mt_thread function until a condition is met
 *
 * Must only be called within a mt_thread function!
 *
 * Waits for any event within the mt_thread function until a condition is met.
 * Is similar to PROCESS_WAIT_EVENT_UNTIL.
 * The only way to reschedule an mt_thread is to send
 * an event to the smt scheduler process. This can be done
 * via smt_post by mt_threads or contiki processes.
 */
#define smt_wait_event_until(cond)  \
    do{                             \
        smt_wait_event();           \
    }while(!(cond))



/**
 * \brief      Initialize the smt library
 * \param c    Briefly describe all parameters.
 *
 * Currently only starts the smt scheduler process. *
 */
void
smt_init(void);

/**
 * \brief      Starts an smt mt_thread
 *
 * \param   thread      Thread control instance
 * \param   function    Thread function
 * \param   data        Pointer to dat to start the thread with
 */
void
smt_start(struct mt_thread *thread, void (* function)(void *), void *data);

/**
 * \brief      Posts an event to an smt mt_thread
 *
 * May be invoked by any other function except irq functions (non preemtive!)
 * Thus a contiki process is able to send an event to a smt m_thread
 *
 * \param   thread      Thread control instance
 * \param   ev          Event id
 * \param   data        Data associated with the event
 */
int
smt_post(struct mt_thread *thread, process_event_t ev, process_data_t data);

/**
 * \brief      Suspends the thread for a given time
 *
 * Must only be called within a mt_thread function!
 *
 * The mt_thread function calling this sleep suspends for interval time.
 * Does not block the rest of the system.
 *
 * \param   interval    Duration (1s=CLOCK_SECOND)
 */
void
smt_sleep(clock_time_t interval);

/**
 * Request a thread to be polled.
 *
 * This function typically is called from an interrupt handler to
 * cause a thread to be polled.
 *
 * \param thread A pointer to the threads mt_thread structure.
 */
void
smt_poll(mt_thread *thread);

/**
 * Yield the thread for a short while.
 *
 * This macro yields the currently running thread for a short while,
 * thus letting other processes and threads run before the thread continues.
 *
 * \hideinitializer
 */
void
smt_pause();

#endif /* SMT_H_ */

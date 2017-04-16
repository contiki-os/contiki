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
 *         Scheduled cooperative multi threading implementation
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
 *         - does not break current mt_thread implementation
 *
 * \author
 *         marcas756 <marcas756@gmail.com>
 */
#ifndef CMT_H_
#define CMT_H_

#include "contiki.h"
#include "sys/mt.h"
#include "memb.h"

#define CMT_ERR_FULL        PROCESS_ERR_FULL
#define CMT_ERR_OK          PROCESS_ERR_OK
#define CMT_MAX_EVENTS      PROCESS_CONF_NUMEVENTS
#define CMT_BROADCAST       PROCESS_BROADCAST
#define CMT_EVENT_POLL      PROCESS_EVENT_POLL
#define CMT_EVENT_CONTINUE  PROCESS_EVENT_CONTINUE

extern struct mt_thread* mt_current(void); /*  access to currently running mt_thread */

#define cmt_current() \
    ((cmt_thread *)mt_current())

#define cmt_exit() \
    mt_exit()

/** \brief  Extended cmt thread structure
 *
 *  This structure extends the mt_thread structure.
 *  This kind of extension makes it possible to keep the current
 *  mt_structure intact without breaking anything in Contiki, but
 *  also makes this implementation less efficient than an optimized mt
 *  implementation that covers cmt (cooperative) and pmt (preemtive).
 *
 **/
typedef struct cmt_thread cmt_thread;
struct cmt_thread {
    struct mt_thread mt_thread; /*!< associated mt_thread ... has to be the first member due to casting */
    cmt_thread *next;           /*!< pointer to the next thread in the list */
    char needspoll;             /*!< threads polling flag (equivalent to processes polling flag) */
    struct ctimer ct;           /*!< sleep timer ... may not be defined as auto var due to stack swapping 6502 arch */
};


/** \brief  Extended cmt event structure
 *
 *  A contiki event delivered to porcesses consists of an event ID
 *  and a pointer to data associated with the event. Also a pointer
 *  to the process to be serviced with the event has to be stored.
 *
 *  As there is no support for mt_thread events, the contiki process event
 *  has to be extended by an mt_thread pointer.
 *
 *  So first the cmt process will receive the process event and extract
 *  The target mt_thread pointer and the event data pointer from process event
 *  data pointer.
 *
 **/
struct cmt_event_ext_t {
    cmt_thread *target;  /*!< Target thread to send the event to */
    void *data;          /*!< Data associated with the event */
};


extern process_event_t cmt_ev;     /*!< Any cmt thread may access this event id */
extern process_data_t  cmt_data;   /*!< Any cmt thread may access this event data */


/**
 * Wait for an event within a mt_thread function
 *
 * Must only be called within a mt_thread function!
 *
 * Waits for any event within the mt_thread function.
 * The only way to reschedule an mt_thread is to send
 * an event to the cmt scheduler process. This can be done
 * via cmt_post by cmt_threads or contiki processes.
 */
#define cmt_wait_event() \
    mt_yield()

/**
 * Wait for an event within a cmt_thread function until a condition is met
 *
 * Must only be called within a mt_thread function!
 *
 * Waits for any event within the cmt_thread function until a condition is met.
 * Is similar to PROCESS_WAIT_EVENT_UNTIL.
 * The only way to reschedule an cmt_thread is to send
 * an event to the cmt scheduler process. This can be done
 * via cmt_post by cmt_threads or contiki processes.
 */
#define cmt_wait_event_until(cond)  \
    do{                             \
        cmt_wait_event();           \
    }while(!(cond))



/**
 * \brief      Initialize the cmt implementation
 *
 * Currently only starts the cmt scheduler process.
 */
void
cmt_init(void);

/**
 * \brief      Starts an cmt mt_thread
 *
 * \param   thread      cmt thread control instance
 * \param   function    Thread function
 * \param   data        Pointer to data to start the thread with
 */
void
cmt_start(struct cmt_thread *thread, void (* function)(void *), void *data);

/**
 * \brief      Posts an event to a cmt thread
 *
 * May be invoked by any other function except irq functions (remeber, we are non preemtive here!)
 * Thus a contiki process is able to send an event to a cmt thread
 *
 * \param   thread      Thread control instance
 * \param   ev          Event id
 * \param   data        Data associated with the event
 */
int
cmt_post(struct cmt_thread *thread, process_event_t ev, process_data_t data);

/**
 * \brief      Suspends the thread for a given time
 *
 * Must only be called within a cmt thread function!
 *
 * The cmt thread function calling this sleep suspends for interval time,
 * but does not block the rest of the system.
 *
 * \param   interval    Duration (1s=CLOCK_SECOND)
 */
void
cmt_sleep(clock_time_t interval);

/**
 * Request a cmt thread to be polled.
 *
 * This function typically is called from an interrupt handler to
 * cause a cmt thread to be polled.
 *
 * \param thread A pointer to the cmt thread to be polled
 */
void
cmt_poll(cmt_thread *thread);

/**
 * Yield the thread for a short while.
 *
 * This macro yields the currently running thread for a short while,
 * thus letting other processes and threads run before the thread continues.
 *
 * \hideinitializer
 */
void
cmt_pause(void);

/**
 * Returns the current event id.
 *
 * \hideinitializer
 */
process_event_t
cmt_get_ev(void);

/**
 * Returns the current event data pointer.
 *
 * \hideinitializer
 */
process_data_t
cmt_get_data(void);



#endif /* SMT_H_ */

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
 *         - In this implementation the cmt thread is derived from contiki process, thus a thread uses
 *           the contiki process scheduler instead of an own scheduler.
 *         - As a cmt instance is also a process instance, process post/post_sync can be used directly
 *           by processes and cmt_threads to post events to each other.
 *         - This example also shows the close logical relationship between protothreads and
 *           usual cooperative threads. Think twice if you really need context switching or not!
 *         - For this special example it would be an advantage if process functions would use a
 *           void pointer for process passing, to avoid explicit casting on application level.
 *         - It would be an advantage to be able to define thread specific stack sizes to avoid
 *           unnecessary RAM usage. Maybe by using cpu min stack size and multiplier? ... to be discussed
 *
 * \author
 *         marcas756 <marcas756@gmail.com>
 */
#ifndef CMT_H_
#define CMT_H_

#include "contiki.h"
#include "sys/mt.h"

#define cmt_current() \
    ((struct cmt_thread*)PROCESS_CURRENT())


/** \brief  Extended cmt process/mt_thread structure
 *
 *  Combines a process with an mt_thread structure.
 *  Process has to be on first position within struct due to casting issues.
 *  (in other words: cmt_thread is derived from process)
 **/
struct cmt_thread {
    struct process process;
    struct mt_thread mt_thread; /*!< associated mt_thread */
    struct etimer et;           /*!< sleep timer ... may not be defined as auto var due to stack swapping 6502 arch */
    void (* function)(void *);  /*!< Thread function pointer (ouch!) */
};

/**
 * \brief     Initializes cooperative multithreading
 *
 *  Optains a startup event id from contiki.
 *  Initializes the mt_thread lib.
 */
void cmt_init();

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
void cmt_sleep(clock_time_t interval);

/**
 * \brief Yield the thread for a short while.
 *
 * Must only be called within a mt_thread function!
 *
 * This macro yields the currently running thread for a short while,
 * thus letting other processes and threads run before the thread continues.
 */
void cmt_pause();

#define cmt_exit() \
    mt_exit()


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
* \brief   Blocks the calling thread until the joined thread terminates.
*          If the calling thread is the joined thread, the function
*          returns immediately.
*
*          Must not be called from a process!
*
* \param   thread      cmt thread control instance
*/
void cmt_join(struct cmt_thread *thread);


/**
* \brief   Blocks the calling process until the joined thread terminates.
*
*          Must not be called from a cmt_thread!
*
* \param   threadptr      cmt thread control instance
*/
#define cmt_process_join(threadptr)  \
        if (process_is_running((struct process *)threadptr)){ \
            PROCESS_WAIT_EVENT_UNTIL( \
                ev == PROCESS_EVENT_EXITED && \
                (threadptr) == data); \
        }




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

#endif /* CMT_H_ */

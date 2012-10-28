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
 */
/*
 * This file is ripped from mt.h of the Contiki Multi-threading library.
 * Fredrik Osterlind <fros@sics.se>
 */
#ifndef __COOJA_MT_H__
#define __COOJA_MT_H__

#include "contiki.h"


/**
 * An opaque structure that is used for holding the state of a thread.
 *
 * The structure should be defined in the "mtarch.h" file. This
 * structure typically holds the entire stack for the thread.
 */
struct cooja_mtarch_thread;

/**
 * Initialize the architecture specific support functions for the
 * multi-thread library.
 *
 * This function is implemented by the architecture specific functions
 * for the multi-thread library and is called by the mt_init()
 * function as part of the initialization of the library. The
 * mtarch_init() function can be used for, e.g., starting preemtion
 * timers or other architecture specific mechanisms required for the
 * operation of the library.
 */
void cooja_mtarch_init(void);

/**
 * Uninstall library and clean up.
 *
 */
void cooja_mtarch_remove(void);

/**
 * Setup the stack frame for a thread that is being started.
 *
 * This function is called by the mt_start() function in order to set
 * up the architecture specific stack of the thread to be started.
 *
 * \param thread A pointer to a struct mtarch_thread for the thread to
 * be started.
 *
 * \param function A pointer to the function that the thread will
 * start executing the first time it is scheduled to run.
 *
 * \param data A pointer to the argument that the function should be
 * passed.
 */
void cooja_mtarch_start(struct cooja_mtarch_thread *thread,
		  void (* function)(void *data),
		  void *data);

/**
 * Yield the processor.
 *
 * This function is called by the mt_yield() function, which is called
 * from the running thread in order to give up the processor.
 *
 */
void cooja_mtarch_yield(void);

/**
 * Start executing a thread.
 *
 * This function is called from mt_exec() and the purpose of the
 * function is to start execution of the thread. The function should
 * switch in the stack of the thread, and does not return until the
 * thread has explicitly yielded (using mt_yield()) or until it is
 * preempted.
 *
 */
void cooja_mtarch_exec(struct cooja_mtarch_thread *thread);


/** @} */


#include "cooja_mtarch.h"

struct cooja_mt_thread {
  int state;
  process_event_t *evptr;
  process_data_t *dataptr;
  struct cooja_mtarch_thread thread;
};

/**
 * No error.
 *
 * \hideinitializer
 */
#define MT_OK 1

/**
 * Initializes the multithreading library.
 *
 */
void cooja_mt_init(void);

/**
 * Uninstalls library and cleans up.
 *
 */
void cooja_mt_remove(void);


/**
 * Starts a multithreading thread.
 *
 * \param thread Pointer to an mt_thread struct that must have been
 * previously allocated by the caller.
 *
 * \param function A pointer to the entry function of the thread that is
 * to be set up.
 *
 * \param data A pointer that will be passed to the entry function.
 *
 */
void cooja_mt_start(struct cooja_mt_thread *thread, void (* function)(void *), void *data);

/**
 * Execute parts of a thread.
 *
 * This function is called by a Contiki process and runs a
 * thread. The function does not return until the thread has yielded,
 * or is preempted.
 *
 * \note The thread must first be initialized with the mt_init() function.
 *
 * \param thread A pointer to a struct mt_thread block that must be
 * allocated by the caller.
 *
 */
void cooja_mt_exec(struct cooja_mt_thread *thread);

/**
 * Post an event to a thread.
 *
 * This function posts an event to a thread. The thread will be
 * scheduled if the thread currently is waiting for the posted event
 * number. If the thread is not waiting for the event, this function
 * does nothing.
 *
 * \note The thread must first be initialized with the mt_init() function.
 *
 * \param thread A pointer to a struct mt_thread block that must be
 * allocated by the caller.
 *
 * \param s The event that is posted to the thread.
 *
 * \param data An opaque pointer to a user specified structure
 * containing additonal information, or NULL if no additional
 * information is needed.
 */
/*void mt_exec_event(struct mt_thread *thread, process_event_t s,
  process_data_t data);*/

/**
 * Voluntarily give up the processor.
 *
 * This function is called by a running thread in order to give up
 * control of the CPU.
 *
 */
void cooja_mt_yield(void);

/**
 * Post an event to another process.
 *
 * This function is called by a running thread and will emit a signal
 * to another Contiki process. This will cause the currently executing
 * thread to yield.
 *
 * \param p The process receiving the signal, or PROCESS_BROADCAST
 * for a broadcast event.
 *
 * \param ev The event to be posted.
 *
 * \param data A pointer to a message that is to be delivered together
 * with the signal.
 *
 */
/*void mt_post(struct process *p, process_event_t ev, process_data_t data);*/

/**
 * Block and wait for an event to occur.
 *
 * This function can be called by a running thread in order to block
 * and wait for an event. The function returns when an event has
 * occured. The event number and the associated data are placed in the
 * variables pointed to by the function arguments.
 *
 * \param ev A pointer to a process_event_t variable. The variable
 * will be filled with the number event that woke the thread.
 *
 * \param data A pointer to a process_data_t variable. The variable
 * will be filled with the data associated with the event that woke
 * the thread.
 *
 */
/*void mt_wait(process_event_t *ev, process_data_t *data);*/

/**
 * Exit a thread.
 *
 * This function is called from within an executing thread in order to
 * exit the thread. The function never returns.
 *
 */
void cooja_mt_exit(void);

/** @} */
/** @} */
#endif /* __MT_H__ */

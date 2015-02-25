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
 *         Example implementation of an contiki event enabled  cooperative mt scheduler
 *
 *         Thread joining tests. This test is only for concept evaluation.
 *
 * \author
 *         marcas756 <marcas756@gmail.com>
 */



#include "contiki.h"
#include <stdint.h>
#include "cmt.h"

#define DEBUG 1
#if DEBUG
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif

#define SIZE 2
const static uint16_t numbers [SIZE] = {7,3} ;
static struct cmt_thread prime_threads [SIZE];
static struct cmt_thread main_thread;


void prime_thread_func (void *data)
{
    uint16_t number,tmp;

    number =*(uint16_t*)data;

    if (number <= 1) {
        goto not_prime;
    }

    for (tmp = 2; tmp*tmp <= number; tmp++)
    {
        if (number%tmp == 0) {
            goto not_prime;
        }

        cmt_pause();
    }

    /* prime: */
    PRINTF("%d : is a prime\n",number);
    cmt_exit();

not_prime:
    PRINTF("%d : is not a prime\n",number);
    cmt_exit();
}


void main_thread_func(void *data)
{
    int tmp;

    for (tmp=0; tmp < SIZE; tmp++)
    {
        PRINTF("Starting prime_thread for %d \n", numbers[tmp]);
        cmt_start(&prime_threads[tmp],prime_thread_func,(void*)&numbers[tmp]);
    }


    for (tmp=0; tmp < SIZE; tmp++)
    {
        PRINTF("Joining prime_thread for %d \n", numbers[tmp]);
        cmt_join(&prime_threads[tmp]);
    }

    PRINTF("Exit main thread\n");
    cmt_exit();
}



/*---------------------------------------------------------------------------*/
PROCESS(cmt_join_test, "cmt_join_test");
AUTOSTART_PROCESSES(&cmt_join_test);
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(cmt_join_test, ev, data)
{
  PROCESS_BEGIN();

  mt_init();

  PRINTF("Starting main thread ... \n");
  cmt_start(&main_thread,main_thread_func,NULL);
  cmt_process_join(&main_thread);

  PRINTF("Exit cmt_join_test \n");

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/


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
 *         Testing mt sleep functionality
 *         Example implementation of an contiki event enabled  mt scheduler
 * \author
 *         marcas756 <marcas756@gmail.com>
 */



#include "contiki.h"
#include "smt.h"
#include <stdint.h>

#define DEBUG 1
#if DEBUG
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif


#define CNTDWN_THREAD_1     0
#define CNTDWN_THREAD_2     1

/*

char is_prime(uint32_t number) {

    uint32_t i;

    if (number <= 1) return 0;

    for (i=2; i*i<=number; i++) {
        if (number % i == 0) return 0;
    }
    return 1;
}

typedef struct {
    uint32_t from;
    uint32_t to;
}prime_thread_data_t ;

void prime_thread(void *data)
{
    mt_thread *this_thread = mt_current();
    PRINTF("%p : Starting\n",this_thread);
}
*/


void cntdwn_thread(void *data)
{
    uint8_t* cntdwn = data;
    mt_thread *this_thread = mt_current();
    PRINTF("%p : Starting\n",this_thread);

    while (*cntdwn)
    {
        PRINTF("%p : cntdwn = %d\n",this_thread, *cntdwn);
        smt_sleep(CLOCK_SECOND);
        (*cntdwn)--;
    }

    PRINTF("%p : Terminating\n",this_thread);

    mt_exit();
}


uint8_t countdown1 = 10;
uint8_t countdown2 = 20;

static mt_thread threads[4];

/*---------------------------------------------------------------------------*/
PROCESS(smt_example, "smt example");
AUTOSTART_PROCESSES(&smt_example);
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(smt_example, ev, data)
{
      static struct etimer et;

      PROCESS_BEGIN();

      mt_init();
      smt_init();

      smt_start(&threads[CNTDWN_THREAD_1],cntdwn_thread,&countdown1);

      /* Delay start of second thread for 0.5s */
      etimer_set(&et,CLOCK_SECOND/2);
      PROCESS_WAIT_EVENT();

      smt_start(&threads[CNTDWN_THREAD_2],cntdwn_thread,&countdown2);


      PROCESS_END();
}
/*---------------------------------------------------------------------------*/


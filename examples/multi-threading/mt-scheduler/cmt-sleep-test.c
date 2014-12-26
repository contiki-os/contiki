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
 *         Example implementation of an contiki event enabled  cooperative mt scheduler
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


#define CNTDWN_THREAD_1     0
#define CNTDWN_THREAD_2     1

void cntdwn_thread(void *data)
{
    uint8_t* cntdwn = data;
    cmt_thread *this_thread = cmt_current();
    cmt_pause();
    PRINTF("%p : Starting\n",this_thread);
    cmt_pause();

    while (*cntdwn)
    {
        cmt_pause();
        PRINTF("%p : cntdwn = %d\n",this_thread, *cntdwn);
        cmt_pause();
        cmt_sleep(CLOCK_SECOND);
        cmt_pause();
        (*cntdwn)--;
        cmt_pause();
        cmt_pause();
        cmt_pause();
        cmt_pause();

    }

    PRINTF("%p : Terminating\n",this_thread);

    cmt_exit();
}


uint8_t countdown1 = 10;
uint8_t countdown2 = 20;

static cmt_thread threads[4];

/*---------------------------------------------------------------------------*/
PROCESS(cmt_example, "cmt example");
AUTOSTART_PROCESSES(&cmt_example);
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(cmt_example, ev, data)
{
      static struct etimer et;

      PROCESS_BEGIN();

      mt_init();
      cmt_init();

      cmt_start(&threads[CNTDWN_THREAD_1],cntdwn_thread,&countdown1);

      /* Delay start of second thread for 0.5s */
      etimer_set(&et,CLOCK_SECOND/2);
      PROCESS_WAIT_EVENT();

      cmt_start(&threads[CNTDWN_THREAD_2],cntdwn_thread,&countdown2);


      PROCESS_END();
}
/*---------------------------------------------------------------------------*/


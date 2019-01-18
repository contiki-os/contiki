/*
 * Copyright (c) 2007, Swedish Institute of Computer Science.
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
 * $Id: energest.c,v 1.6 2008/01/14 16:18:39 thiemovoigt Exp $
 */

/**
 * \file
 *         Implementation of the energy estimation module
 * \author
 *         Adam Dunkels <adam@sics.se>
 */

#include "lib/energest.h"
#include "contiki-conf.h"

#if ENERGEST_CONF_ON

int energest_total_count;
energest_t energest_total_time[ENERGEST_TYPE_MAX];
unsigned short energest_current_time[ENERGEST_TYPE_MAX];
#ifdef ENERGEST_CONF_LEVELDEVICE_LEVELS
energest_t energest_leveldevice_current_leveltime[ENERGEST_CONF_LEVELDEVICE_LEVELS];
#endif

/*---------------------------------------------------------------------------*/
void
energest_init(void)
{
  int i;
  for(i = 0; i < ENERGEST_TYPE_MAX; ++i) {
    energest_total_time[i].current = energest_current_time[i] = 0;
  }
#ifdef ENERGEST_CONF_LEVELDEVICE_LEVELS 
  for(i = 0; i < ENERGEST_CONF_LEVELDEVICE_LEVELS; ++i) {
    energest_leveldevice_current_leveltime[i].current = 0;
  }
#endif
}
/*---------------------------------------------------------------------------*/
unsigned long
energest_type_time(int type)
{
  return energest_total_time[type].current;
}
/*---------------------------------------------------------------------------*/
unsigned long
energest_leveldevice_leveltime(int powerlevel)
{
#ifdef ENERGEST_CONF_LEVELDEVICE_LEVELS
  return energest_leveldevice_current_leveltime[powerlevel].current;
#else
  return 0;
#endif
}
/*---------------------------------------------------------------------------*/
void
energest_type_set(int type, unsigned long val)
{
  energest_total_time[type].current = val;
}
/*---------------------------------------------------------------------------*/
#else /* ENERGEST_CONF_ON */
void energest_type_set(int type, unsigned long val) {}
void energest_init(void) {}
unsigned long energest_type_time(int type) { return 0; }
#endif /* ENERGEST_CONF_ON */

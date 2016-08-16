/*
 * Copyright (c) 2016, George Oikonomou - http://www.spd.gr
 * Copyright (c) 2012-2013, Centre National de la Recherche Scientifique.
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
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 */
/*---------------------------------------------------------------------------*/
/*
 * Author: George Oikonomou
 * Loosely based on the example contributed by Etienne Duble (CNRS / LIG), as
 * part of the work done for the ANR ARESA2 project.
 */
/*---------------------------------------------------------------------------*/
#include "net/mac/mac.h"
#include "net/mac/rdc.h"
#include "net/netstack.h"
#include "sensniff.h"
/*---------------------------------------------------------------------------*/
static void
send(mac_callback_t sent, void *ptr)
{
}
/*---------------------------------------------------------------------------*/
static void
send_list(mac_callback_t sent, void *ptr, struct rdc_buf_list *list)
{
}
/*---------------------------------------------------------------------------*/
static void
input(void)
{
  sensniff_output_frame();
}
/*---------------------------------------------------------------------------*/
static int
on(void)
{
  NETSTACK_RADIO.on();
  return 1;
}
/*---------------------------------------------------------------------------*/
static int
off(int keep_radio_on)
{
  return keep_radio_on;
}
/*---------------------------------------------------------------------------*/
static unsigned short
cca(void)
{
  return 0;
}
/*---------------------------------------------------------------------------*/
static void
init(void)
{
  on();
}
/*---------------------------------------------------------------------------*/
const struct rdc_driver sensniff_rdc_driver = {
  "sensniff-rdc",
  init,
  send,
  send_list,
  input,
  on,
  off,
  cca,
};
/*---------------------------------------------------------------------------*/

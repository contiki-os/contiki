/*
 * Copyright (c) 2013, Institute for Pervasive Computing, ETH Zurich
 * Copyright (c) 2014, Texas Instruments Incorporated - http://www.ti.com/
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
 */
/*---------------------------------------------------------------------------*/
/**
 * \addtogroup cc26xx-web-demo
 * @{
 *
 * \file
 *      CoAP resource to toggle LEDs. Slightly modified copy of the one found
 *      in Contiki's original CoAP example.
 * \author
 *      Matthias Kovatsch <kovatsch@inf.ethz.ch> (original)
 */
/*---------------------------------------------------------------------------*/
#include "contiki.h"
#include "rest-engine.h"
#include "dev/leds.h"

#include <string.h>
/*---------------------------------------------------------------------------*/
static void
res_post_handler_red(void *request, void *response, uint8_t *buffer,
                     uint16_t preferred_size, int32_t *offset)
{
  leds_toggle(LEDS_RED);
}
/*---------------------------------------------------------------------------*/
static void
res_post_handler_green(void *request, void *response, uint8_t *buffer,
                       uint16_t preferred_size, int32_t *offset)
{
  leds_toggle(LEDS_GREEN);
}
/*---------------------------------------------------------------------------*/
/* Toggles the red led */
RESOURCE(res_toggle_red,
         "title=\"Red LED\";rt=\"Control\"",
         NULL,
         res_post_handler_red,
         NULL,
         NULL);

/* Toggles the green led */
RESOURCE(res_toggle_green,
         "title=\"Green LED\";rt=\"Control\"",
         NULL,
         res_post_handler_green,
         NULL,
         NULL);
/*---------------------------------------------------------------------------*/
/* An additional 2 LEDs on the Srf */
#if BOARD_SMARTRF06EB
/*---------------------------------------------------------------------------*/
static void
res_post_handler_yellow(void *request, void *response, uint8_t *buffer,
                        uint16_t preferred_size, int32_t *offset)
{
  leds_toggle(LEDS_YELLOW);
}
/*---------------------------------------------------------------------------*/
static void
res_post_handler_orange(void *request, void *response, uint8_t *buffer,
                        uint16_t preferred_size, int32_t *offset)
{
  leds_toggle(LEDS_ORANGE);
}
/*---------------------------------------------------------------------------*/
/* Toggles the yellow led */
RESOURCE(res_toggle_yellow,
         "title=\"Yellow LED\";rt=\"Control\"",
         NULL,
         res_post_handler_yellow,
         NULL,
         NULL);

/* Toggles the orange led */
RESOURCE(res_toggle_orange,
         "title=\"Orange LED\";rt=\"Control\"",
         NULL,
         res_post_handler_orange,
         NULL,
         NULL);
#endif
/*---------------------------------------------------------------------------*/
/** @} */

/*
 * Copyright (c) 2015, CETIC.
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
 */

/**
 * \file
 *         Simple CoAP Library
 * \author
 *         6LBR Team <6lbr@cetic.be>
 */
#ifndef RD_CLIENT_H_
#define RD_CLIENT_H_

#include "contiki.h"
#include "contiki-net.h"

#ifdef RD_CLIENT_CONF_ENABLED
#define RD_CLIENT_ENABLED RD_CLIENT_CONF_ENABLED
#else
#define RD_CLIENT_ENABLED 1
#endif

#ifdef RD_CLIENT_CONF_LIFETIME
#define RD_CLIENT_LIFETIME RD_CLIENT_CONF_LIFETIME
#else
#define RD_CLIENT_LIFETIME 60
#endif

enum rd_client_status_t
{
  RD_CLIENT_UNCONFIGURED,
  RD_CLIENT_BOOTSTRAPPING,
  RD_CLIENT_REGISTERING,
  RD_CLIENT_REGISTERED,
};

void
rd_client_init(void);

int
rd_client_status(void);

void
rd_client_set_rd_address(uip_ipaddr_t const *new_rd_server_ipaddr, uint16_t port);

void
rd_client_set_resources_list(char const * resources_list);

#endif /* RD_CLIENT_H_ */

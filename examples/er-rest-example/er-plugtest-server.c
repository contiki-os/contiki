/*
 * Copyright (c) 2013, Institute for Pervasive Computing, ETH Zurich
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

/**
 * \file
 *      Server for the ETSI IoT CoAP Plugtests, Las Vegas, NV, USA, Nov 2013.
 * \author
 *      Matthias Kovatsch <kovatsch@inf.ethz.ch>
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "contiki.h"
#include "contiki-net.h"
#include "er-coap.h"
#include "er-coap-transactions.h"
#include "er-coap-separate.h"
#include "rest-engine.h"
#include "er-plugtest.h"

/*
 * Resources to be activated need to be imported through the extern keyword.
 * The build system automatically compiles the resources in the corresponding
 * sub-directory.
 */
extern resource_t
  res_plugtest_test,
  res_plugtest_validate,
  res_plugtest_create1,
  res_plugtest_create2,
  res_plugtest_create3,
  res_plugtest_longpath,
  res_plugtest_query,
  res_plugtest_locquery,
  res_plugtest_multi,
  res_plugtest_link1,
  res_plugtest_link2,
  res_plugtest_link3,
  res_plugtest_path,
  res_plugtest_separate,
  res_plugtest_large,
  res_plugtest_large_update,
  res_plugtest_large_create,
  res_plugtest_obs,
  res_mirror;

PROCESS(plugtest_server, "PlugtestServer");
AUTOSTART_PROCESSES(&plugtest_server);

PROCESS_THREAD(plugtest_server, ev, data)
{
  PROCESS_BEGIN();

  PRINTF("ETSI IoT CoAP Plugtests Server\n");

#ifdef RF_CHANNEL
  PRINTF("RF channel: %u\n", RF_CHANNEL);
#endif
#ifdef IEEE802154_PANID
  PRINTF("PAN ID: 0x%04X\n", IEEE802154_PANID);
#endif

  PRINTF("uIP buffer: %u\n", UIP_BUFSIZE);
  PRINTF("LL header: %u\n", UIP_LLH_LEN);
  PRINTF("IP+UDP header: %u\n", UIP_IPUDPH_LEN);
  PRINTF("REST max chunk: %u\n", REST_MAX_CHUNK_SIZE);

  /* Initialize the REST engine. */
  rest_init_engine();

  /* Activate the application-specific resources. */
  rest_activate_resource(&res_plugtest_test, "test");
  rest_activate_resource(&res_plugtest_validate, "validate");
  rest_activate_resource(&res_plugtest_create1, "create1");
  rest_activate_resource(&res_plugtest_create2, "create2");
  rest_activate_resource(&res_plugtest_create3, "create3");
  rest_activate_resource(&res_plugtest_longpath, "seg1/seg2/seg3");
  rest_activate_resource(&res_plugtest_query, "query");
  rest_activate_resource(&res_plugtest_locquery, "location-query");
  rest_activate_resource(&res_plugtest_multi, "multi-format");
  rest_activate_resource(&res_plugtest_link1, "link1");
  rest_activate_resource(&res_plugtest_link2, "link2");
  rest_activate_resource(&res_plugtest_link3, "link3");
  rest_activate_resource(&res_plugtest_path, "path");
  rest_activate_resource(&res_plugtest_separate, "separate");
  rest_activate_resource(&res_plugtest_large, "large");
  rest_activate_resource(&res_plugtest_large_update, "large-update");
  rest_activate_resource(&res_plugtest_large_create, "large-create");
  rest_activate_resource(&res_plugtest_obs, "obs");

  rest_activate_resource(&res_mirror, "mirror");

  /* Define application-specific events here. */
  while(1) {
    PROCESS_WAIT_EVENT();
  }                             /* while (1) */

  PROCESS_END();
}

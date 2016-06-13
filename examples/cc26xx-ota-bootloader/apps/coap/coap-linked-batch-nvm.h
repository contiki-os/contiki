/*
* Copyright (c) 2014, CETIC.
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

#ifndef COAP_LINKED_BATCH_NVM_H_
#define COAP_LINKED_BATCH_NVM_H_

#include "contiki-conf.h"

#ifdef CORE_ITF_CONF_LINKED_BATCH_NVM
#define CORE_INTERFACE_LINKED_BATCH_NVM CORE_ITF_CONF_LINKED_BATCH_NVM
#else
#define CORE_INTERFACE_LINKED_BATCH_NVM 1
#endif

#ifdef CORE_ITF_CONF_USER_LINKED_BATCH_NB
#define CORE_ITF_USER_LINKED_BATCH_NB CORE_ITF_CONF_USER_LINKED_BATCH_NB
#else
#define CORE_ITF_USER_LINKED_BATCH_NB 2
#endif

#ifdef CORE_ITF_CONF_LINKED_BATCH_MAX_URI_SIZE
#define CORE_ITF_LINKED_BATCH_MAX_URI_SIZE CORE_ITF_CONF_LINKED_BATCH_MAX_URI_SIZE
#else
#define CORE_ITF_LINKED_BATCH_MAX_URI_SIZE 40
#endif

void
linked_batch_table_init(void);

#if CORE_INTERFACE_LINKED_BATCH_NVM
#define CORE_INTERFACE_LINKED_BATCH_NVM_DATA \
	char linked_batch_data[CORE_ITF_USER_LINKED_BATCH_NB][CORE_ITF_LINKED_BATCH_MAX_URI_SIZE]
#define CORE_INTERFACE_LINKED_BATCH_NVM_INIT(nvm_data) linked_batch_table_clear_nvm_links()
#else
#define CORE_INTERFACE_LINKED_BATCH_NVM_DATA
#define CORE_INTERFACE_LINKED_BATCH_NVM_INIT(nvm_data)
#endif

#endif /* COAP_LINKED_BATCH_NVM_H_ */

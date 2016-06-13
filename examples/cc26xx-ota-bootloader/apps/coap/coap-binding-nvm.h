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

#ifndef COAP_BINDING_NVM_H
#define COAP_BINDING_NVM_H

#include "contiki-conf.h"

#ifdef CORE_ITF_CONF_BINDING_TABLE_NVM
#define CORE_INTERFACE_BINDING_TABLE_NVM CORE_ITF_CONF_BINDING_TABLE_NVM
#else
#define CORE_INTERFACE_BINDING_TABLE_NVM 1
#endif

#ifdef COAP_BINDING_CONF_MAX_URI_SIZE
#define COAP_BINDING_MAX_URI_SIZE COAP_BINDING_CONF_MAX_URI_SIZE
#else
#define COAP_BINDING_MAX_URI_SIZE 40
#endif

#ifdef CORE_ITF_CONF_USER_BINDING_NB
#define CORE_ITF_USER_BINDING_NB CORE_ITF_CONF_USER_BINDING_NB
#else
#define CORE_ITF_USER_BINDING_NB 2
#endif

/*---------------------------------------------------------------------------*/

#if CORE_INTERFACE_BINDING_TABLE_NVM
#define CORE_INTERFACE_BINDING_TABLE_NVM_DATA \
	nvm_binding_data_t binding_data[CORE_ITF_USER_BINDING_NB]
#define CORE_INTERFACE_BINDING_TABLE_NVM_INIT(nvm_data) resource_binding_clear_nvm_bindings()
#else
#define CORE_INTERFACE_BINDING_TABLE_NVM_DATA
#define CORE_INTERFACE_BINDING_TABLE_NVM_INIT(nvm_data)
#endif

/*---------------------------------------------------------------------------*/

typedef struct {
	uint8_t dest_addr[16];
	uint16_t dest_port;
	char uri[COAP_BINDING_MAX_URI_SIZE];
	char resource[COAP_BINDING_MAX_URI_SIZE];
	int flags;
	int pmin;
	int pmax;
	int step;
	int less_than;
	int greater_than;
} nvm_binding_data_t;

#endif

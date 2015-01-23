/*
 * Copyright (c) 2015, 3B Scientific GmbH.
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
 */

/**
 * \addtogroup cc32xx
 * @{
 *
 * \defgroup cc32xx-wifi cc32xx wireless network wrapper
 *
 * Wrapper for the cc32xx wireless network controller
 * @{
 *
 * \file
 * Headers of the cc32xx wireless network wrapper
 */

#ifndef WIFI_H_
#define WIFI_H_

// Status bits - These are used to set/reset the corresponding bits in
// given variable
typedef enum {
    STATUS_BIT_NWP_INIT = 0, // If this bit is set: Network Processor is
                             // powered up

    STATUS_BIT_CONNECTION,   // If this bit is set: the device is connected to
                             // the AP or client is connected to device (AP)

    STATUS_BIT_IP_LEASED,    // If this bit is set: the device has leased IP to
                             // any connected client

    STATUS_BIT_IP_AQUIRED,   // If this bit is set: the device has acquired an IP

} wifi_statusbits_t;

#define CLR_STATUS_BIT_ALL(status_variable)  	(status_variable = 0)
#define SET_STATUS_BIT(status_variable, bit) 	status_variable |= (1<<(bit))
#define CLR_STATUS_BIT(status_variable, bit) 	status_variable &= ~(1<<(bit))
#define CLR_STATUS_BIT_ALL(status_variable)   	(status_variable = 0)
#define GET_STATUS_BIT(status_variable, bit) 	(0 != (status_variable & (1<<(bit))))

#define IS_NW_PROCSR_ON(status_variable)     	GET_STATUS_BIT(status_variable,\
													STATUS_BIT_NWP_INIT)
#define IS_CONNECTED(status_variable)        	GET_STATUS_BIT(status_variable,\
													STATUS_BIT_CONNECTION)
#define IS_IP_LEASED(status_variable)        	GET_STATUS_BIT(status_variable,\
											    	STATUS_BIT_IP_LEASED)
#define IS_IP_ACQUIRED(status_variable)       	GET_STATUS_BIT(status_variable,\
                                                	STATUS_BIT_IP_AQUIRED)

void wifi_init(void);
uint16_t wifi_poll(void);
void wifi_send(void);
void wifi_exit(void);

#endif /* WIFI_H_ */

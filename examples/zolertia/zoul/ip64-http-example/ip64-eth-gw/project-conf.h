/*
 * Copyright (c) 2016, Zolertia - http://www.zolertia.com
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
 *
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
/**
 * \addtogroup zoul-examples
 * @{
 *
 * \defgroup eth-gw-router Zolertia eth-gw border router
 *
 * Implements a simple IP64 router with a webserver
 *
 * @{
 *
 * \file
 *  Configuration file for the eth-gw IP64 router
 *
 * \author
 *         Antonio Lignan <alinan@zolertia.com>
 */
/*---------------------------------------------------------------------------*/
#ifndef PROJECT_CONF_H_
#define PROJECT_CONF_H_

/* Prevent the router from dropping below LPM2 to avoid RAM overflow */
#define LPM_CONF_MAX_PM             0

/* Use either the cc1200_driver for sub-1GHz, or cc2538_rf_driver (default)
 * for 2.4GHz built-in radio interface
 */
#undef  NETSTACK_CONF_RADIO
#define NETSTACK_CONF_RADIO         cc2538_rf_driver

/* Disable RDC */
#define NETSTACK_CONF_RDC           nullrdc_driver

/* USe DHCP */
#define IP64_CONF_DHCP              1

/* Webserver settings */
#define WEBSERVER_CONF_ROUTE_LINKS  0
#define UIP_CONF_RECEIVE_WINDOW     128
#define WEBSERVER_CONF_CFS_CONNS    2
#define UIP_CONF_BUFFER_SIZE        900
#define UIP_CONF_TCP_MSS            128

#endif /* PROJECT_CONF_H_ */
/*---------------------------------------------------------------------------*/
/**
 * @}
 * @}
 */

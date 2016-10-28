/*
 * Copyright (c) 2016, relayr http://relayr.io/
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
 * \defgroup ota-update-bootloader Simple OTA update bootloader
 *
 * @{
 * \file
 * Project specific configuration for CoAP OTA firmware update bootloader
 */
/*---------------------------------------------------------------------------*/
#ifndef PROJECT_CONF_H_
#define PROJECT_CONF_H_
/*---------------------------------------------------------------------------*/
/* User configuration */
#define STATUS_LED                    LEDS_RED

/* Disabling TCP on CoAP nodes. */
#undef UIP_CONF_TCP
#define UIP_CONF_TCP                   0

/*---------------------------------------------------------------------------*/
/* Select the minimum low power mode the node should drop to */
#define LPM_CONF_MAX_PM               0

/* Use either the cc1200_driver for sub-1GHz, or cc2538_rf_driver (default)
 * for 2.4GHz built-in radio interface
 */
#undef  NETSTACK_CONF_RADIO
#define NETSTACK_CONF_RADIO           cc2538_rf_driver

/* Alternate between ANTENNA_SW_SELECT_SUBGHZ or ANTENNA_SW_SELECT_2_4GHZ */
#define ANTENNA_SW_SELECT_DEF_CONF    ANTENNA_SW_SELECT_2_4GHZ

#define NETSTACK_CONF_RDC             nullrdc_driver

/* Filtering .well-known/core per query can be disabled to save space. */
#undef COAP_LINK_FORMAT_FILTERING
#define COAP_LINK_FORMAT_FILTERING     0
#undef COAP_PROXY_OPTION_PROCESSING
#define COAP_PROXY_OPTION_PROCESSING   0

#define FLASH_CONF_FW_ADDR             (COFFEE_START + COFFEE_SIZE)
/* disable watchdog to alow flash erase/programing */
#define WATCHDOG_CONF_ENABLE           0

#endif /* PROJECT_CONF_H_ */
/*---------------------------------------------------------------------------*/
/**
 * @}
 * @}
 */

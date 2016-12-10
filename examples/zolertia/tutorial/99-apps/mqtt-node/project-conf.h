/*
 * Copyright (c) 2012, Texas Instruments Incorporated - http://www.ti.com/
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
 * \addtogroup cc2538-mqtt-demo
 * @{
 *
 * \file
 * Project specific configuration defines for the MQTT demo
 */
/*---------------------------------------------------------------------------*/
#ifndef PROJECT_CONF_H_
#define PROJECT_CONF_H_
/*---------------------------------------------------------------------------*/
/* User configuration */
#ifndef DEVICE_ID
#define DEVICE_ID                     "mqtt"
#endif

#define STATUS_LED                    LEDS_GREEN

/* This is the base-time unit, if using a DEFAULT_SAMPLING_INTERVAL of 1 second
 * (given by the CLOCK_SECOND macro) the node will periodically publish every
 * DEFAULT_PUBLISH_INTERVAL seconds.
 */
#define DEFAULT_PUBLISH_INTERVAL      45
#define DEFAULT_SAMPLING_INTERVAL     CLOCK_SECOND

/* Minimum and maximum update rate values */
#define DEFAULT_UPDATE_PERIOD_MIN     5
#define DEFAULT_UPDATE_PERIOD_MAX     600
/*---------------------------------------------------------------------------*/
/* Select the minimum low power mode the node should drop to */
#define LPM_CONF_MAX_PM               1

/* In case we need to change the default 6LoWPAN prefix context */
#define UIP_CONF_DS6_DEFAULT_PREFIX   0xaaaa

/* Use either the cc1200_driver for sub-1GHz, or cc2538_rf_driver (default)
 * for 2.4GHz built-in radio interface
 */
#undef  NETSTACK_CONF_RADIO
#define NETSTACK_CONF_RADIO           cc2538_rf_driver

/* Alternate between ANTENNA_SW_SELECT_SUBGHZ or ANTENNA_SW_SELECT_2_4GHZ */
#define ANTENNA_SW_SELECT_DEF_CONF    ANTENNA_SW_SELECT_2_4GHZ

/* Use nullrdc_driver to disable RDC, or contikimac_driver to enable */
#define NETSTACK_CONF_RDC             nullrdc_driver

#define COFFEE_MAGIC_WORD             0xABCD
#define COFFEE_CONF_LOG_TABLE_LIMIT   16
#define COFFEE_CONF_LOG_SIZE          256
#define COFFEE_CONF_MICRO_LOGS        1
#define COFFEE_CONF_APPEND_ONLY       0

/* Security related configuration */
#if WITH_LLSEC_ENABLED
#undef LLSEC802154_CONF_ENABLED
#define LLSEC802154_CONF_ENABLED      1

#undef LLSEC802154_CONF_SECURITY
#define LLSEC802154_CONF_SECURITY     1

#define NONCORESEC_CONF_SEC_LVL       7

#undef NETSTACK_CONF_FRAMER
#define NETSTACK_CONF_FRAMER          noncoresec_framer
#undef NETSTACK_CONF_LLSEC
#define NETSTACK_CONF_LLSEC           noncoresec_driver

#undef AES_128_CONF
#define AES_128_CONF                  aes_128_driver

#define LLSEC_ANTIREPLAY_ENABLED            0
#define LLSEC_REBOOT_WORKAROUND_ENABLED     1

#define NONCORESEC_CONF_KEY { 0x00 , 0x01 , 0x02 , 0x03 , \
                              0x04 , 0x05 , 0x06 , 0x07 , \
                              0x08 , 0x09 , 0x0A , 0x0B , \
                              0x0C , 0x0D , 0x0E , 0x0F }
#else
#undef NETSTACK_CONF_LLSEC
#define NETSTACK_CONF_LLSEC nullsec_driver
#endif

/* Size optimizations */
/* Disabling printf saves 16 bytes */
#define DISABLE_PRINTF                      0
#if DISABLE_PRINTF
#define UART_CONF_ENABLE                    0
#define STARTUP_CONF_VERBOSE                0
#define DEBUG_PLATFORM                      0
#define DEBUG_APP                           0
#define DEBUG_CLIENT                        0
#else /* DISABLE_PRINTF */
#define UART_CONF_ENABLE                    1
#define STARTUP_CONF_VERBOSE                1
#define DEBUG_PLATFORM                      0
#define DEBUG_APP                           0
#define DEBUG_CLIENT                        1
#endif /* DISABLE_PRINTF */

#define APP_BUFFER_SIZE                     512
#define NBR_TABLE_CONF_MAX_NEIGHBORS        10
#define UIP_CONF_MAX_ROUTES                 10

#endif /* PROJECT_CONF_H_ */
/*---------------------------------------------------------------------------*/
/** @} */

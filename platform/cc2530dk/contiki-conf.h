#ifndef __CONTIKI_CONF_H__
#define __CONTIKI_CONF_H__

#include "8051def.h"
#include "sys/cc.h"
#include <string.h>

/* Include Project Specific conf */
#ifdef PROJECT_CONF_H
#include "project-conf.h"
#endif /* PROJECT_CONF_H */

#include "models.h"

/*
 * Define this as 1 to poll the etimer process from within main instead of from
 * the clock ISR. This reduces the ISR's stack usage and may prevent crashes.
 */
#ifndef CLOCK_CONF_STACK_FRIENDLY
#define CLOCK_CONF_STACK_FRIENDLY 1
#endif

#ifndef STACK_CONF_DEBUGGING
#define STACK_CONF_DEBUGGING  0
#endif

/* Energest Module */
#ifndef ENERGEST_CONF_ON
#define ENERGEST_CONF_ON      0
#endif

/* Verbose Startup? Turning this off saves plenty of bytes of CODE in HOME */
#ifndef STARTUP_CONF_VERBOSE
#define STARTUP_CONF_VERBOSE  0
#endif

/* More CODE space savings by turning off process names */
#define PROCESS_CONF_NO_PROCESS_NAMES 1

/*
 * USARTs:
 *   SmartRF RS232 -> USART0 / Alternative 1 (UART)
 *   SmartRF LCD   -> USART1 / Alternative 2 (SPI)
 */
#define UART_ON_USART     0

#define UART1_CONF_ENABLE 0

#ifndef UART0_CONF_ENABLE
#define UART0_CONF_ENABLE  1
#endif
#ifndef UART0_CONF_WITH_INPUT
#define UART0_CONF_WITH_INPUT 0
#endif

#ifndef UART0_CONF_HIGH_SPEED
#define UART0_CONF_HIGH_SPEED 0
#endif

/* USB output buffering enabled by default (relevant to cc2531 builds only) */
#ifndef USB_SERIAL_CONF_BUFFERED
#define USB_SERIAL_CONF_BUFFERED 1
#endif

/* Are we a SLIP bridge? */
#if SLIP_ARCH_CONF_ENABLE
/* Make sure the UART is enabled, with interrupts */
#undef UART0_CONF_ENABLE
#undef UART0_CONF_WITH_INPUT
#define UART0_CONF_ENABLE  1
#define UART0_CONF_WITH_INPUT 1
#define UIP_FALLBACK_INTERFACE slip_interface
#endif

/* Output all captured frames over the UART in hexdump format */
#ifndef CC2530_RF_CONF_HEXDUMP
#define CC2530_RF_CONF_HEXDUMP 0
#endif

#if CC2530_RF_CONF_HEXDUMP
/* We need UART1 output */
#undef UART_ZERO_CONF_ENABLE
#define UART_ZERO_CONF_ENABLE   1
#endif

/* Code Shortcuts */
/*
 * When set, this directive also configures the following bypasses:
 *   - process_post_synch() in tcpip_input() (we call packet_input())
 *   - process_post_synch() in tcpip_uipcall (we call the relevant pthread)
 *   - mac_call_sent_callback() is replaced with sent() in various places
 *
 * These are good things to do, they reduce stack usage and prevent crashes
 */
#define NETSTACK_CONF_SHORTCUTS   1

/*
 * By default we read our MAC from the (read-only) Information Page (primary
 * location). In order to have a user-programmable mac, define this as 0 to
 * use the secondary location (addresses 0xFFE8 - 0xFFEF on the last flash page)
 */
#ifndef CC2530_CONF_MAC_FROM_PRIMARY
#define CC2530_CONF_MAC_FROM_PRIMARY 1
#endif

/* Interrupt Number 6: Shared between P2 Inputs, I2C and USB
 * A single ISR handles all of the above. Leave this as is if you are not
 * interested in any of the above. Define as 1 (e.g. in project-conf.h) if
 * at least one of those interrupt sources will need handled */
#ifndef PORT_2_ISR_ENABLED
#define PORT_2_ISR_ENABLED 0
#endif

/*
 * Sensors
 * It is harmless to #define XYZ 1
 * even if the sensor is not present on our device
 */
#ifndef BUTTON_SENSOR_CONF_ON
#define BUTTON_SENSOR_CONF_ON   1  /* Buttons */
#endif

/* B2 on the cc2531 USB stick can be a reset button or a general-purpose one */
#ifndef CC2531_CONF_B2_REBOOTS
#define CC2531_CONF_B2_REBOOTS        0  /* General Purpose by default */
#endif

/* ADC - Turning this off will disable everything below */
#ifndef ADC_SENSOR_CONF_ON
#define ADC_SENSOR_CONF_ON      1
#endif
#define TEMP_SENSOR_CONF_ON     1  /* Temperature */
#define VDD_SENSOR_CONF_ON      1  /* Supply Voltage */
#define BATTERY_SENSOR_CONF_ON  0  /* Battery */

/* Low Power Modes - We only support PM0/Idle and PM1 */
#ifndef LPM_CONF_MODE
#define LPM_CONF_MODE         0 /* 0: no LPM, 1: MCU IDLE, 2: Drop to PM1 */
#endif

/* DMA Configuration */
#ifndef DMA_CONF_ON
#define DMA_CONF_ON 0
#endif

/* Viztool on by default for IPv6 builds */
#if UIP_CONF_IPV6
#ifndef VIZTOOL_CONF_ON
#define VIZTOOL_CONF_ON        1
#endif /* VIZTOOL_CONF_ON */
#endif /* UIP_CONF_IPV6 */

/* Network Stack */
#if UIP_CONF_IPV6
#define NETSTACK_CONF_NETWORK sicslowpan_driver
#else
#define NETSTACK_CONF_NETWORK rime_driver
#endif

#ifndef NETSTACK_CONF_MAC
#define NETSTACK_CONF_MAC     csma_driver
#endif

#ifndef NETSTACK_CONF_RDC
#define NETSTACK_CONF_RDC     nullrdc_driver
#define NULLRDC_802154_AUTOACK 1
#define NULLRDC_802154_AUTOACK_HW 1
#endif

#ifndef NETSTACK_CONF_RDC_CHANNEL_CHECK_RATE
#define NETSTACK_CONF_RDC_CHANNEL_CHECK_RATE 8
#endif

#define NETSTACK_CONF_FRAMER  framer_802154
#define NETSTACK_CONF_RADIO   cc2530_rf_driver

/* RF Config */
#define IEEE802154_CONF_PANID 0x5449 /* TI */

#ifndef CC2530_RF_CONF_CHANNEL
#define CC2530_RF_CONF_CHANNEL    25
#endif /* CC2530_RF_CONF_CHANNEL */

#ifndef CC2530_RF_CONF_AUTOACK
#define CC2530_RF_CONF_AUTOACK 1
#endif /* CC2530_CONF_AUTOACK */

#if UIP_CONF_IPV6
/* Addresses, Sizes and Interfaces */
/* 8-byte addresses here, 2 otherwise */
#define RIMEADDR_CONF_SIZE                   8
#define UIP_CONF_LL_802154                   1
#define UIP_CONF_LLH_LEN                     0
#define UIP_CONF_NETIF_MAX_ADDRESSES         3

/* TCP, UDP, ICMP */
#define UIP_CONF_TCP                         0
#define UIP_CONF_UDP                         1
#define UIP_CONF_UDP_CHECKSUMS               1

/* ND and Routing */
#ifndef UIP_CONF_ROUTER
#define UIP_CONF_ROUTER                      1
#endif

/* Prevent SDCC compile error when UIP_CONF_ROUTER == 0 */
#if !UIP_CONF_ROUTER
#define UIP_CONF_DS6_AADDR_NBU               1
#endif

#define UIP_CONF_ND6_SEND_RA                 0
#define UIP_CONF_IP_FORWARD                  0
#define RPL_CONF_STATS                       0
#define RPL_CONF_MAX_DAG_ENTRIES             1
#ifndef RPL_CONF_OF
#define RPL_CONF_OF rpl_of_etx
#endif

#define UIP_CONF_ND6_REACHABLE_TIME     600000
#define UIP_CONF_ND6_RETRANS_TIMER       10000

#ifndef UIP_CONF_DS6_NBR_NBU
#define UIP_CONF_DS6_NBR_NBU                 4 /* Handle n Neighbors */
#endif
#ifndef UIP_CONF_DS6_ROUTE_NBU
#define UIP_CONF_DS6_ROUTE_NBU               4 /* Handle n Routes */
#endif

/* uIP */
#define UIP_CONF_BUFFER_SIZE               240
#define UIP_CONF_IPV6_QUEUE_PKT              0
#define UIP_CONF_IPV6_CHECKS                 1
#define UIP_CONF_IPV6_REASSEMBLY             0

/* 6lowpan */
#define SICSLOWPAN_CONF_COMPRESSION          SICSLOWPAN_COMPRESSION_HC06
#ifndef SICSLOWPAN_CONF_FRAG
#define SICSLOWPAN_CONF_FRAG                 0 /* About 2KB of CODE if 1 */
#endif
#define SICSLOWPAN_CONF_MAXAGE               8

/* Define our IPv6 prefixes/contexts here */
#define SICSLOWPAN_CONF_MAX_ADDR_CONTEXTS    1
#define SICSLOWPAN_CONF_ADDR_CONTEXT_0 { \
  addr_contexts[0].prefix[0] = 0xaa; \
  addr_contexts[0].prefix[1] = 0xaa; \
}

#define MAC_CONF_CHANNEL_CHECK_RATE          8
#define QUEUEBUF_CONF_NUM                    6

#else /* UIP_CONF_IPV6 */
/* Network setup for non-IPv6 (rime). */
#define UIP_CONF_IP_FORWARD                  1
#define UIP_CONF_BUFFER_SIZE               108
#define RIME_CONF_NO_POLITE_ANNOUCEMENTS     0
#define QUEUEBUF_CONF_NUM                    8
#endif /* UIP_CONF_IPV6 */

#endif /* __CONTIKI_CONF_H__ */

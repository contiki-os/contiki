#ifndef __CONTIKI_CONF_H__
#define __CONTIKI_CONF_H__

#include "8051def.h"
#include "sys/cc.h"
#include <string.h>

/* Include Project Specific conf */
#ifdef PROJECT_CONF_H
#include "project-conf.h"
#endif /* PROJECT_CONF_H */

/*
 * Define this as 1 to poll the etimer process from within main instead of from
 * the clock ISR. This reduces the ISR's stack usage and may prevent crashes.
 */
#ifndef CLOCK_CONF_STACK_FRIENDLY
#define CLOCK_CONF_STACK_FRIENDLY 1
#endif

/* Memory filesystem RAM size. */
#define CFS_RAM_CONF_SIZE		512

/* Logging.. */
#define LOG_CONF_ENABLED		0

#ifndef STACK_CONF_DEBUGGING
#define STACK_CONF_DEBUGGING  0
#endif

/* Energest Module */
#ifndef ENERGEST_CONF_ON
#define ENERGEST_CONF_ON      0
#endif

/* Verbose Startup? Turning this off reduces our footprint a fair bit */
#define STARTUP_CONF_VERBOSE  0

/* More CODE space savings by turning off process names */
#define PROCESS_CONF_NO_PROCESS_NAMES 1

/*
 * UARTs: 1=>Enabled, 0=>Disabled. Default: Both Disabled (see uart.h)
 * Disabling UARTs reduces our CODE footprint
 * Disabling UART1 also disables all debugging output.
 * Should be used when nodes are meant to run on batteries
 *
 * On N740, by enabling UART1, you are also enabling an ugly hack which aims
 * to detect the USB connection during execution. It will then turn on/off
 * UART1 RX interrupts accordingly. This seems to work but you have been warned
 * If you start seeing random crashes when on battery, this is where to look.
 */
#ifndef UART_ONE_CONF_ENABLE
#define UART_ONE_CONF_ENABLE  1
#endif
#ifndef UART_ONE_CONF_WITH_INPUT
#define UART_ONE_CONF_WITH_INPUT 0
#endif
#define UART_ZERO_CONF_ENABLE 0

#ifndef UART_ONE_CONF_HIGH_SPEED
#define UART_ONE_CONF_HIGH_SPEED 0
#endif

#define SLIP_RADIO_CONF_NO_PUTCHAR 1

#if defined (UIP_FALLBACK_INTERFACE) || defined (CMD_CONF_OUTPUT)
#define SLIP_ARCH_CONF_ENABLE      1
#endif

/* Are we a SLIP bridge? */
#if SLIP_ARCH_CONF_ENABLE
/* Make sure UART1 is enabled, with interrupts */
#undef UART_ONE_CONF_ENABLE
#undef UART_ONE_CONF_WITH_INPUT
#define UART_ONE_CONF_ENABLE  1
#define UART_ONE_CONF_WITH_INPUT 1
#endif

/* Output all captured frames over the UART in hexdump format */
#ifndef CC2430_RF_CONF_HEXDUMP
#define CC2430_RF_CONF_HEXDUMP 0
#endif

#if CC2430_RF_CONF_HEXDUMP
/* We need UART1 output */
#undef UART_ONE_CONF_ENABLE
#define UART_ONE_CONF_ENABLE   1
#endif

/* Code Shortcuts */
/*
 * When set, the RF driver is no longer a contiki process and the RX ISR is
 * disabled. Instead of polling the radio process when data arrives, we
 * periodically check for data by directly invoking the driver from main()
 *
 * When set, this directive also configures the following bypasses:
 *   - process_post_synch() in tcpip_input() (we call packet_input())
 *   - process_post_synch() in tcpip_uipcall (we call the relevant pthread)
 *   - mac_call_sent_callback() is replaced with sent() in various places
 *
 * These are good things to do, they reduce stack usage and prevent crashes
 */
#define NETSTACK_CONF_SHORTCUTS   1

/*
 * Sensors
 * It is harmless to #define XYZ 1
 * even if the sensor is not present on our device
 */
#ifndef BUTTON_SENSOR_CONF_ON
#define BUTTON_SENSOR_CONF_ON   1  /* Buttons */
#endif
/* ADC - Turning this off will disable everything below */
#ifndef ADC_SENSOR_CONF_ON
#define ADC_SENSOR_CONF_ON      1
#endif
#define TEMP_SENSOR_CONF_ON     1  /* Temperature */
#define BATTERY_SENSOR_CONF_ON  1  /* Battery */
#define VDD_SENSOR_CONF_ON      1  /* Supply Voltage */
#define ACC_SENSOR_CONF_ON      1  /* Accelerometer */
#define ACC_SENSOR_CONF_GSEL    0  /* Acc. g-Select => 1: +/-11g, 0: +/-3g */
#define LIGHT_SENSOR_CONF_ON    1  /* Light */

/* Watchdog */
#define WDT_CONF_INTERVAL     0
#define WDT_CONF_TIMER_MODE   0 /* 0 or undefined for watchdog mode */

/* Low Power Modes - We only support PM0/Idle and PM1 */
#ifndef LPM_CONF_MODE
#define LPM_CONF_MODE         1 /* 0: no LPM, 1: MCU IDLE, 2: Drop to PM1 */
#endif

/* DMA Configuration */
#ifndef DMA_CONF_ON
#define DMA_CONF_ON 0
#endif

/* N740 Serial Flash */
#ifndef M25P16_CONF_ON
#define M25P16_CONF_ON  1
#endif

/* XXX argh, ugly hack to make stuff compile! */
#define snprintf(BUF, SIZE, ...) sprintf(BUF, __VA_ARGS__)

/* Sensinode-Specific Tools and APPs */
/* Viztool on by default for IPv6 builds */
#if UIP_CONF_IPV6
#ifndef VIZTOOL_CONF_ON
#define VIZTOOL_CONF_ON        1
#endif /* VIZTOOL_CONF_ON */
#endif /* UIP_CONF_IPV6 */

/* BatMon off by default unless we build with APPS += batmon */
#ifndef BATMON_CONF_ON
#define BATMON_CONF_ON         0
#endif

/* Network Stack */
#ifndef NETSTACK_CONF_NETWORK
#if UIP_CONF_IPV6
#define NETSTACK_CONF_NETWORK sicslowpan_driver
#else
#define NETSTACK_CONF_NETWORK rime_driver
#endif /* UIP_CONF_IPV6 */
#endif /* NETSTACK_CONF_NETWORK */

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

#ifndef NETSTACK_CONF_FRAMER
#define NETSTACK_CONF_FRAMER  framer_802154
#endif

#define NETSTACK_CONF_RADIO   cc2430_rf_driver

/* RF Config */
#define IEEE802154_CONF_PANID 0x4C55 /* LU */

#ifndef CC2430_RF_CONF_CHANNEL
#define CC2430_RF_CONF_CHANNEL    25
#endif /* CC2430_RF_CONF_CHANNEL */

#ifndef CC2430_RF_CONF_TX_POWER
#define CC2430_RF_CONF_TX_POWER 0x5F /* Datasheet recommended value */
#endif

#ifndef CC2430_RF_CONF_AUTOACK
#define CC2430_RF_CONF_AUTOACK 1
#endif /* CC2430_CONF_AUTOACK */

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

#define UIP_CONF_ND6_SEND_RA                 0
#define UIP_CONF_IP_FORWARD                  0
#define RPL_CONF_STATS                       0
#define RPL_CONF_MAX_DAG_ENTRIES             1
#ifndef RPL_CONF_OF
#define RPL_CONF_OF rpl_mrhof
#endif

#define UIP_CONF_ND6_REACHABLE_TIME     600000
#define UIP_CONF_ND6_RETRANS_TIMER       10000

#ifndef NBR_TABLE_CONF_MAX_NEIGHBORS
#define NBR_TABLE_CONF_MAX_NEIGHBORS                 4 /* Handle n Neighbors */
#endif
#ifndef UIP_CONF_MAX_ROUTES
#define UIP_CONF_MAX_ROUTES               4 /* Handle n Routes */
#endif

/* uIP */
#ifndef UIP_CONF_BUFFER_SIZE
#define UIP_CONF_BUFFER_SIZE               240
#endif
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
  addr_contexts[0].prefix[0] = 0x20; \
  addr_contexts[0].prefix[1] = 0x01; \
  addr_contexts[0].prefix[2] = 0x06; \
  addr_contexts[0].prefix[3] = 0x30; \
  addr_contexts[0].prefix[4] = 0x03; \
  addr_contexts[0].prefix[5] = 0x01; \
  addr_contexts[0].prefix[6] = 0x64; \
  addr_contexts[0].prefix[7] = 0x53; \
}

#define MAC_CONF_CHANNEL_CHECK_RATE          8
#ifndef QUEUEBUF_CONF_NUM
#define QUEUEBUF_CONF_NUM                    6
#endif

#else /* UIP_CONF_IPV6 */
/* Network setup for non-IPv6 (rime). */
#define UIP_CONF_IP_FORWARD                  1
#define UIP_CONF_BUFFER_SIZE               108
#define RIME_CONF_NO_POLITE_ANNOUCEMENTS     0
#define QUEUEBUF_CONF_NUM                    8
#endif /* UIP_CONF_IPV6 */

/* Prevent SDCC compile error when UIP_CONF_ROUTER == 0 */
#if !UIP_CONF_ROUTER
#define UIP_CONF_DS6_AADDR_NBU               1
#endif

#endif /* __CONTIKI_CONF_H__ */

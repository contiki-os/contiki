/*
 * Copyright (c) 2013, Loughborough University.
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
 * 3. Neither the name of the University nor the names of its contributors
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
 * \addtogroup cc2538-examples
 * @{
 *
 * @{
 *
 * \defgroup cc2538-spi cc2538dk SPI Test Project
 *
 *    This example tests the correct functionality of SPI interface in both Master
 *    and Slave mode.
 *
 *    More specifically, it is a very basic example testing
 *     SPI in master and slave modes as well as
 *    their ISR functionality
 *
 *    This example will not work as it is if multiple SPI devices are connected at the same time.
 *    its ment to work with 2 devices master and slave
 *
 * @{
 *
 * \file
 *         Tests related to SPI interface
 *
 *
 * \author
 *         Vasilis Michopoulos <basilismicho@gmail.com>
 */

#include "contiki.h"
#include "contiki-lib.h"
#include "contiki-net.h"
#include "sys/clock.h"
#include "sys/rtimer.h"
#include "dev/leds.h"
#include "spi-arch.h"
#include "dev/spi.h"
#include "dev/gpio.h"

#include <string.h>
/*---------------------------------------------------------------------------*/
#define SIZE 30
#define FIFO 4		// This can either be set to 4(half FIFO interrupt trigger or fully pre-load FIFO by setting this to 8)
#define DEBUG DEBUG_PRINT
#include "net/ip/uip-debug.h"
#include "dev/watchdog.h"
#include "net/rpl/rpl.h"
/*---------------------------------------------------------------------------*/
#if SPI_FW
#define MAX_PAYLOAD_LEN 120
#define UDP_SERVER_PORT 60000
#define UDP_CLIENT_PORT 60001
#define UIP_IP_BUF   ((struct uip_ip_hdr *)&uip_buf[UIP_LLH_LEN])
#define UIP_UDP_BUF  ((struct uip_udp_hdr *)&uip_buf[uip_l2_l3_hdr_len])
/*---------------------------------------------------------------------------*/
static struct uip_udp_conn *client_conn;
static uip_ipaddr_t server_ipaddr;
static void send_packet(void);
/*---------------------------------------------------------------------------*/
#endif

#if SSI_MODE_SLAVE
static uint16_t spi_len= 0;
static uint16_t udp_len= 0;
static uint8_t data_received = 1;
#endif

static uint8_t tx_data[SIZE];
static uint8_t rx_data[SIZE];


/*---------------------------------------------------------------------------*/
PROCESS(spi_test_process, "SPI test process");
AUTOSTART_PROCESSES(&spi_test_process);

#if SSI_MODE_SLAVE
/*---------------------------------------------------------------------------*/
int
ssi_input(void)
{
	spi_len = 0;
  while(spi_len < SIZE){
	while(!(REG(SSI0_BASE + SSI_SR) & SSI_SR_RNE));
    rx_data[spi_len] = SPI_RXBUF;

    /*buffer has been pre-loaded since interrupt doesn't trigger from byte 1
     * Wait until you have transmitted the pre-loaded values before u add extra data
     *
     ***This (len + FIFO) < SIZE is to ensure we wont write more data
     ***Than we want to transmit as slave (since we pre-load 4 bytes)
     * Additionally, it will only work as long as the data transmitted from Slave
     * are <= bytes than the data to be transmitted by the master
     * Since slave will only transmit upon reception this is a
     * realistic assumption (if you want to transmit more data than masters
     * transmission it has to be done over multiple master transmissions)*/
    if((spi_len + FIFO) < SIZE) {
      SPI_TXBUF = tx_data[spi_len + FIFO];
    }
    spi_len++;
  }
  data_received = 1;
  return 0;
}
/*---------------------------------------------------------------------------*/
void
ssi_reset(void)
{
  data_received = 0;
  udp_len = 0;

#if SPI_FW
  send_packet();
#endif
  int i;
  PRINTF("receiving");
  for(i = 0; i < SIZE; i++) {
    PRINTF("%x:", rx_data[i]);
  }
  PRINTF("\n");

	memset(tx_data, 0, SIZE);
	memset(rx_data, 0, SIZE);
  /*reset TX/RX FIFO*/
  SPI_FLUSH();

}
/*---------------------------------------------------------------------------*/
#endif

#if SPI_FW
/*---------------------------------------------------------------------------*/
static void
send_packet(void)
{
  char buf[spi_len + 1];
  memset(buf, 0, sizeof(buf));
  memcpy(buf, rx_data, spi_len);
  uip_udp_packet_sendto(client_conn, buf, sizeof(buf),
                        &server_ipaddr, UIP_HTONS(UDP_SERVER_PORT));
}
/*---------------------------------------------------------------------------*/
static void
tcpip_handler(void)
{
	char * rep = "OK";
	uint8_t i;
	if(uip_newdata()) {
		udp_len = uip_datalen();
		memcpy(tx_data, uip_appdata, udp_len);

		PRINTF("received %u bytes:", udp_len);
		  for(i = 0; i < udp_len; i++) {
			PRINTF("%x :", tx_data[i]);
		  }
		PRINTF("\n");


		  /*pre-load transmission buffer since interrupt is triggered at 4 bytes*/
		  for(i = 0; i < FIFO; i++) {
		    SPI_TXBUF = tx_data[i];
		  }

		/*reply*/
		uip_udp_packet_sendto(client_conn, rep , strlen(rep),
				&UIP_IP_BUF->srcipaddr, UIP_UDP_BUF->srcport);
	}
	return;
}
/*---------------------------------------------------------------------------*/
#endif

PROCESS_THREAD(spi_test_process, ev, data) {
  static struct etimer et;
  PROCESS_BEGIN();

#if SPI_FW
  /* new connection with remote host */
  client_conn = udp_new(NULL, UIP_HTONS(0), NULL);
  if(client_conn == NULL) {
    PRINTF("No UDP connection available, exiting the process!\n");
    PROCESS_EXIT();
  }
  udp_bind(client_conn, UIP_HTONS(UDP_CLIENT_PORT));

  uip_ip6addr(&server_ipaddr, 0xaaaa, 0, 0, 0, 0, 0, 0, 1);
#endif

#if SSI_MODE_SLAVE
  ssi_set_input(ssi_input);
  /*If slave clock is slower than master
   * rx buffer will be over-written*/
  etimer_set(&et, CLOCK_SECOND / 5);
#else
  etimer_set(&et, 3 * CLOCK_SECOND);
  int8_t i = 0;
#endif

  memset(tx_data, 0, sizeof(tx_data));
  memset(rx_data, 0, sizeof(rx_data));

  spi_init();
  /*GPIO_B= 1 PIN = 5 RF06 = RF1.17*/
  spi_cs_init(1, 5, 1);

  while(1) {
    PROCESS_YIELD();
#if SPI_FW
    if(ev == tcpip_event) {
      tcpip_handler();
    }
#endif

    if(etimer_expired(&et)) {
      etimer_reset(&et);

#if SSI_MODE_SLAVE
      if(data_received) {
        /*Prepare for next SSI TX/RX*/
        ssi_reset();
      }
#else
      GPIO_CLR_PIN(GPIO_PORT_TO_BASE(1), GPIO_PIN_MASK(5));
      for(i = 0; i < SIZE; i++) {
        tx_data[i] = i;
        SPI_WRITE(tx_data[i]);
        rx_data[i] = SPI_RXBUF;
      }
      GPIO_SET_PIN(GPIO_PORT_TO_BASE(1), GPIO_PIN_MASK(5));

      PRINTF("Sending:");
      for(i = 0; i < SIZE; i++) {
        PRINTF("%x", tx_data[i]);
      }
      PRINTF("\n");

      PRINTF("Received:");
      for(i = 0; i < SIZE; i++) {
        PRINTF("%x", rx_data[i]);
      }
      PRINTF("\n");
#endif

      leds_toggle(LEDS_RED);
    }
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
/**
 * @}
 * @}
 */

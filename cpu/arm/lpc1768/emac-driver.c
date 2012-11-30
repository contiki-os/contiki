/*
 * This is an example of how to write a network device driver ("packet
 * service") for Contiki. A packet service is a regular Contiki
 * service that does two things:
 * # Checks for incoming packets and delivers those to the TCP/IP stack
 * # Provides an output function that transmits packets
 *
 * The output function is registered with the Contiki service
 * mechanism, whereas incoming packets must be checked inside a
 * Contiki process. We use the same process for checking for incoming
 * packets and for registering the service.
 *
 * NOTE: This example does not work with the uip-fw module (packet
 * forwarding with multiple interfaces). It only works with a single
 * interface.
 */

#include "emac-driver.h"
#include "emac.h"
#include <stdio.h>

#define BUF ((struct uip_eth_hdr *)&uip_buf[0])
#define IPBUF ((struct uip_tcpip_hdr *)&uip_buf[UIP_LLH_LEN])

/*---------------------------------------------------------------------------*/
/*
 * We declare the process that we use to register the service, and to
 * check for incoming packets.
 */
PROCESS(emac_lpc1768, "LPC1768 EMAC Service Process");

static struct etimer timer;  // for periodic ARP processing

/*---------------------------------------------------------------------------*/
/*
 * This is the poll handler function in the process below. This poll
 * handler function checks for incoming packets and delivers them to
 * the TCP/IP stack.
 */
static void
pollhandler(void)
{
  /*
   * We assume that we have some hardware device that notifies us when
   * a new packet has arrived. We also assume that we have a function
   * that pulls out the new packet (here called
   * check_and_copy_packet()) and puts it in the uip_buf[] buffer. The
   * function returns the length of the incoming packet, and we store
   * it in the global uip_len variable. If the packet is longer than
   * zero bytes, we hand it over to the TCP/IP stack.
   */
  uip_len = tapdev_read(uip_buf);

  /*
   * The function tcpip_input() delivers the packet in the uip_buf[]
   * buffer to the TCP/IP stack.
   */
  if (uip_len > 0)
    {
      printf("Received packet of %u bytes\n", uip_len);
#if UIP_CONF_IPV6
      if (BUF ->type == uip_htons(UIP_ETHTYPE_IPV6))
        {
          printf("Habemus IPv6 packet!\n");
          //uip_neighbor_add(&IPBUF ->srcipaddr, &BUF ->src);
          tcpip_input();
          //uip_input();
        }
#else
      if (BUF ->type == UIP_HTONS(UIP_ETHTYPE_IP))
        {
          printf("Habemus IP packet\n");
          printf("IP type is %x\n", IPBUF ->vhl);
          uip_arp_ipin();
          uip_input();
          /* If the above function invocation resulted in data that
           should be sent out on the network, the global variable
           uip_len is set to a value > 0. */

          if (uip_len > 0)
            {
              uip_arp_out();
              tapdev_send(uip_buf, uip_len);
            }
        }
      else if (BUF ->type == UIP_HTONS(UIP_ETHTYPE_ARP))
        {
          printf("Habemus ARP packet\n");
          uip_arp_arpin();
          /* If the above function invocation resulted in data that
           should be sent out on the network, the global variable
           uip_len is set to a value > 0. */
          if (uip_len > 0)
            {
              tapdev_send(uip_buf, uip_len);
            }
        }
#endif
      //If we don't know how to process it, just discard the packet
      else{
          uip_len=0;
      }
    }

  /*
   * Now we'll make sure that the poll handler is executed
   * repeatedly. We do this by calling process_poll() with this
   * process as its argument.
   *
   * In many cases, the hardware will cause an interrupt to be
   * executed when a new packet arrives. For such hardware devices,
   * the interrupt handler calls process_poll() (which is safe to use
   * in an interrupt context) instead.
   */
  process_poll(&emac_lpc1768);
}
/*---------------------------------------------------------------------------*/
/*
 * Next, we define the function that transmits packets. This function
 * is called from the TCP/IP stack when a packet is to be
 * transmitted. The packet is located in the uip_buf[] buffer, and the
 * length of the packet is in the uip_len variable.
 */
#if UIP_CONF_IPV6
uint8_t
send_packet(uip_lladdr_t * lladdr)
#else
uint8_t
send_packet(void)
#endif
{
#if UIP_CONF_IPV6
  /*
   * If L3 dest is multicast, build L2 multicast address
   * as per RFC 2464 section 7
   * else fill with th eaddrsess in argument
   */
  if (lladdr == NULL )
    {
      /* the dest must be multicast */
      (&BUF ->dest)->addr[0] = 0x33;
      (&BUF ->dest)->addr[1] = 0x33;
      (&BUF ->dest)->addr[2] = IPBUF ->destipaddr.u8[12];
      (&BUF ->dest)->addr[3] = IPBUF ->destipaddr.u8[13];
      (&BUF ->dest)->addr[4] = IPBUF ->destipaddr.u8[14];
      (&BUF ->dest)->addr[5] = IPBUF ->destipaddr.u8[15];
    }
  else
    {
      memcpy(&BUF ->dest, lladdr, UIP_LLADDR_LEN);
    }
  memcpy(&BUF ->src, &uip_lladdr, UIP_LLADDR_LEN);
  BUF ->type = UIP_HTONS(UIP_ETHTYPE_IPV6); //math tmp

  uip_len += sizeof(struct uip_eth_hdr);

#else
  uip_arp_out();
#endif

  tapdev_send(uip_buf, uip_len);
}

//This is just a wrapper for the Ethernet module interrupt
//to call a contiki process_poll function
void
poll_eth_driver(void)
{
  process_poll(&emac_lpc1768);
}

/*---------------------------------------------------------------------------*/
/*
 * Now we declare the service. We call the service
 * example_packet_service because of the name of this file. The
 * service should be an instance of the "packet service" service, so
 * we give packet_service as the second argument. Finally we give our
 * send_packet() function as the last argument, because of how the
 * packet_service interface is defined.
 *
 * We'll register this service with the Contiki system in the process
 * defined below.
 */
/*---------------------------------------------------------------------------*/
/*
 * Finally, we define the process that does the work.
 */PROCESS_THREAD(emac_lpc1768, ev, data)
{
  /*
   * This process has a poll handler, so we declare it here. Note that
   * the PROCESS_POLLHANDLER() macro must come before the
   * PROCESS_BEGIN() macro.
   */
  PROCESS_POLLHANDLER(pollhandler());

  /*
   * The process begins here.
   */PROCESS_BEGIN()
    ;

    /*
     * We start with initializing the hardware.
     */
    tapdev_init();

    tcpip_set_outputfunc(send_packet);

    process_poll(&emac_lpc1768);

#if UIP_CONF_IPV6
    PROCESS_WAIT_UNTIL(ev == PROCESS_EVENT_EXIT);

#else
    // 10 second ARP timer
    etimer_set(&timer, 10 * CLOCK_SECOND);

    while (ev != PROCESS_EVENT_EXIT)
      {
        PROCESS_WAIT_EVENT()
        ;

        if (ev == PROCESS_EVENT_TIMER)
          {
            etimer_set(&timer, 10 * CLOCK_SECOND);
            uip_arp_timer();
          }
      }
#endif
    /*
     * Here endeth the process.
     */
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/


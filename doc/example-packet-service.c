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

/*
 * We include the "contiki-net.h" file to get all the network
 * functions.
 */
#include "contiki-net.h"

/*---------------------------------------------------------------------------*/
/*
 * We declare the process that we use to register the service, and to
 * check for incoming packets.
 */
PROCESS(example_packet_service_process, "Example packet service process");
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
  uip_len = check_and_copy_packet();

  /*
   * The function tcpip_input() delivers the packet in the uip_buf[]
   * buffer to the TCP/IP stack.
   */
  if(uip_len > 0) {
    tcpip_input();
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
  process_poll(&example_packet_service_process);
}
/*---------------------------------------------------------------------------*/
/*
 * Next, we define the function that transmits packets. This function
 * is called from the TCP/IP stack when a packet is to be
 * transmitted. The packet is located in the uip_buf[] buffer, and the
 * length of the packet is in the uip_len variable.
 */
static void
send_packet(void)
{
  let_the_hardware_send_the_packet(uip_buf, uip_len);
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
SERVICE(example_packet_service, packet_service, { send_packet });
/*---------------------------------------------------------------------------*/
/*
 * Finally, we define the process that does the work. 
 */
PROCESS_THREAD(example_packet_service_process, ev, data)
{
  /*
   * This process has a poll handler, so we declare it here. Note that
   * the PROCESS_POLLHANDLER() macro must come before the
   * PROCESS_BEGIN() macro.
   */
  PROCESS_POLLHANDLER(pollhandler());

  /*
   * The process begins here.
   */
  PROCESS_BEGIN();

  /*
   * We start with initializing the hardware.
   */
  initialize_the_hardware();

  /*
   * Register the service. This will cause any other instances of the
   * same service to be removed.
   */
  SERVICE_REGISTER(example_packet_service);

  /*
   * Now we'll make sure that the poll handler is executed
   * initially. We do this by calling process_poll() with this
   * process as its argument.
   */
  process_poll(&example_packet_service_process);

  /*
   * And we wait for either the process to exit, or for the service to
   * be removed (by someone else).
   */
  PROCESS_WAIT_EVENT_UNTIL(ev == PROCESS_EVENT_EXIT ||
			   ev == PROCESS_EVENT_SERVICE_REMOVED);

  /*
   * And we always end with explicitly removing the service.
   */  
  SERVICE_REMOVE(example_packet_service);

  /*
   * Here endeth the process.
   */
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/

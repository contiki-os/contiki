/*
 * This is an example of how to write a network device driver ("packet
 * driver") for Contiki. A packet driver is a regular Contiki process
 * that does two things:
 * # Checks for incoming packets and delivers those to the TCP/IP stack
 * # Provides an output function that transmits packets
 *
 * The output function is registered with the Contiki TCP/IP stack,
 * whereas incoming packets must be checked inside a Contiki process.
 * We use the same process for checking for incoming packets and for
 * registering the output function.
 */

/*
 * We include the "contiki-net.h" file to get all the network functions.
 */
#include "contiki-net.h"

/*---------------------------------------------------------------------------*/
/*
 * We declare the process that we use to register with the TCP/IP stack,
 * and to check for incoming packets.
 */
PROCESS(example_packet_driver_process, "Example packet driver process");
/*---------------------------------------------------------------------------*/
/*
 * Next, we define the function that transmits packets. This function
 * is called from the TCP/IP stack when a packet is to be transmitted.
 * The packet is located in the uip_buf[] buffer, and the length of the
 * packet is in the uip_len variable.
 */
uint8_t
example_packet_driver_output(void)
{
  let_the_hardware_send_the_packet(uip_buf, uip_len);

  /*
   * A network device driver returns always zero.
   */
  return 0;
}
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
   * Now we'll make sure that the poll handler is executed repeatedly.
   * We do this by calling process_poll() with this process as its
   * argument.
   *
   * In many cases, the hardware will cause an interrupt to be executed
   * when a new packet arrives. For such hardware devices, the interrupt
   * handler calls process_poll() (which is safe to use in an interrupt
   * context) instead.
   */
  process_poll(&example_packet_driver_process);
}
/*---------------------------------------------------------------------------*/
/*
 * Finally, we define the process that does the work. 
 */
PROCESS_THREAD(example_packet_driver_process, ev, data)
{
  /*
   * This process has a poll handler, so we declare it here. Note that
   * the PROCESS_POLLHANDLER() macro must come before the PROCESS_BEGIN()
   * macro.
   */
  PROCESS_POLLHANDLER(pollhandler());

  /*
   * This process has an exit handler, so we declare it here. Note that
   * the PROCESS_EXITHANDLER() macro must come before the PROCESS_BEGIN()
   * macro.
   */
  PROCESS_EXITHANDLER(exithandler());

  /*
   * The process begins here.
   */
  PROCESS_BEGIN();

  /*
   * We start with initializing the hardware.
   */
  initialize_the_hardware();

  /*
   * Register the driver. This will cause any previously registered driver
   * to be ignored by the TCP/IP stack.
   */
  tcpip_set_outputfunc(example_packet_driver_output);

  /*
   * Now we'll make sure that the poll handler is executed initially. We do
   * this by calling process_poll() with this process as its argument.
   */
  process_poll(&example_packet_driver_process);

  /*
   * And we wait for the process to exit.
   */
  PROCESS_WAIT_EVENT_UNTIL(ev == PROCESS_EVENT_EXIT);

  /*
   * Now we shutdown the hardware.
   */
  shutdown_the_hardware();

  /*
   * Here ends the process.
   */
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/

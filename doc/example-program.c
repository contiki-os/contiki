/*
 * This file contains an example of how a Contiki program looks.
 *
 * The program opens a UDP broadcast connection and sends one packet
 * every second.
 */

#include "contiki.h"
#include "contiki-net.h"

/*
 * All Contiki programs must have a process, and we declare it here.
 */
PROCESS(example_program_process, "Example process");

/*
 * To make the program send a packet once every second, we use an
 * event timer (etimer).
 */
static struct etimer timer;

/*---------------------------------------------------------------------------*/
/*
 * Here we implement the process. The process is run whenever an event
 * occurs, and the parameters "ev" and "data" will we set to the event
 * type and any data that may be passed along with the event.
 */
PROCESS_THREAD(example_program_process, ev, data)
{
  /*
   * Declare the UDP connection. Note that this *MUST* be declared
   * static, or otherwise the contents may be destroyed. The reason
   * for this is that the process runs as a protothread, and
   * protothreads do not support stack variables.
   */
  static struct uip_udp_conn *c;
  
  /*
   * A process thread starts with PROCESS_BEGIN() and ends with
   * PROCESS_END().
   */  
  PROCESS_BEGIN();

  /*
   * We create the UDP connection to port 4321. We don't want to
   * attach any special data to the connection, so we pass it a NULL
   * parameter.
   */
  c = udp_broadcast_new(UIP_HTONS(4321), NULL);
  
  /*
   * Loop for ever.
   */
  while(1) {

    /*
     * We set a timer that wakes us up once every second. 
     */
    etimer_set(&timer, CLOCK_SECOND);
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&timer));

    /*
     * Now, this is a the tricky bit: in order for us to send a UDP
     * packet, we must call upon the uIP TCP/IP stack process to call
     * us. (uIP works under the Hollywood principle: "Don't call us,
     * we'll call you".) We use the function tcpip_poll_udp() to tell
     * uIP to call us, and then we wait for the uIP event to come.
     */
    tcpip_poll_udp(c);
    PROCESS_WAIT_EVENT_UNTIL(ev == tcpip_event);

    /*
     * We can now send our packet.
     */
    uip_send("Hello", 5);

    /*
     * We're done now, so we'll just loop again.
     */
  }

  /*
   * The process ends here. Even though our program sits is a while(1)
   * loop, we must put the PROCESS_END() at the end of the process, or
   * else the program won't compile.
   */
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/

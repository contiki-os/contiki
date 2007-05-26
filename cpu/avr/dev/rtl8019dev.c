#include "net/uip.h"
#include "dev/rtl8019dev.h"

/*****************************************************************************
*  Module Name:       Realtek 8019AS Driver Interface for uIP-AVR Port
*
*  Created By:        Louis Beaudoin (www.embedded-creations.com)
*
*  Original Release:  September 21, 2002
*
*  Module Description:
*  Provides three functions to interface with the Realtek 8019AS driver
*  These functions can be called directly from the main uIP control loop
*  to send packets from uip_buf and uip_appbuf, and store incoming packets to
*  uip_buf
*
*  September 30, 2002 - Louis Beaudoin
*    Modifications required to handle the packet receive function changes in
*      rtl8019.c.  There is no longer a need to poll for an empty buffer or
*      an overflow.
*    Added support for the Imagecraft Compiler
*
*****************************************************************************/


#define IP_TCP_HEADER_LENGTH 40
#define TOTAL_HEADER_LENGTH (IP_TCP_HEADER_LENGTH+ETHERNET_HEADER_LENGTH)



void RTL8019dev_init(void)
{
  initRTL8019();
}


void RTL8019dev_send(void)
{
  RTL8019beginPacketSend(uip_len);
  
  // send packet, using data in uip_appdata if over the IP+TCP header size
  if( uip_len <= TOTAL_HEADER_LENGTH ) {
    RTL8019sendPacketData(uip_buf, uip_len);
  } else {
    uip_len -= TOTAL_HEADER_LENGTH;
    RTL8019sendPacketData(uip_buf, TOTAL_HEADER_LENGTH);
    RTL8019sendPacketData((unsigned char *)uip_appdata, uip_len);
  }
  
  RTL8019endPacketSend();
}



unsigned int RTL8019dev_poll(void)
{
  unsigned int packetLength;
	
  packetLength = RTL8019beginPacketRetreive();

  // if there's no packet or an error - exit without ending the operation
  if( !packetLength )
    return 0;

  // drop anything too big for the buffer
  if( packetLength > UIP_BUFSIZE )
  {
    RTL8019endPacketRetreive();
    return 0;
  }
	
  // copy the packet data into the uIP packet buffer
  RTL8019retreivePacketData( uip_buf, packetLength );
  RTL8019endPacketRetreive();
		
  return packetLength;
}


void RTL8019dev_exit(void)
{
}

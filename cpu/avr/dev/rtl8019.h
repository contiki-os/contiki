#ifndef __RTL8019_H__
#define __RTL8019_H__

/*****************************************************************************
*  Module Name:       Realtek 8019AS Driver
*  
*  Created By:        Louis Beaudoin (www.embedded-creations.com)
*
*  Original Release:  September 21, 2002 
*
*  Module Description:  
*  Provides functions to initialize the Realtek 8019AS, and send and retreive
*  packets
*
*  September 30, 2002 - Louis Beaudoin
*    Receive functions modified to handle errors encountered when receiving a
*      fast data stream.  Functions now manually retreive data instead of
*      using the send packet command.  Interface improved by checking for
*      overruns and data in the buffer internally.
*    Corrected the overrun function - overrun flag was not reset after overrun
*    Added support for the Imagecraft Compiler
*    Added support to communicate with the NIC using general I/O ports
*
*****************************************************************************/

/*#include "delay.h"*/

#include "compiler.h"


/*****************************************************************************
*  RTL8019beginPacketSend(unsigned int packetLength)
*  Args:        unsigned int - length of the Ethernet frame (see note)
*  Created By:  Louis Beaudoin
*  Date:        September 21, 2002
*  Description: Sets up the NIC to send a packet
*  Notes:       The NIC will not send packets less than 60 bytes long (the min
*                 Ethernet frame length.  The transmit length is automatically
*                 increased to 60 bytes if packetLength is < 60
*****************************************************************************/
void RTL8019beginPacketSend(unsigned int packetLength);


/*****************************************************************************
*  RTL8019sendPacketData(unsigned char * localBuffer, unsigned int length)
*  Args:        1. unsigned char * localBuffer - Pointer to the beginning of
*                    the buffer to load into the NIC
*               2. unsigned char length - number of bytes to copy to
*                    the NIC
*  Created By:  Louis Beaudoin
*  Date:        September 21, 2002
*  Description: Loads length # of bytes from a local buffer to the transmit
*                 packet buffer
*  Notes:       RTL8019beginPacketSend() must be called before sending
*                 any data.
*               Several calls to RTL8019retreivePacketData() may be made to 
*                 copy packet data from different buffers
*****************************************************************************/
void RTL8019sendPacketData(unsigned char * localBuffer, unsigned int length);


/*****************************************************************************
*  RTL8019endPacketSend()
*  Created By:  Louis Beaudoin
*  Date:        September 21, 2002
*  Description: Ends a packet send operation and instructs the NIC to transmit
*                 the frame over the network
*****************************************************************************/
void RTL8019endPacketSend(void);


/*****************************************************************************
*  initRTL8019(void);
*
*  Created By:  Louis Beaudoin
*  Date:        September 21, 2002
*  Description: Sets up the RTL8019 NIC hardware interface, and initializes
*                 the buffers and configuration of the NIC
*****************************************************************************/
void initRTL8019(void);


/*****************************************************************************
*  processRTL8019Interrupt(void);
*
*  Created By:  Louis Beaudoin
*  Date:        September 21, 2002
*  Description: Reads the NIC's ISR register looking for a receive buffer
*                 overrun - which is then handled.
*  Notes:       The function does not need to be called in response to an
*                 interrupt.  The function can be polled and the NIC's INT
*                 line not used.  This function should be called before
*                 attempting to retreive a packet from the NIC
*****************************************************************************/
void processRTL8019Interrupt(void);


/*****************************************************************************
*  unsigned char RTL8019ReceiveEmpty(void);
*
*  Returns:     non-zero (true) if buffer is empty, zero if data in buffer
*  Created By:  Louis Beaudoin
*  Date:        September 21, 2002
*  Description: Compares the BNRY and CURR receive buffer pointers to see if
*                 there is a packet in the receive buffer
*  ** Removed as of version 0.60.1 **
*****************************************************************************/
//unsigned char RTL8019ReceiveEmpty(void);


/*****************************************************************************
*  unsigned int RTL8019beginPacketRetreive()
*  Returns:     unsigned int - length of the Ethernet frame (see note)
*  Created By:  Louis Beaudoin
*  Date:        September 21, 2002
*  Description: Sets up the NIC to retreive a packet
*  Notes:       The size returned is the size of all the data in the Ethernet
*                 frame minus the Ethernet checksum.  This may include unused
*                 trailer bytes appended if data is less than the minimum
*                 Ethernet frame length (60 bytes).  A size of zero indicates
*                 there are no packets available.
*               A call to RTL8019beginPacketRetreive() must be followed by a
*                 call to RTL8019endPacketRetreive() regardless if data is
*                 retreived, unless 0 is returned.
*****************************************************************************/
unsigned int RTL8019beginPacketRetreive(void);


/*****************************************************************************
*  RTL8019retreivePacketData(unsigned char * localBuffer, unsigned int length)
*  Args:        1. unsigned char * localBuffer - Pointer to the beginning of
*                    the buffer to store the ethernet frame.
*               2. unsigned char length - number of bytes to copy to
*                    localBuffer
*  Created By:  Louis Beaudoin
*  Date:        September 21, 2002
*  Description: Loads length # of bytes from the receive packet buffer to
*                 a local buffer
*  Notes:       RTL8019beginPacketRetreive() must be called before retreiving
*                 any data.
*               Several calls to RTL8019retreivePacketData() may be made to 
*                 copy packet data to different buffers
*****************************************************************************/
void RTL8019retreivePacketData(unsigned char * localBuffer,
                               unsigned int length);

/*****************************************************************************
*  RTL8019endPacketRetreive()
*  Created By:  Louis Beaudoin
*  Date:        September 21, 2002
*  Description: Ends a packet retreive operation begun by calling
*                 RTL8019beginPacketRetreive().  The NIC buffer space used by
*                 the retreived packet is freed
*  Notes:       A packet may be removed from the buffer without being read
*                 by calling RTL8019endPacketRetreive() after
*                 RTL8019beginPacketRetreive().
*****************************************************************************/
void RTL8019endPacketRetreive(void);


/*****************************************************************************
*
*  AVR hardware setup
*
*    External SRAM Interface:
*      The five NIC address lines are taken from A8-A12 (uses the
*      non-multiplexed address port so no latch is required)
*
*    General I/O Interface:
*      Two full ports are required for the address and data buses.  Two pins
*        from another port are used to control the read and write lines
*
*    One output pin is required for hard resetting the NIC
*
*****************************************************************************/

// set to 1 to use the External SRAM Interface - 0 for General I/O
#define MEMORY_MAPPED_NIC 1

#if MEMORY_MAPPED_NIC /*** NIC Interface through External SRAM Interface ****/

// NIC is mapped from address 0x8000 - 0x9F00
#define MEMORY_MAPPED_RTL8019_OFFSET 0x8300

#else /************ NIC Interface through General I/O *******************/

// RTL8019 address port
#define RTL8019_ADDRESS_PORT        PORTC
#define RTL8019_ADDRESS_DDR         DDRC

// RTL8019 data port
#define RTL8019_DATA_PORT           PORTA
#define RTL8019_DATA_DDR            DDRA
#define RTL8019_DATA_PIN            PINA

// RTL8019 control port
#define RTL8019_CONTROL_PORT        PORTD
#define RTL8019_CONTROL_DDR         DDRD
#define RTL8019_CONTROL_READPIN     PD7
#define RTL8019_CONTROL_WRITEPIN    PD6


// macros to control the read and write pins
#define RTL8019_CLEAR_READ   cbi(RTL8019_CONTROL_PORT,\
                                 RTL8019_CONTROL_READPIN)
#define RTL8019_SET_READ     sbi(RTL8019_CONTROL_PORT,\
                                 RTL8019_CONTROL_READPIN) 
#define RTL8019_CLEAR_WRITE  cbi(RTL8019_CONTROL_PORT,\
                                 RTL8019_CONTROL_WRITEPIN)
#define RTL8019_SET_WRITE    sbi(RTL8019_CONTROL_PORT,\
                                 RTL8019_CONTROL_WRITEPIN)

#endif /** NIC Interface **/



// RTL RESET - Port B pin 0
#define RTL8019_RESET_PORT 	PORTE
#define RTL8019_RESET_DDR 	DDRE
#define RTL8019_RESET_PIN 	PORTE0





/*****************************************************************************
*
*  Ethernet constants
*
*****************************************************************************/
#define ETHERNET_MIN_PACKET_LENGTH	0x3C
#define ETHERNET_HEADER_LENGTH		0x0E



/*****************************************************************************
*
* MAC address assigned to the RTL8019
*
*****************************************************************************/
/*#define MYMAC_0 '0'
#define MYMAC_1 'F'
#define MYMAC_2 'F'
#define MYMAC_3 'I'
#define MYMAC_4 'C'
#define MYMAC_5 'E'*/

#define MYMAC_0 0x00
#define MYMAC_1 0x06
#define MYMAC_2 0x98
#define MYMAC_3 0x01
#define MYMAC_4 0x02
#define MYMAC_5 0x26











#endif /* __RTL8019_H__ */

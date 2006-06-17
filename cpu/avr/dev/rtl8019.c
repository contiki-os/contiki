#include "rtl8019.h"
#include "delay.h"
#include "debug.h"
#include "avr/pgmspace.h"
#include "rtlregs.h"

#ifndef cbi
#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#endif
#ifndef sbi
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
#endif

#define outp(val, port) do { (port) = (val); } while(0)
#define inp(port) (port)

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
*  November 15, 2002 - Louis Beaudoin
*    processRTL8019Interrupt() - bit mask mistake fixed
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


/*****************************************************************************
*  writeRTL( RTL_ADDRESS, RTL_DATA )
*  Args:        1. unsigned char RTL_ADDRESS - register offset of RTL register
*               2. unsigned char RTL_DATA - data to write to register
*  Created By:  Louis Beaudoin
*  Date:        September 21, 2002
*  Description: Writes byte to RTL8019 register.
*
*  Notes - If using the External SRAM Interface, performs a write to
*            address MEMORY_MAPPED_RTL8019_OFFSET + (RTL_ADDRESS<<8)
*            The address is sent in the non-multiplxed upper address port so
*            no latch is required.
*
*          If using general I/O ports, the data port is left in the input
*            state with pullups enabled
*
*****************************************************************************/
#if MEMORY_MAPPED_NIC == 1
/*#define writeRTL(RTL_ADDRESS,RTL_DATA) do{ *(volatile unsigned char *) \
                             (MEMORY_MAPPED_RTL8019_OFFSET \
                             + (((unsigned char)(RTL_ADDRESS)) << 8)) = \
                             (unsigned char)(RTL_DATA); } while(0)*/
#define writeRTL nic_write
#else


void writeRTL(unsigned char address, unsigned char data)
{
    // put the address and data in the port registers - data port is output
    outp( address, RTL8019_ADDRESS_PORT );
    outp( 0xFF, RTL8019_DATA_DDR );
    outp( data, RTL8019_DATA_PORT );
    
	// toggle write pin
    RTL8019_CLEAR_WRITE;
    RTL8019_SET_WRITE;
    
	// set data port back to input with pullups enabled
    outp( 0x00, RTL8019_DATA_DDR );
    outp( 0xFF, RTL8019_DATA_PORT );
}



#endif

/*****************************************************************************
*  readRTL(RTL_ADDRESS)
*  Args:        unsigned char RTL_ADDRESS - register offset of RTL register
*  Created By:  Louis Beaudoin
*  Date:        September 21, 2002
*  Description: Reads byte from RTL8019 register
*
*  Notes - If using the External SRAM Interface, performs a read from
*            address MEMORY_MAPPED_RTL8019_OFFSET + (RTL_ADDRESS<<8)
*            The address is sent in the non-multiplxed upper address port so
*            no latch is required.
*
*          If using general I/O ports, the data port is assumed to already be
*            an input, and is left as an input port when done
*
*****************************************************************************/
#if MEMORY_MAPPED_NIC == 1
/*#define readRTL(RTL_ADDRESS) (*(volatile unsigned char *) \
                       (MEMORY_MAPPED_RTL8019_OFFSET \
                       + (((unsigned char)(RTL_ADDRESS)) << 8)) )*/
#define readRTL nic_read
#else

unsigned char readRTL(unsigned char address)
{
   unsigned char byte;
   
   // drive the read address
   outp( address, RTL8019_ADDRESS_PORT );
    
   //nop();
   
   // assert read
   RTL8019_CLEAR_READ;
   nop();
   
   // read in the data
   byte = inp( RTL8019_DATA_PIN );

   // negate read
   RTL8019_SET_READ;

   return byte;
}

#endif



/*****************************************************************************
*  RTL8019setupPorts(void);
*
*  Created By:  Louis Beaudoin
*  Date:        September 21, 2002
*  Description: Sets up the ports used for communication with the RTL8019 NIC
*                 (data bus, address bus, read, write, and reset)
*****************************************************************************/
void RTL8019setupPorts(void)
{
volatile unsigned char *base = (unsigned char *)0x8300;

#if MEMORY_MAPPED_NIC == 1
  	// enable external SRAM interface - no wait states
  	outp(inp(MCUCR) | (1<<SRE), MCUCR);

#else

    // make the address port output
    outp( 0xFF, RTL8019_ADDRESS_DDR );
    
    // make the data port input with pull-ups
    outp( 0xFF, RTL8019_DATA_PORT );

	// make the control port read and write pins outputs and asserted
	//outp( inp(RTL8019_CONTROL_DDR) | (1<<RTL8019_CONTROL_READPIN) |
	//          (1<<RTL8019_CONTROL_WRITEPIN), RTL8019_CONTROL_DDR );
	sbi( RTL8019_CONTROL_DDR, RTL8019_CONTROL_READPIN );
	sbi( RTL8019_CONTROL_DDR, RTL8019_CONTROL_WRITEPIN );
	          
	//outp( inp(RTL8019_CONTROL_PORT) | (1<<RTL8019_CONTROL_READPIN) |
	//          (1<<RTL8019_CONTROL_WRITEPIN), RTL8019_CONTROL_PORT );
	sbi( RTL8019_CONTROL_PORT, RTL8019_CONTROL_READPIN );
	sbi( RTL8019_CONTROL_PORT, RTL8019_CONTROL_WRITEPIN );

#endif

	// enable output pin for Resetting the RTL8019
	sbi( RTL8019_RESET_DDR, RTL8019_RESET_PIN );
	
	
	


}



/*****************************************************************************
*  HARD_RESET_RTL8019()
*
*  Created By:  Louis Beaudoin
*  Date:        September 21, 2002
*  Description: Simply toggles the pin that resets the NIC
*****************************************************************************/
/*#define HARD_RESET_RTL8019() do{ sbi(RTL8019_RESET_PORT, RTL8019_RESET_PIN); \
                                Delay_10ms(1); \
                                cbi(RTL8019_RESET_PORT, RTL8019_RESET_PIN);} \
                                while(0)*/



/*****************************************************************************
*  overrun(void);
*
*  Created By:  Louis Beaudoin
*  Date:        September 21, 2002
*  Description: "Canned" receive buffer overrun function originally from
*                 a National Semiconductor appnote
*  Notes:       This function must be called before retreiving packets from
*                 the NIC if there is a buffer overrun
*****************************************************************************/
void overrun(void);




//******************************************************************
//*	REALTEK CONTROL REGISTER OFFSETS
//*   All offsets in Page 0 unless otherwise specified
//*	  All functions accessing CR must leave CR in page 0 upon exit
//******************************************************************
#define CR		 	0x00
#define PSTART		0x01
#define PAR0      	0x01    // Page 1
#define CR9346    	0x01    // Page 3
#define PSTOP		0x02
#define BNRY		0x03
#define TSR			0x04
#define TPSR		0x04
#define TBCR0		0x05
#define NCR			0x05
#define TBCR1		0x06
#define ISR			0x07
#define CURR		0x07   // Page 1
#define RSAR0		0x08
#define CRDA0		0x08
#define RSAR1		0x09
#define CRDA1		0x09
#define RBCR0		0x0A
#define RBCR1		0x0B
#define RSR			0x0C
#define RCR			0x0C
#define TCR			0x0D
#define CNTR0		0x0D
#define DCR			0x0E
#define CNTR1		0x0E
#define IMR			0x0F
#define CNTR2		0x0F
#define RDMAPORT  	0x10
#define RSTPORT   	0x18


/*****************************************************************************
*
* RTL ISR Register Bits
*
*****************************************************************************/
#define ISR_RST	7
#define ISR_OVW 4
#define ISR_PRX 0
#define ISR_RDC 6
#define ISR_PTX 1


/*****************************************************************************
*
*  RTL Register Initialization Values
*
*****************************************************************************/
// RCR : accept broadcast packets and packets destined to this MAC
//         drop short frames and receive errors
#define RCR_INIT		0x04

// TCR : default transmit operation - CRC is generated
#define TCR_INIT		0x00

// DCR : allows send packet to be used for packet retreival
//         FIFO threshold: 8-bits (works)
//         8-bit transfer mode
#define DCR_INIT		0x58

// IMR : interrupt enabled for receive and overrun events
#define IMR_INIT		0x11

// buffer boundaries - transmit has 6 256-byte pages
//   receive has 26 256-byte pages
//   entire available packet buffer space is allocated
#define TXSTART_INIT   	0x40
#define RXSTART_INIT   	0x46
#define RXSTOP_INIT    	0x60



void RTL8019beginPacketSend(unsigned int packetLength)
{

  volatile unsigned char *base = (unsigned char *)0x8300;
	unsigned int sendPacketLength;
	sendPacketLength = (packetLength>=ETHERNET_MIN_PACKET_LENGTH) ?
	                 packetLength : ETHERNET_MIN_PACKET_LENGTH ;
	
	//start the NIC
	writeRTL(CR,0x22);
	
	// still transmitting a packet - wait for it to finish
	while( readRTL(CR) & 0x04 );

	//load beginning page for transmit buffer
	writeRTL(TPSR,TXSTART_INIT);
	
	//set start address for remote DMA operation
	writeRTL(RSAR0,0x00);
	writeRTL(RSAR1,0x40);
	
	//clear the packet stored interrupt
	writeRTL(ISR,(1<<ISR_PTX));

	//load data byte count for remote DMA
	writeRTL(RBCR0, (unsigned char)(packetLength));
	writeRTL(RBCR1, (unsigned char)(packetLength>>8));

	writeRTL(TBCR0, (unsigned char)(sendPacketLength));
	writeRTL(TBCR1, (unsigned char)((sendPacketLength)>>8));
	
	//do remote write operation
	writeRTL(CR,0x12);
}



void RTL8019sendPacketData(unsigned char * localBuffer, unsigned int length)
{
	unsigned int i;
	volatile unsigned char *base = (unsigned char *)0x8300;
	for(i=0;i<length;i++)
		writeRTL(RDMAPORT, localBuffer[i]);
}



void RTL8019endPacketSend(void)
{
  volatile unsigned char *base = (unsigned char *)0x8300;
	//send the contents of the transmit buffer onto the network
	writeRTL(CR,0x24);
	
	// clear the remote DMA interrupt
	writeRTL(ISR, (1<<ISR_RDC));
}




// pointers to locations in the RTL8019 receive buffer
static unsigned char nextPage;
static unsigned int currentRetreiveAddress;

// location of items in the RTL8019's page header
#define  enetpacketstatus     0x00
#define  nextblock_ptr        0x01
#define	 enetpacketLenL		  0x02
#define	 enetpacketLenH		  0x03



unsigned int RTL8019beginPacketRetreive(void)
{
  volatile unsigned char *base = (unsigned char *)0x8300;
	unsigned char i;
	unsigned char bnry;
	
	unsigned char pageheader[4];
	unsigned int rxlen;
	
	// check for and handle an overflow
	processRTL8019Interrupt();
	
	// read CURR from page 1
	writeRTL(CR,0x62);
	i = readRTL(CURR);
	
	// return to page 0
	writeRTL(CR,0x22);
	
	// read the boundary register - pointing to the beginning of the packet
	bnry = readRTL(BNRY) ;

	/*	debug_print(PSTR("bnry: "));
		debug_print8(bnry);*/

	/*	debug_print(PSTR("RXSTOP_INIT: "));
	debug_print8(RXSTOP_INIT);
	debug_print(PSTR("RXSTART_INIT: "));
	debug_print8(RXSTART_INIT);*/
	// return if there is no packet in the buffer
	if( bnry == i ) {
	  return 0;
	}
	

	// clear the packet received interrupt flag
	writeRTL(ISR, (1<<ISR_PRX));
	
	
	// the boundary pointer is invalid, reset the contents of the buffer and exit
	if( (bnry >= RXSTOP_INIT) || (bnry < RXSTART_INIT) )
	{
		writeRTL(BNRY, RXSTART_INIT);
		writeRTL(CR, 0x62);
		writeRTL(CURR, RXSTART_INIT);
		writeRTL(CR, 0x22);
		return 0;
	}

	// initiate DMA to transfer the RTL8019 packet header
    writeRTL(RBCR0, 4);
    writeRTL(RBCR1, 0);
    writeRTL(RSAR0, 0);
    writeRTL(RSAR1, bnry);
    writeRTL(CR, 0x0A);
    /*    	debug_print(PSTR("Page header: "));*/

	for(i=0;i<4;i++) {
	  pageheader[i] = readRTL(RDMAPORT);
	  /*	  debug_print8(pageheader[i]);*/
	}
	
	// end the DMA operation
    writeRTL(CR, 0x22);
    for(i = 0; i <= 20; i++) {
      if(readRTL(ISR) & 1<<6) {
	break;
      }
    }
    writeRTL(ISR, 1<<6);

	
	
	rxlen = (pageheader[enetpacketLenH]<<8) + pageheader[enetpacketLenL];
	nextPage = pageheader[nextblock_ptr] ;
	
	currentRetreiveAddress = (bnry<<8) + 4;

	/*	debug_print(PSTR("nextPage: "));
		debug_print8(nextPage);*/
	
	// if the nextPage pointer is invalid, the packet is not ready yet - exit
	if( (nextPage >= RXSTOP_INIT) || (nextPage < RXSTART_INIT) ) {
	  /*	  UDR0 = '0';*/
	  return 0;
	}
    
    return rxlen-4;
}


void RTL8019retreivePacketData(unsigned char * localBuffer, unsigned int length)
{
	unsigned int i;
	volatile unsigned char *base = (unsigned char *)0x8300;
	// initiate DMA to transfer the data
    writeRTL(RBCR0, (unsigned char)length);
    writeRTL(RBCR1, (unsigned char)(length>>8));
    writeRTL(RSAR0, (unsigned char)currentRetreiveAddress);
    writeRTL(RSAR1, (unsigned char)(currentRetreiveAddress>>8));
    writeRTL(CR, 0x0A);
	for(i=0;i<length;i++)
		localBuffer[i] = readRTL(RDMAPORT);

	// end the DMA operation
    writeRTL(CR, 0x22);
    for(i = 0; i <= 20; i++)
        if(readRTL(ISR) & 1<<6)
            break;
    writeRTL(ISR, 1<<6);
    
    currentRetreiveAddress += length;
    if( currentRetreiveAddress >= 0x6000 )
    	currentRetreiveAddress = currentRetreiveAddress - (0x6000-0x4600) ;
}



void RTL8019endPacketRetreive(void)
{
  volatile unsigned char *base = (unsigned char *)0x8300;
	unsigned char i;

	// end the DMA operation
    writeRTL(CR, 0x22);
    for(i = 0; i <= 20; i++)
        if(readRTL(ISR) & 1<<6)
            break;
    writeRTL(ISR, 1<<6);

	// set the boundary register to point to the start of the next packet
    writeRTL(BNRY, nextPage);
}


void overrun(void)
{
  volatile unsigned char *base = (unsigned char *)0x8300;
	unsigned char data_L, resend;

	data_L = readRTL(CR);
	writeRTL(CR, 0x21);
	Delay_1ms(2);
	writeRTL(RBCR0, 0x00);
	writeRTL(RBCR1, 0x00);
	if(!(data_L & 0x04))
		resend = 0;
	else if(data_L & 0x04)
	{
		data_L = readRTL(ISR);
		if((data_L & 0x02) || (data_L & 0x08))
	    	resend = 0;
	    else
	    	resend = 1;
	}
	
	writeRTL(TCR, 0x02);
	writeRTL(CR, 0x22);
	writeRTL(BNRY, RXSTART_INIT);
	writeRTL(CR, 0x62);
	writeRTL(CURR, RXSTART_INIT);
	writeRTL(CR, 0x22);
	writeRTL(ISR, 0x10);
	writeRTL(TCR, TCR_INIT);
	
	writeRTL(ISR, 0xFF);
}




/*!
 * \brief Size of a single ring buffer page.
 */
#define NIC_PAGE_SIZE   0x100

/*!
 * \brief First ring buffer page address.
 */
#define NIC_START_PAGE  0x40

/*!
 * \brief Last ring buffer page address plus 1.
 */
#define NIC_STOP_PAGE   0x60

/*!
 * \brief Number of pages in a single transmit buffer.
 *
 * This should be at least the MTU size.
 */
#define NIC_TX_PAGES    6

/*!
 * \brief Number of transmit buffers.
 */
#define NIC_TX_BUFFERS  2

/*!
 * \brief Controller memory layout:
 *
 * 0x4000 - 0x4bff  3k bytes transmit buffer
 * 0x4c00 - 0x5fff  5k bytes receive buffer
 */
#define NIC_FIRST_TX_PAGE   NIC_START_PAGE
#define NIC_FIRST_RX_PAGE   (NIC_FIRST_TX_PAGE + NIC_TX_PAGES * NIC_TX_BUFFERS)

/*!
 * \brief Standard sizing information
 */
#define TX_PAGES 12         /* Allow for 2 back-to-back frames */

static unsigned char mac[6] = {0x00,0x06,0x98,0x01,0x02,0x29};
void Delay(long nops)
{
    volatile long i;

    for(i = 0; i < nops; i++)
#ifdef __IMAGECRAFT__
        asm("nop\n");
#else
        asm volatile("nop\n\t"::);
#endif
}

static int NicReset(void)
{
volatile unsigned char *base = (unsigned char *)0x8300;
    unsigned char i;
    unsigned char j;

    for(j = 0; j < 20; j++) {
        debug_print(PSTR("SW-Reset..."));
        i = nic_read(NIC_RESET);
        Delay(500);
        nic_write(NIC_RESET, i);
        for(i = 0; i < 20; i++) {
            Delay(5000);

            /*
             * ID detection added for version 1.1 boards.
             */
            if((nic_read(NIC_PG0_ISR) & NIC_ISR_RST) != 0 &&
               nic_read(NIC_PG0_RBCR0) == 0x50 &&
               nic_read(NIC_PG0_RBCR1) == 0x70) {
                debug_print(PSTR("OK\r\n"));
                return 0;
            }
        }
        debug_print(PSTR("failed\r\n\x07"));

        /*
         * Toggle the hardware reset line. Since Ethernut version 1.3 the
         * hardware reset pin of the nic is no longer connected to bit 4
         * on port E, but wired to the board reset line.
         */
        if(j == 10) {
            debug_print(PSTR("Ethernut 1.1 HW-Reset\r\n"));
            sbi(DDRE, 4);
            sbi(PORTE, 4);
            Delay(100000);
            cbi(PORTE, 4);
            Delay(250000);
        }
    }
    return -1;
}

void initRTL8019(void)
{
  unsigned char i, rb;
  volatile unsigned char *base = (unsigned char *)0x8300;
  
  RTL8019setupPorts();

  /*#define nic_write writeRTL
    #define nic_read readRTL*/
      /*
     * Disable NIC interrupts.
     */
    cbi(EIMSK, INT5);

    /*    if(NicReset(base))
	  return -1;*/
#if 0
    /*
     * Mask all interrupts and clear any interrupt status flag to set the
     * INT pin back to low.
     */
    nic_write(NIC_PG0_IMR, 0);
    nic_write(NIC_PG0_ISR, 0xff);

    /*
     * During reset the nic loaded its initial configuration from an
     * external eeprom. On the ethernut board we do not have any
     * configuration eeprom, but simply tied the eeprom data line to
     * high level. So we have to clear some bits in the configuration
     * register. Switch to register page 3.
     */
    nic_write(NIC_CR, NIC_CR_STP | NIC_CR_RD2 | NIC_CR_PS0 | NIC_CR_PS1);

    /*
     * The nic configuration registers are write protected unless both
     * EEM bits are set to 1.
     */
    nic_write(NIC_PG3_EECR, NIC_EECR_EEM0 | NIC_EECR_EEM1);

    /*
     * Disable sleep and power down.
     */
    nic_write(NIC_PG3_CONFIG3, 0);

    /*
     * Network media had been set to 10Base2 by the virtual EEPROM and
     * will be set now to auto detect. This will initiate a link test.
     * We don't force 10BaseT, because this would disable the link test.
     */
    nic_write(NIC_PG3_CONFIG2, NIC_CONFIG2_BSELB);

    /*
     * Reenable write protection of the nic configuration registers
     * and wait for link test to complete.
     */
    nic_write(NIC_PG3_EECR, 0);
    /*    NutSleep(WAIT500);*/
    Delay_10ms(50);

    /*
     * Switch to register page 0 and set data configuration register
     * to byte-wide DMA transfers, normal operation (no loopback),
     * send command not executed and 8 byte fifo threshold.
     */
    nic_write(NIC_CR, NIC_CR_STP | NIC_CR_RD2);
    nic_write(NIC_PG0_DCR, NIC_DCR_LS | NIC_DCR_FT1);

    /*
     * Clear remote dma byte count register.
     */
    nic_write(NIC_PG0_RBCR0, 0);
    nic_write(NIC_PG0_RBCR1, 0);

    /*
     * Temporarily set receiver to monitor mode and transmitter to
     * internal loopback mode. Incoming packets will not be stored
     * in the nic ring buffer and no data will be send to the network.
     */
    nic_write(NIC_PG0_RCR, NIC_RCR_MON);
    nic_write(NIC_PG0_TCR, NIC_TCR_LB0);

    /*
     * Configure the nic's ring buffer page layout.
     * NIC_PG0_BNRY: Last page read.
     * NIC_PG0_PSTART: First page of receiver buffer.
     * NIC_PG0_PSTOP: Last page of receiver buffer.
     */
    nic_write(NIC_PG0_TPSR, NIC_FIRST_TX_PAGE);
    nic_write(NIC_PG0_BNRY, NIC_STOP_PAGE - 1);
    nic_write(NIC_PG0_PSTART, NIC_FIRST_RX_PAGE);
    nic_write(NIC_PG0_PSTOP, NIC_STOP_PAGE);

    /*
     * Once again clear interrupt status register.
     */
    nic_write(NIC_PG0_ISR, 0xff);

    /*
     * Switch to register page 1 and copy our MAC address into the nic.
     * We are still in stop mode.
     */
    nic_write(NIC_CR, NIC_CR_STP | NIC_CR_RD2 | NIC_CR_PS0);
    for(i = 0; i < 6; i++)
        nic_write(NIC_PG1_PAR0 + i, mac[i]);

    /*
     * Clear multicast filter bits to disable all packets.
     */
    for(i = 0; i < 8; i++)
        nic_write(NIC_PG1_MAR0 + i, 0);

    /*
     * Set current page pointer to one page after the boundary pointer.
     */
    nic_write(NIC_PG1_CURR, NIC_START_PAGE + TX_PAGES);

    /*
     * Switch back to register page 0, remaining in stop mode.
     */
    nic_write(NIC_CR, NIC_CR_STP | NIC_CR_RD2);

    /*
     * Take receiver out of monitor mode and enable it for accepting
     * broadcasts.
     */
    nic_write(NIC_PG0_RCR, NIC_RCR_AB);

    /*
     * Clear all interrupt status flags and enable interrupts.
     */
    nic_write(NIC_PG0_ISR, 0xff);
    nic_write(NIC_PG0_IMR, NIC_IMR_PRXE | NIC_IMR_PTXE | NIC_IMR_RXEE |
                           NIC_IMR_TXEE | NIC_IMR_OVWE);

    /*
     * Fire up the nic by clearing the stop bit and setting the start bit.
     * To activate the local receive dma we must also take the nic out of
     * the local loopback mode.
     */
    nic_write(NIC_CR, NIC_CR_STA | NIC_CR_RD2);
    nic_write(NIC_PG0_TCR, 0);

    /*    NutSleep(WAIT500);*/
    Delay_10ms(50);


#endif /* 0 */

    NicReset();
    
    debug_print(PSTR("Init controller..."));
    nic_write(NIC_PG0_IMR, 0);
    nic_write(NIC_PG0_ISR, 0xff);
    nic_write(NIC_CR, NIC_CR_STP | NIC_CR_RD2 | NIC_CR_PS0 | NIC_CR_PS1);
    nic_write(NIC_PG3_EECR, NIC_EECR_EEM0 | NIC_EECR_EEM1);
    nic_write(NIC_PG3_CONFIG3, 0);
    nic_write(NIC_PG3_CONFIG2, NIC_CONFIG2_BSELB);
    nic_write(NIC_PG3_EECR, 0);
    /*    Delay(50000);*/
    Delay_10ms(200);
    nic_write(NIC_CR, NIC_CR_STP | NIC_CR_RD2);
    nic_write(NIC_PG0_DCR, NIC_DCR_LS | NIC_DCR_FT1);
    nic_write(NIC_PG0_RBCR0, 0);
    nic_write(NIC_PG0_RBCR1, 0);
    nic_write(NIC_PG0_RCR, NIC_RCR_MON);
    nic_write(NIC_PG0_TCR, NIC_TCR_LB0);
    nic_write(NIC_PG0_TPSR, NIC_FIRST_TX_PAGE);
    nic_write(NIC_PG0_BNRY, NIC_STOP_PAGE - 1);
    nic_write(NIC_PG0_PSTART, NIC_FIRST_RX_PAGE);
    nic_write(NIC_PG0_PSTOP, NIC_STOP_PAGE);
    nic_write(NIC_PG0_ISR, 0xff);
    nic_write(NIC_CR, NIC_CR_STP | NIC_CR_RD2 | NIC_CR_PS0);
    for(i = 0; i < 6; i++)
        nic_write(NIC_PG1_PAR0 + i, mac[i]);
    for(i = 0; i < 8; i++)
        nic_write(NIC_PG1_MAR0 + i, 0);
    nic_write(NIC_PG1_CURR, NIC_START_PAGE + TX_PAGES);
    nic_write(NIC_CR, NIC_CR_STP | NIC_CR_RD2);
    nic_write(NIC_PG0_RCR, NIC_RCR_AB);
    nic_write(NIC_PG0_ISR, 0xff);
    nic_write(NIC_PG0_IMR, 0);
    nic_write(NIC_CR, NIC_CR_STA | NIC_CR_RD2);
    nic_write(NIC_PG0_TCR, 0);
    /*    Delay(1000000)*/
    Delay_10ms(200);


        nic_write(NIC_CR, NIC_CR_STA | NIC_CR_RD2 | NIC_CR_PS0 | NIC_CR_PS1);
    rb = nic_read(NIC_PG3_CONFIG0);
    debug_print8(rb);
    switch(rb & 0xC0) {
    case 0x00:
        debug_print(PSTR("RTL8019AS "));
        if(rb & 0x08)
            debug_print(PSTR("jumper mode: "));
        if(rb & 0x20)
            debug_print(PSTR("AUI "));
        if(rb & 0x10)
            debug_print(PSTR("PNP "));
        break;
    case 0xC0:
        debug_print(PSTR("RTL8019 "));
        if(rb & 0x08)
            debug_print(PSTR("jumper mode: "));
        break;
    default:
        debug_print(PSTR("Unknown chip "));
	debug_print8(rb);
        break;
    }
    if(rb & 0x04)
        debug_print(PSTR("BNC\x07 "));
    if(rb & 0x03)
        debug_print(PSTR("Failed\x07 "));

    /*    rb = nic_read(NIC_PG3_CONFIG1);
	  debug_print8(rb);*/
    /*    NutPrintFormat(0, "IRQ%u ", (rb >> 4) & 7);*/
    /*    debug_print("IRQ ");
	  debug_print8((rb >> 4) & 7);*/

    rb = nic_read(NIC_PG3_CONFIG2);
    debug_print8(rb);
    switch(rb & 0xC0) {
    case 0x00:
        debug_print(PSTR("Auto "));
        break;
    case 0x40:
        debug_print(PSTR("10BaseT "));
        break;
    case 0x80:
        debug_print(PSTR("10Base5 "));
        break;
    case 0xC0:
        debug_print(PSTR("10Base2 "));
        break;
    }


    return;
    
  /*  HARD_RESET_RTL8019();*/

  // do soft reset
  writeRTL( ISR, readRTL(ISR) ) ;
  Delay_10ms(5);
  
  writeRTL(CR,0x21);       // stop the NIC, abort DMA, page 0
  Delay_1ms(2);               // make sure nothing is coming in or going out
  writeRTL(DCR, DCR_INIT);    // 0x58
  writeRTL(RBCR0,0x00);
  writeRTL(RBCR1,0x00);
  writeRTL(RCR,0x04);
  writeRTL(TPSR, TXSTART_INIT);
  writeRTL(TCR,0x02);
  writeRTL(PSTART, RXSTART_INIT);
  writeRTL(BNRY, RXSTART_INIT);
  writeRTL(PSTOP, RXSTOP_INIT);
  writeRTL(CR, 0x61);
  Delay_1ms(2);
  writeRTL(CURR, RXSTART_INIT);
  
  writeRTL(PAR0+0, MYMAC_0);
  writeRTL(PAR0+1, MYMAC_1);
  writeRTL(PAR0+2, MYMAC_2);
  writeRTL(PAR0+3, MYMAC_3);
  writeRTL(PAR0+4, MYMAC_4);
  writeRTL(PAR0+5, MYMAC_5);
     	  
  writeRTL(CR,0x21);
  writeRTL(DCR, DCR_INIT);
  writeRTL(CR,0x22);
  writeRTL(ISR,0xFF);
  writeRTL(IMR, IMR_INIT);
  writeRTL(TCR, TCR_INIT);
	
  writeRTL(CR, 0x22);	// start the NIC
}


void processRTL8019Interrupt(void)
{
  volatile unsigned char *base = (unsigned char *)0x8300;
  unsigned char byte = readRTL(ISR);
	
  if( byte & (1<<ISR_OVW) )
    overrun();

}

/*
  unsigned char RTL8019ReceiveEmpty(void)
  {
  unsigned char temp;

  // read CURR from page 1
  writeRTL(CR,0x62);
  temp = readRTL(CURR);
	
  // return to page 0
  writeRTL(CR,0x22);
	
  return ( readRTL(BNRY) == temp );
	
  }*/

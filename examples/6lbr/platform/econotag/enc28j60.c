/*********************************************
 * vim:sw=8:ts=8:si:et
 * To use the above modeline in vim you must have "set modeline" in your .vimrc
 * Author: Guido Socher 
 * Copyright: GPL V2
 *
 * Based on the enc28j60.c file from the AVRlib library by Pascal Stang.
 * For AVRlib See http://www.procyonengineering.com/
 * Used with explicit permission of Pascal Stang.
 *
 * Title: Microchip ENC28J60 Ethernet Interface Driver
 * Chip type           : ATMEGA88 with ENC28J60
 *********************************************/

/* Adapted to Contiki by Maciej Wasilak (wasilak@gmail.com)
 * Ported to mc1322x platform by Laurent Deru (laurent.deru@cetic.be)
 */

#include "spi.h"

#include "enc28j60.h"
#include "eth-drv.h"
//
#define F_CPU 8000000UL         // 8 MHz

#define DEBUG 0
#include "net/uip-debug.h"

static uint8_t Enc28j60Bank;
static uint16_t NextPacketPtr;

#define csactive() { *SPI_SETUP = (*SPI_SETUP & ~SPI_SS_SETUP_MASK) | ( 2 << SPI_SS_SETUP_OFFSET); }
#define cspassive() { *SPI_SETUP = (*SPI_SETUP & ~SPI_SS_SETUP_MASK) | ( 3 << SPI_SS_SETUP_OFFSET); }
#define waitspi() { int clock = clock_time(); while ( (*SPI_CLK_CTRL & SPI_SCK_COUNT_MASK) > 0 && (clock_time() - clock) < 100) ; if (clock_time()-clock >=100) printf("waitspi() timeout\n");}
#define spitransaction(read,write) { \
	*SPI_CLK_CTRL |= (write) << SPI_DATA_LENGTH_OFFSET; \
	*SPI_CLK_CTRL |= ((read) + (write) - 1) << SPI_SCK_COUNT_OFFSET; \
	*SPI_CLK_CTRL |= 1 << SPI_START_OFFSET; \
	waitspi(); \
}

uint32_t
enc28j60ReadOp(uint8_t op, uint8_t address)
{
  //printf("read (%x) %x\r\n", op, address);

  csactive();

  *SPI_RX_DATA = 0;
  *SPI_CLK_CTRL |= 8 << SPI_DATA_LENGTH_OFFSET;
  if(address & 0x80) {
    //We need to discard a dummy byte
    *SPI_CLK_CTRL |= 23 << SPI_SCK_COUNT_OFFSET;
  } else {
    *SPI_CLK_CTRL |= 15 << SPI_SCK_COUNT_OFFSET;
  }
  *SPI_TX_DATA = (uint32_t) (op | (address & ADDR_MASK)) << 24;
  *SPI_CLK_CTRL |= 1 << SPI_START_OFFSET;

  waitspi();

  cspassive();

  if(address & 0x80) {
    //We need to discard a dummy byte
    return *SPI_RX_DATA & 0xFF;
  } else {
    return *SPI_RX_DATA;
  }
}

void
enc28j60WriteOp(uint8_t op, uint8_t address, uint8_t data)
{
  //printf("write (%x) %x <= %x\r\n", op, address, data);

  csactive();
  *SPI_CLK_CTRL |= 16 << SPI_DATA_LENGTH_OFFSET;
  *SPI_CLK_CTRL |= 15 << SPI_SCK_COUNT_OFFSET;
  *SPI_TX_DATA =
    ((uint32_t) (op | (address & ADDR_MASK)) << 24) | (((uint32_t) data) <<
                                                       16);
  *SPI_CLK_CTRL |= 1 << SPI_START_OFFSET;

  waitspi();

  cspassive();
}

void
enc28j60ReadBuffer(uint16_t len, uint8_t * data)
{
#if UIP_CONF_LLH_LEN == 0
  uint8_t header_counter = 0;
#endif
  csactive();
  // issue read command
  *SPI_TX_DATA = ENC28J60_READ_BUF_MEM << 24;
  spitransaction(0, 8);
#if UIP_CONF_LLH_LEN == 0
  header_counter = 0;
  while(header_counter < ETHERNET_LLH_LEN) {
    len--;
    // read data
    spitransaction(8, 0);
    ll_header[header_counter] = *SPI_RX_DATA;
    header_counter++;
  }
  while(len) {
    len--;
    // read data
    spitransaction(8, 0);
    *data = *SPI_RX_DATA;
    data++;
  }
  *data = '\0';
#elif UIP_CONF_LLH_LEN == 14
  while(len) {
    len--;
    // read data
    spitransaction(8, 0);
    *data = *SPI_RX_DATA;
    data++;
  }
  *data = '\0';
#else
#error "UIP_CONF_LLH_LEN value neither 0 nor 14."
#endif
  cspassive();
}

void
enc28j60WriteBuffer(uint16_t len, uint8_t * data)
{
#if UIP_CONF_LLH_LEN == 0
  uint8_t header_counter = 0;
#endif
  csactive();
  // issue write command
  *SPI_TX_DATA = ENC28J60_WRITE_BUF_MEM << 24;
  spitransaction(0, 8);
#if UIP_CONF_LLH_LEN == 0
  header_counter = 0;
  while(header_counter < ETHERNET_LLH_LEN) {
    len--;
    // write data
    *SPI_TX_DATA = ((uint32_t) ll_header[header_counter]) << 24;
    header_counter++;
    spitransaction(0, 8);
  }
  while(len) {
    len--;
    // write data
    *SPI_TX_DATA = ((uint32_t) * data) << 24;
    data++;
    spitransaction(0, 8);
  }
#elif UIP_CONF_LLH_LEN == 14
  while(len) {
    len--;
    // write data
    *SPI_TX_DATA = ((uint32_t) * data) << 24;
    data++;
    spitransaction(0, 8);
  }
#else
#error "UIP_CONF_LLH_LEN value neither 0 nor 14."
#endif
  cspassive();
}

void
enc28j60SetBank(uint8_t address)
{
  // set the bank (if needed)
  if((address & BANK_MASK) != Enc28j60Bank) {
    // set the bank
    enc28j60WriteOp(ENC28J60_BIT_FIELD_CLR, ECON1,
                    (ECON1_BSEL1 | ECON1_BSEL0));
    enc28j60WriteOp(ENC28J60_BIT_FIELD_SET, ECON1,
                    (address & BANK_MASK) >> 5);
    Enc28j60Bank = (address & BANK_MASK);
  }
}

uint32_t
enc28j60Read(uint8_t address)
{
  // set the bank
  enc28j60SetBank(address);
  // do the read
  return enc28j60ReadOp(ENC28J60_READ_CTRL_REG, address);
}

void
enc28j60Write(uint8_t address, uint8_t data)
{
  // set the bank
  enc28j60SetBank(address);
  // do the write
  enc28j60WriteOp(ENC28J60_WRITE_CTRL_REG, address, data);
}

uint16_t
enc28j60PhyRead(uint8_t address)
{
  uint16_t data;

  enc28j60Write(MIREGADR, address);
  enc28j60Write(MICMD, enc28j60Read(MICMD) | MICMD_MIIRD);


  // wait until the PHY write completes
  while(enc28j60Read(MISTAT) & MISTAT_BUSY) {
    //_delay_us(15);
    clock_delay(15);
  }

  enc28j60Write(MICMD, enc28j60Read(MICMD) & ~MICMD_MIIRD);
  data = (uint16_t) enc28j60Read(MIRDL);
  data |= ((uint16_t) enc28j60Read(MIRDH) << 8);

  //DPRINTF("read phy reg %u:%u\r\n", phyreg, data);
  return data;
}

void
enc28j60PhyWrite(uint8_t address, uint16_t data)
{
  // set the PHY register address
  enc28j60Write(MIREGADR, address);
  // write the PHY data
  enc28j60Write(MIWRL, data);
  enc28j60Write(MIWRH, data >> 8);
  // wait until the PHY write completes
  uint8_t status;

  while((status = enc28j60Read(MISTAT)) & MISTAT_BUSY) {
    //_delay_us(15);
    clock_delay(15);
  }
}


void
enc28j60Init(uint8_t * macaddr)
{
  unsigned int i = 0;

  //Setup SPI frequency and mode
  *SPI_SETUP = (*SPI_SETUP & ~SPI_SS_SETUP_MASK) | (1 << SPI_SS_SETUP_OFFSET);
  *SPI_SETUP = (*SPI_SETUP & ~SPI_SCK_FREQ_MASK) | (1 << SPI_SCK_FREQ_OFFSET);
  //Enable SPI I/O
  gpio_select_function(4, 1);
  gpio_select_function(5, 1);
  gpio_select_function(6, 1);
  gpio_select_function(7, 1);

  // perform system reset
  clock_delay(50);
  enc28j60WriteOp(ENC28J60_SOFT_RESET, 0, ENC28J60_SOFT_RESET);
  clock_delay(50);
  //  while((i = enc28j60getrev()) != 6) {
  //    printf("Invalid rev : %u\n", i);
  //    clock_delay(50);
  //    enc28j60WriteOp(ENC28J60_SOFT_RESET, 0, ENC28J60_SOFT_RESET);
  //  }

  // check CLKRDY bit to see if reset is complete
  // The CLKRDY does not work. See Rev. B4 Silicon Errata point. Just wait.
  //while(!(enc28j60Read(ESTAT) & ESTAT_CLKRDY));
  // do bank 0 stuff
  // initialize receive buffer
  // 16-bit transfers, must write low byte first
  // set receive buffer start address
  NextPacketPtr = RXSTART_INIT;
  // Rx start
  enc28j60Write(ERXSTL, RXSTART_INIT & 0xFF);
  enc28j60Write(ERXSTH, RXSTART_INIT >> 8);
  // set receive pointer address
  enc28j60Write(ERXRDPTL, RXSTART_INIT & 0xFF);
  enc28j60Write(ERXRDPTH, RXSTART_INIT >> 8);
  // RX end
  enc28j60Write(ERXNDL, RXSTOP_INIT & 0xFF);
  enc28j60Write(ERXNDH, RXSTOP_INIT >> 8);
  // TX start
  enc28j60Write(ETXSTL, TXSTART_INIT & 0xFF);
  enc28j60Write(ETXSTH, TXSTART_INIT >> 8);
  // TX end
  enc28j60Write(ETXNDL, TXSTOP_INIT & 0xFF);
  enc28j60Write(ETXNDH, TXSTOP_INIT >> 8);
  // do bank 1 stuff, packet filter:
#if CETIC_6LBR_TRANSPARENTBRIDGE
  //Enter Promiscuous mode (capture all packets)
  enc28j60Write(ERXFCON, 0);
#else
  enc28j60Write(ERXFCON, ERXFCON_UCEN | ERXFCON_MCEN | ERXFCON_BCEN);
#endif
  // do bank 2 stuff
  // enable MAC receive
  enc28j60Write(MACON1, MACON1_MARXEN | MACON1_TXPAUS | MACON1_RXPAUS);
  // bring MAC out of reset
  enc28j60Write(MACON2, 0x00);
  // enable automatic padding to 60bytes and CRC operations
  enc28j60WriteOp(ENC28J60_BIT_FIELD_SET, MACON3,
                  MACON3_PADCFG0 | MACON3_TXCRCEN | MACON3_FRMLNEN);
  // set inter-frame gap (non-back-to-back)
  enc28j60Write(MAIPGL, 0x12);
  enc28j60Write(MAIPGH, 0x0C);
  // set inter-frame gap (back-to-back)
  enc28j60Write(MABBIPG, 0x12);
  // Set the maximum packet size which the controller will accept
  // Do not send packets longer than MAX_FRAMELEN:
  enc28j60Write(MAMXFLL, MAX_FRAMELEN & 0xFF);
  enc28j60Write(MAMXFLH, MAX_FRAMELEN >> 8);
  // do bank 3 stuff
  // write MAC address
  // NOTE: MAC address in ENC28J60 is byte-backward
  enc28j60Write(MAADR5, macaddr[0]);
  enc28j60Write(MAADR4, macaddr[1]);
  enc28j60Write(MAADR3, macaddr[2]);
  enc28j60Write(MAADR2, macaddr[3]);
  enc28j60Write(MAADR1, macaddr[4]);
  enc28j60Write(MAADR0, macaddr[5]);
  // no loopback of transmitted frames
  enc28j60PhyWrite(PHCON2, PHCON2_HDLDIS);
  // switch to bank 0
  enc28j60SetBank(ECON1);
  // enable interrutps
  //enc28j60WriteOp(ENC28J60_BIT_FIELD_SET, EIE, EIE_INTIE|EIE_PKTIE);
  // enable packet reception
  enc28j60WriteOp(ENC28J60_BIT_FIELD_SET, ECON1, ECON1_RXEN);
  clock_delay(10);
  enc28j60PhyWrite(PHLCON, 0x476);
  clock_delay(20);
  printf("ENC28 REV %u\n", enc28j60getrev());
/*
    //Debug code
    enc28j60PhyWrite(PHLCON,0x3880);//0x476);
    printf("MAC0 %x\r\n", enc28j60Read(MAADR5));
    printf("MICMD %x\r\n", enc28j60Read(MICMD));
    printf("MAC5 %x\r\n", enc28j60Read(MAADR0));
    printf("MAC1 %x\r\n", enc28j60Read(MAADR4));
    printf("PHY: %x\r\n", enc28j60PhyRead(PHLCON));
*/
}

// read the revision of the chip:
uint8_t
enc28j60getrev(void)
{
  return (enc28j60Read(EREVID));
}

void
enc28j60PacketSend(uint16_t len, uint8_t * packet)
{
  //if((enc28j60PhyRead(PHSTAT1) & PHSTAT1_LLSTAT)==0)
  //{
  //      PRINTF("ERROR: Ethernet down\n");
  //}
  // Set the write pointer to start of transmit buffer area
  enc28j60Write(EWRPTL, TXSTART_INIT & 0xFF);
  enc28j60Write(EWRPTH, TXSTART_INIT >> 8);
  // Set the TXND pointer to correspond to the packet size given
  enc28j60Write(ETXNDL, (TXSTART_INIT + len) & 0xFF);
  enc28j60Write(ETXNDH, (TXSTART_INIT + len) >> 8);
  // write per-packet control byte (0x00 means use macon3 settings)
  enc28j60WriteOp(ENC28J60_WRITE_BUF_MEM, 0, 0x00);
  // copy the packet into the transmit buffer
  enc28j60WriteBuffer(len, packet);
  // send the contents of the transmit buffer onto the network
  enc28j60WriteOp(ENC28J60_BIT_FIELD_SET, ECON1, ECON1_TXRTS);
  // Reset the transmit logic problem. See Rev. B4 Silicon Errata point 12.
  if((enc28j60Read(EIR) & EIR_TXERIF)) {
    enc28j60WriteOp(ENC28J60_BIT_FIELD_CLR, ECON1, ECON1_TXRTS);
  }
}

// Gets a packet from the network receive buffer, if one is available.
// The packet will by headed by an ethernet header.
//      maxlen  The maximum acceptable length of a retrieved packet.
//      packet  Pointer where packet data should be stored.
// Returns: Packet length in bytes if a packet was retrieved, zero otherwise.
uint16_t
enc28j60PacketReceive(uint16_t maxlen, uint8_t * packet)
{
  uint16_t rxstat;
  uint16_t len;

  // check if a packet has been received and buffered
  //if( !(enc28j60Read(EIR) & EIR_PKTIF) ){
  // The above does not work. See Rev. B4 Silicon Errata point 6.
  if(enc28j60Read(EPKTCNT) == 0) {
    return (0);
  }
  // Set the read pointer to the start of the received packet
  enc28j60Write(ERDPTL, (NextPacketPtr));
  enc28j60Write(ERDPTH, (NextPacketPtr) >> 8);
  // read the next packet pointer
  NextPacketPtr = enc28j60ReadOp(ENC28J60_READ_BUF_MEM, 0);
  NextPacketPtr |= enc28j60ReadOp(ENC28J60_READ_BUF_MEM, 0) << 8;
  // read the packet length (see datasheet page 43)
  len = enc28j60ReadOp(ENC28J60_READ_BUF_MEM, 0);
  len |= enc28j60ReadOp(ENC28J60_READ_BUF_MEM, 0) << 8;
  len -= 4;                     //remove the CRC count
  // read the receive status (see datasheet page 43)
  rxstat = enc28j60ReadOp(ENC28J60_READ_BUF_MEM, 0);
  rxstat |= enc28j60ReadOp(ENC28J60_READ_BUF_MEM, 0) << 8;
  // limit retrieve length
  if(len > maxlen - 1) {
    len = maxlen - 1;
  }
  // check CRC and symbol errors (see datasheet page 44, table 7-3):
  // The ERXFCON.CRCEN is set by default. Normally we should not
  // need to check this.
  if((rxstat & 0x80) == 0) {
    // invalid
    len = 0;
  } else {
    // copy the packet from the receive buffer
    enc28j60ReadBuffer(len, packet);
  }
  // Move the RX read pointer to the start of the next received packet
  // This frees the memory we just read out
  enc28j60Write(ERXRDPTL, (NextPacketPtr));
  enc28j60Write(ERXRDPTH, (NextPacketPtr) >> 8);
  // decrement the packet counter indicate we are done with this packet
  enc28j60WriteOp(ENC28J60_BIT_FIELD_SET, ECON2, ECON2_PKTDEC);
#if UIP_CONF_LLH_LEN == 0
  return (len - ETHERNET_LLH_LEN);
#elif UIP_CONF_LLH_LEN == 14
  return (len);
#endif
}

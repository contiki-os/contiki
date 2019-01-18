#include <AT91SAM7S64.h>
#include <stdint.h>
#include <dev/spi.h>
#include <sam7s-spi.h>

/* Prevents interrupts using SPI at inappropriate times */
unsigned char spi_busy = 0;

#define SPI_SPEED 1000000  /* 1MHz clock*/
#define SPI_DLYBCT 1
#define SPI_DLYBS 20

#define SPI_TRANSFER (AT91C_PA12_MISO | AT91C_PA13_MOSI | AT91C_PA14_SPCK)

#define SPI_CS (AT91C_PA11_NPCS0 | AT91C_PA31_NPCS1)

void
spi_init()
{
  static uint8_t initialised = 0;
  if (!initialised) {
    *AT91C_SPI_CR = AT91C_SPI_SPIDIS | AT91C_SPI_SWRST;
    *AT91C_PMC_PCER = (1 << AT91C_ID_SPI);
    *AT91C_PIOA_ASR = SPI_TRANSFER | SPI_CS;
    *AT91C_PIOA_PDR = SPI_TRANSFER | SPI_CS;
    *AT91C_PIOA_PPUER = AT91C_PA12_MISO | SPI_CS;
    *AT91C_SPI_MR = (AT91C_SPI_MSTR | AT91C_SPI_PS_FIXED
		     | AT91C_SPI_MODFDIS | AT91C_SPI_PCS);

    /* It seems necessary to set the clock speed for chip select 0
       even if it's not used. */
    AT91C_SPI_CSR[0] = (MCK/SPI_SPEED)<<8;

    *AT91C_SPI_CR = AT91C_SPI_SPIEN;
    initialised = 1;
  }
}

void
spi_init_chip_select(unsigned int chip, unsigned int speed,
		     unsigned int dlybct,
		     unsigned int dlybs, unsigned int phase,
		     unsigned int polarity)
{
  spi_init();

  AT91C_SPI_CSR[chip] =
    ((dlybct<<24) | (dlybs<<16) | (((MCK+speed/2)/speed)<<8)
     | (phase?AT91C_SPI_NCPHA:0) | (polarity?AT91C_SPI_CPOL:0)
     | AT91C_SPI_BITS_8 | AT91C_SPI_CSAAT);
}

#if 0
#define DBG_SEND dbg_blocking_putchar('>');
#define DBG_RECV dbg_blocking_putchar('<');
#else
#define DBG_SEND
#define DBG_RECV
#endif

void
spi_transfer(unsigned int chip, const struct spi_block *block, unsigned int blocks)
{
  spi_busy = 1;
  while(!(*AT91C_SPI_SR & AT91C_SPI_TXEMPTY)); /* wait unti previous transfer is done */
  
  /* Clear any data left in the receiver */
  (void)*AT91C_SPI_RDR;
  (void)*AT91C_SPI_RDR;

  /* Select chip */
  *AT91C_SPI_MR = ((*AT91C_SPI_MR & ~AT91C_SPI_PCS)
		   | ((~(1<<chip) & 0x0f) << 16));
  
  while(blocks-- > 0) {
    struct spi_block current = *block++;
    if (current.send) {
      if (current.receive) {
	/* Send and receive */
	while(current.len-- > 0) {
	  while(!(*AT91C_SPI_SR & AT91C_SPI_TDRE));
	  *AT91C_SPI_TDR = *current.send++;
	  DBG_SEND;
	  while(!(*AT91C_SPI_SR & AT91C_SPI_RDRF));
	  *current.receive++ = *AT91C_SPI_RDR;
	  DBG_RECV;
	}
      } else {
	/* Send only */
	while(current.len-- > 0) {
	  while(!(*AT91C_SPI_SR & AT91C_SPI_TDRE));
	  *AT91C_SPI_TDR = *current.send++;
	  DBG_SEND;
	  while(!(*AT91C_SPI_SR & AT91C_SPI_RDRF));
	  (void)*AT91C_SPI_RDR;
	  DBG_RECV;
	}
      }
    } else {
      if (current.receive) {
	/* Receive only */
	while(current.len-- > 0) {
	  while(!(*AT91C_SPI_SR & AT91C_SPI_TDRE));
	  *AT91C_SPI_TDR = 0;
	  DBG_SEND;
	  while(!(*AT91C_SPI_SR & AT91C_SPI_RDRF));
	  *current.receive++ = *AT91C_SPI_RDR;
	  DBG_RECV;
	}
      } else {
	/* Clock only */
	while(current.len-- > 0) {
	  while(!(*AT91C_SPI_SR & AT91C_SPI_TDRE));
	  *AT91C_SPI_TDR = 0;
	  DBG_SEND;
	  while(!(*AT91C_SPI_SR & AT91C_SPI_RDRF));
	  (void)*AT91C_SPI_RDR;
	  DBG_RECV;
	}
      }
    } 
  }
  *AT91C_SPI_CR = AT91C_SPI_LASTXFER;
    
  spi_busy = 0;
}

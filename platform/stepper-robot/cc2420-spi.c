#include <AT91SAM7S64.h>
#include <net/uip.h>
#include <dev/cc2420.h>
#include <sam7s-spi.h>

#define SPI_SPEED 1000000  /* 1MHz clock*/
#define SPI_DLYBCT 1
#define SPI_DLYBS 20


#ifndef BV
#define BV(b) (1<<(b))
#endif

void
cc2420_arch_init(void)
{
  spi_init();

  AT91C_SPI_CSR[CC2420_DEFAULT_DEV] =
    ((SPI_DLYBCT<<24) | (SPI_DLYBS<<16) | (((MCK+SPI_SPEED/2)/SPI_SPEED)<<8)
		      | AT91C_SPI_NCPHA | AT91C_SPI_BITS_8 | AT91C_SPI_CSAAT);
}

#ifndef __CORE_SPI_H__KBMMOKI6CG__
#define __CORE_SPI_H__KBMMOKI6CG__
#include <stdint.h>

void
spi_init();

#define SPI_POLARITY_INACTIVE_HIGH 1
#define SPI_POLARITY_INACTIVE_LOW 1

#define SPI_PHASE_CHANGE_CAPTURE 0
#define SPI_PHASE_CAPTURE_CHANGE 1

struct spi_block {
  const uint8_t *send;	/* NULL for receive only */
  uint8_t *receive;	/* NULL for send only */
  uint16_t len;		/* transfer length, non-zero */
};

void
spi_transfer(unsigned int chip, const struct spi_block *block, unsigned int blocks);

void
spi_init_chip_select(unsigned int chip, unsigned int speed,
		     unsigned int dlybct,
		     unsigned int dlybs, unsigned int phase,
		     unsigned int polarity);

#endif /* __CORE_SPI_H__KBMMOKI6CG__ */

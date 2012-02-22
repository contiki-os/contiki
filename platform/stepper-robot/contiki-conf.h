#ifndef __CONTIKI_CONF_H__CDBB4VIH3I__
#define __CONTIKI_CONF_H__CDBB4VIH3I__

#include <stdint.h>
#include <cc2420-interrupt.h>
#include <AT91SAM7S64.h>
#include <io.h>

#define CCIF
#define CLIF

#define WITH_UIP 1
#define WITH_ASCII 1

#define CLOCK_CONF_SECOND 100

/* These names are deprecated, use C99 names. */
typedef uint8_t u8_t;
typedef uint16_t u16_t;
typedef uint32_t u32_t;
typedef int8_t s8_t;
typedef int16_t s16_t;
typedef int32_t s32_t;

typedef unsigned int clock_time_t;
typedef unsigned int uip_stats_t;

#ifndef BV
#define BV(x) (1<<(x))
#endif

/* SPI */
#define SPI_TXBUF *AT91C_SPI_TDR

#define SPI_RXBUF ((unsigned char)*AT91C_SPI_RDR)

#define SPI_WAITFOREOTx() while ((*AT91C_SPI_SR & AT91C_SPI_TXEMPTY) == 0)

#define	SPI_WAITFOREORx() while ((*AT91C_SPI_SR & AT91C_SPI_RDRF) == 0)

/* CC2420 control pins */
/* LOOP count for waiting 20 symbols in the CC2420 code - same as MSP? */
#define CC2420_CONF_SYMBOL_LOOP_COUNT 800


#define FIFO_IS_1       (*AT91C_PIOA_PDSR & AT91C_PIO_PA2)
#define VREG_IS_1       1 /* Hardwired */
#define FIFOP_IS_1      (*AT91C_PIOA_PDSR & AT91C_PIO_PA30)
#define SFD_IS_1        (*AT91C_PIOA_PDSR & AT91C_PIO_PA15)
#define CCA_IS_1        1

#define SET_RESET_INACTIVE()	setreg(CC2420_MAIN, 0xf800);
#define SET_RESET_ACTIVE()	setreg(CC2420_MAIN, 0x0000);

#define SET_VREG_ACTIVE()
#define SET_VREG_INACTIVE()

#define FIFOP_INT_INIT() cc2420_interrupt_fifop_int_init()
#define DISABLE_FIFOP_INT() (*AT91C_AIC_IDCR = (1 << AT91C_ID_IRQ1))
#define ENABLE_FIFOP_INT() (*AT91C_AIC_IECR = (1 << AT91C_ID_IRQ1))

#define CC2420_DEFAULT_DEV 1

#define SPI_ENABLE() \
do { \
*AT91C_SPI_MR = ((*AT91C_SPI_MR & ~AT91C_SPI_PCS) \
		 | ((~(1<<CC2420_DEFAULT_DEV) & 0x0f) << 16));\
} while(0)

/* Automatically enabled when transmitting */

#define SPI_DISABLE()	(*AT91C_SPI_CR = AT91C_SPI_LASTXFER)

#define CLEAR_FIFOP_INT() (*AT91C_AIC_ICCR = (1 << AT91C_ID_IRQ1))

#define LOOP_20_SYMBOLS (MCK/28125) /* Assume the loop takes 9 cycles */

/* uIP configuration */
#define UIP_CONF_LLH_LEN         0
#define UIP_CONF_BROADCAST       1
#define UIP_CONF_LOGGING 1
#define UIP_CONF_BUFFER_SIZE 116

#define UIP_CONF_TCP_FORWARD 1

/* USB definitions */
#define USB_EP1_SIZE 64
#define USB_EP2_SIZE 64
#define USB_EP3_SIZE 64

#define USB_RBC_NUM_BLOCKS 16
/* Prefix for relocation sections in ELF files */
#define REL_SECT_PREFIX ".rel"

#define CC_BYTE_ALIGNED __attribute__ ((packed, aligned(1)))


#endif /* __CONTIKI_CONF_H__CDBB4VIH3I__ */

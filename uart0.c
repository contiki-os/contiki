#include "uart0.h"
#include <iodefine.h>
#include <iodefine_ext.h>

void uart0_init(void)
{
    /* Reference R01AN0459EJ0100 or hardware manual for details */
    PIOR.pior = 0U;                                         /* Disable IO redirection */
    PM1.pm1 |= 0x06U;                                       /* Set P11 and P12 as inputs */
    SAU0EN = 1;                                             /* Supply clock to serial array unit 0 */
    SPS0.sps0 = 0x44U;                                      /* Set input clock (CK00 and CK01) to fclk/16 = 2MHz */
    ST0.st0 = 0x03U;                                        /* Stop operation of channel 0 and 1 */
    /* Setup interrupts (disable) */
    STMK0 = 1;                                              /* Disable INTST0 interrupt */
    STIF0 = 0;                                              /* Clear INTST0 interrupt request flag */
    STPR10 = 1;                                             /* Set INTST0 priority: lowest  */
    STPR00 = 1;
    SRMK0 = 1;                                              /* Disable INTSR0 interrupt */
    SRIF0 = 0;                                              /* Clear INTSR0 interrupt request flag */
    SRPR10 = 1;                                             /* Set INTSR0 priority: lowest */
    SRPR00 = 1;
    SREMK0 = 1;                                             /* Disable INTSRE0 interrupt */
    SREIF0 = 0;                                             /* Clear INTSRE0 interrupt request flag */
    SREPR10 = 1;                                            /* Set INTSRE0 priority: lowest */
    SREPR00 = 1;
    /* Setup operation mode for transmitter (channel 0) */
    SMR00.smr00 = 0x0023U;                                  /* Operation clock : CK00,
                                                               Transfer clock : division of CK00
                                                               Start trigger : software
                                                               Detect falling edge as start bit
                                                               Operation mode : UART
                                                               Interrupt source : buffer empty
                                                            */
    SCR00.scr00 = 0x8097U;                                  /* Transmission only
                                                               Reception error interrupt masked
                                                               Phase clock : type 1
                                                               No parity
                                                               LSB first
                                                               1 stop bit
                                                               8-bit data length
                                                            */
    SDR00.sdr00 = 0xCE00U;                                  /* transfer clock : operation clock divided by 208
                                                               2 MHz / 208 = ~9600 bps
                                                            */
    /* Setup operation mode for receiver (channel 1) */
    NFEN0.nfen0 |= 1;                                       /* Enable noise filter on RxD0 pin */
    SIR01.sir01 = 0x0007U;                                  /* Clear error flags */
    SMR01.smr01 = 0x0122U;                                  /* Operation clock : CK00
                                                               Transfer clock : division of CK00
                                                               Start trigger : valid edge on RxD pin
                                                               Detect falling edge as start bit
                                                               Operation mode : UART
                                                               Interrupt source : transfer end
                                                            */
    SCR01.scr01 = 0x4097U;                                  /* Reception only
                                                               Reception error interrupt masked
                                                               Phase clock : type 1
                                                               No parity
                                                               LSB first
                                                               1 stop bit
                                                               8-bit data length
                                                            */
    SDR01.sdr01 = 0xCE00U;                                  /* transfer clock : operation clock divided by 208
                                                               2 MHz / 208 = ~9600 bps
                                                            */
    SO0.so0 |= 1;                                           /* Prepare for use of channel 0 */
    SOE0.soe0 |= 1;
    P1.p1 |= (1 << 2);                                      /* Set TxD0 high */
    PM1.pm1 &= ~(1 << 2);                                   /* Set output mode for TxD0 */
    PM1.pm1 |= (1 << 1);                                    /* Set input mode for RxD0 */
    SS0.ss0 |= 0x03U;                                       /* Enable UART0 operation (both channels) */
    STIF0 = 1;                                              /* Set buffer empty interrupt request flag */
}

int uart0_puts(const char * s)
{
    int len = 0;
    SMR00.smr00 |= 0x0001U;                                 /* Set buffer empty interrupt */
    while ('\0' != *s)
    {
        while (0 == STIF0);
        STIF0 = 0;
        SDR00.sdr00 = *s++;
        ++len;
    }
#if 0
    while (0 == STIF0);
    STIF0 = 0;
    SDR00.sdr00 = '\r';
#endif
    while (0 == STIF0);
    STIF0 = 0;
    SMR00.smr00 &= ~0x0001U;
    SDR00.sdr00 = '\n';
    while (0 == STIF0);
#if 0
    while (0 != SSR00.BIT.bit6);                            /* Wait until TSF00 == 0 */
#endif
    return len;
}

__attribute__((interrupt))
void st0_handler(void)
{
}

__attribute__((interrupt))
void sr0_handler(void)
{
}

/* This is actually INTSRE0 interrupt handler */
__attribute__((interrupt))
void tm01h_handler(void)
{
}

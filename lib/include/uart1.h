#ifndef UART1_H
#define UART1_H

#include <types.h>

#define UCON      (0)
/* UCON bits */
#define UCON_SAMP     10
#define UCON_SAMP_8X   0
#define UCON_SAMP_16X  1

#define USTAT     (0x04)
#define UDATA     (0x08)
#define URXCON    (0x0c)
#define UTXCON    (0x10)
#define UCTS      (0x14)
#define UBRCNT    (0x18)

#define UART1_BASE (0x80005000)
#define UART2_BASE (0x8000b000)

#define UART1_UCON       ((volatile uint32_t *) ( UART1_BASE + UCON   ))
#define UART1_USTAT      ((volatile uint32_t *) ( UART1_BASE + USTAT  ))
#define UART1_UDATA      ((volatile uint32_t *) ( UART1_BASE + UDATA  ))
#define UART1_URXCON     ((volatile uint32_t *) ( UART1_BASE + URXCON ))
#define UART1_UTXCON     ((volatile uint32_t *) ( UART1_BASE + UTXCON ))
#define UART1_UCTS       ((volatile uint32_t *) ( UART1_BASE + UCTS   ))
#define UART1_UBRCNT     ((volatile uint32_t *) ( UART1_BASE + UBRCNT ))

#define UART2_UCON       ((volatile uint32_t *) ( UART2_BASE + UCON   ))
#define UART2_USTAT      ((volatile uint32_t *) ( UART2_BASE + USTAT  ))
#define UART2_UDATA      ((volatile uint32_t *) ( UART2_BASE + UDATA  ))
#define UART2_URXCON     ((volatile uint32_t *) ( UART2_BASE + URXCON ))
#define UART2_UTXCON     ((volatile uint32_t *) ( UART2_BASE + UTXCON ))
#define UART2_UCTS       ((volatile uint32_t *) ( UART2_BASE + UCTS   ))
#define UART2_UBRCNT     ((volatile uint32_t *) ( UART2_BASE + UBRCNT ))

extern volatile uint32_t  u1_head, u1_tail;
void uart1_putc(char c);
#define uart1_can_get() (*UART1_URXCON > 0)
uint8_t uart1_getc(void);



#endif

#ifndef UART1_H
#define UART1_H

#define UART1_CON       ((volatile uint32_t *) 0x80005000)
#define UART1_STAT      ((volatile uint32_t *) 0x80005004)
#define UART1_DATA      ((volatile uint32_t *) 0x80005008)
#define UR1CON          ((volatile uint32_t *) 0x8000500c)
#define UT1CON          ((volatile uint32_t *) 0x80005010)
#define UART1_CTS       ((volatile uint32_t *) 0x80005014)
#define UART1_BR        ((volatile uint32_t *) 0x80005018)

int uart1_putchar(int c);

#define uart1_can_get() (*UR1CON > 0)

#endif

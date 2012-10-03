#include "QB-R5F100SL-TB.h"
#include "uart0.h"

volatile unsigned char ticks = 0;
volatile unsigned char flag_1hz = 0;

__attribute__((interrupt))
void wdti_handler(void)
{
}

__attribute__((interrupt))
void it_handler(void)
{
    ++ticks;
    LED1 ^= 1;
    if (0 == (0x07 & ticks))
    {
        flag_1hz = 1;
    }
}

int main(void)
{
    asm("di");
    /* Setup LEDs */
    LED1 = 1;
    LED2 = 1;
    LED1_PIN = 0;
    LED2_PIN = 0;
    /* Setup clocks */
    CMC.cmc = 0x11U;                                        /* Enable XT1, disable X1 */
    CSC.csc = 0x80U;                                        /* Start XT1 and HOCO, stop X1 */
    CKC.ckc = 0x00U;
    /* Delay 1 second */
    register unsigned long int i;
    for (i = 0x000FFFFFUL; i; --i)
        asm("nop");
    OSMC.osmc = 0x00;                                       /* Supply fsub to peripherals, including Interval Timer */
    uart0_init();
    /* Setup 12-bit interval timer */
    RTCEN = 1;                                              /* Enable 12-bit interval timer and RTC */
    ITMK = 1;                                               /* Disable IT interrupt */
    ITPR0 = 0;                                              /* Set interrupt priority - highest */
    ITPR1 = 0;
    ITMC.itmc = 0x8FFFU;                                    /* Set maximum period 4096/32768Hz = 1/8 s, and start timer */
    ITIF = 0;                                               /* Clear interrupt request flag */
    ITMK = 0;                                               /* Enable IT interrupt */
    asm ("ei");                                             /* Enable interrupts */
    for(;;)
    {
        if (flag_1hz)
        {
            LED2 = 0;
            flag_1hz = 0;
            const char msg[] = "Hello, RL78! [:";
            uart0_puts(msg);
            LED2 = 1;
        }
        asm("halt");
    }
}

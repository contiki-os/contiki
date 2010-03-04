#ifndef ISR_H
#define ISR_H

#define INTBASE        (0x80020000)
#define INTCNTL_OFF           (0x0)
#define INTENNUM_OFF          (0x8)
#define INTDISNUM_OFF         (0xC)
#define INTENABLE_OFF        (0x10)
#define INTSRC_OFF           (0x30)
#define NIPEND_OFF           (0x38)

#define INTCNTL      ((volatile uint32_t *) (INTBASE + INTCNTL_OFF))
#define INTENNUM     ((volatile uint32_t *) (INTBASE + INTENNUM_OFF))
#define INTDISNUM    ((volatile uint32_t *) (INTBASE + INTDISNUM_OFF))
#define INTENABLE    ((volatile uint32_t *) (INTBASE + INTENABLE_OFF))
#define INTSRC       ((volatile uint32_t *) (INTBASE + INTSRC_OFF))
#define NIPEND       ((volatile uint32_t *) (INTBASE + NIPEND_OFF))

enum interrupt_nums {
	INT_NUM_ASM = 0,
	INT_NUM_UART1,
	INT_NUM_UART2,
	INT_NUM_CRM,
	INT_NUM_I2C,
	INT_NUM_TMR,
	INT_NUM_SPIF,
	INT_NUM_MACA,
	INT_NUM_SSI,
	INT_NUM_ADC,
	INT_NUM_SPI,
};

#define global_irq_disable() (set_bit(*INTCNTL,20))
#define global_irq_enable()  (clear_bit(*INTCNTL,20))

#define enable_irq(irq)  (*INTENNUM  = INT_NUM_##irq)
#define disable_irq(irq) (*INTDISNUM = INT_NUM_##irq)


extern void tmr0_isr(void) __attribute__((weak));
extern void tmr1_isr(void) __attribute__((weak));
extern void tmr2_isr(void) __attribute__((weak));
extern void tmr3_isr(void) __attribute__((weak));

extern void rtc_isr(void) __attribute__((weak));
extern void kbi4_isr(void) __attribute__((weak));
extern void kbi5_isr(void) __attribute__((weak));
extern void kbi6_isr(void) __attribute__((weak));
extern void kbi7_isr(void) __attribute__((weak));

extern void uart1_isr(void) __attribute__((weak));

extern void maca_isr(void) __attribute__((weak));


#endif


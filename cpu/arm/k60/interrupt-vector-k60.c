/*
 * Interrupt vector for K60 MCU.
 *
 * Note: It is not necessary to modify this file to define custom interrupt
 * service routines. All symbols are defined weak, it is only necessary to
 * define a function with the same name in another file to override the default
 * interrupt handlers.
 */

#define SECTION(x) __attribute__ ((section(#x)))
#define ISR_VECTOR_SECTION SECTION(.vector_table)
void reset_handler(void) __attribute__((naked));
void _isr_nmi(void) __attribute__((interrupt));
void _isr_hardfault(void) __attribute__((interrupt));
void _isr_memmanage(void) __attribute__((interrupt));
void _isr_busfault(void) __attribute__((interrupt));
void _isr_usagefault(void) __attribute__((interrupt));


/* Default handler for interrupts, infinite loop */
static void unhandled_interrupt(void) __attribute__((interrupt));

#define UNHANDLED_ALIAS __attribute__((weak, alias("unhandled_interrupt")));

/* __attribute__((naked)) in order to not add any function prologue to the
 * default hardfault handler written in asm */
static void dHardFault_handler(void) __attribute__((naked));

/* ARM Cortex defined interrupt vectors */
void reset_handler(void) __attribute__((naked));
void _isr_nmi(void) __attribute__((interrupt));
void _isr_hardfault(void) __attribute__((weak, alias("dHardFault_handler")));
void _isr_memmanage(void) __attribute__((weak, alias("dMemManage_handler")));
void _isr_busfault(void) __attribute__((weak, alias("dBusFault_handler")));
void _isr_usagefault(void) __attribute__((weak, alias("dUsageFault_handler")));
void _isr_reserved(void) UNHANDLED_ALIAS;
/* void _isr_reserved(void) UNHANDLED_ALIAS; */
/* void _isr_reserved(void) UNHANDLED_ALIAS; */
/* void _isr_reserved(void) UNHANDLED_ALIAS; */
void _isr_svcall(void) UNHANDLED_ALIAS;
void _isr_debugmonitor(void) UNHANDLED_ALIAS;
/* void _isr_reserved(void) UNHANDLED_ALIAS; */
void _isr_pendsv(void) UNHANDLED_ALIAS;
void _isr_systick(void) UNHANDLED_ALIAS;

/* device-specific (freescale) defined interrupt vectors */
void _isr_dma0_complete(void) UNHANDLED_ALIAS;
void _isr_dma1_complete(void) UNHANDLED_ALIAS;
void _isr_dma2_complete(void) UNHANDLED_ALIAS;
void _isr_dma3_complete(void) UNHANDLED_ALIAS;
void _isr_dma4_complete(void) UNHANDLED_ALIAS;
void _isr_dma5_complete(void) UNHANDLED_ALIAS;
void _isr_dma6_complete(void) UNHANDLED_ALIAS;
void _isr_dma7_complete(void) UNHANDLED_ALIAS;
void _isr_dma8_complete(void) UNHANDLED_ALIAS;
void _isr_dma9_complete(void) UNHANDLED_ALIAS;
void _isr_dma10_complete(void) UNHANDLED_ALIAS;
void _isr_dma11_complete(void) UNHANDLED_ALIAS;
void _isr_dma12_complete(void) UNHANDLED_ALIAS;
void _isr_dma13_complete(void) UNHANDLED_ALIAS;
void _isr_dma14_complete(void) UNHANDLED_ALIAS;
void _isr_dma15_complete(void) UNHANDLED_ALIAS;
void _isr_dma_error(void) UNHANDLED_ALIAS;
void _isr_mcm(void) UNHANDLED_ALIAS;
void _isr_flash_command_complete(void) UNHANDLED_ALIAS;
void _isr_flash_read_collision(void) UNHANDLED_ALIAS;
void _isr_low_voltage(void) UNHANDLED_ALIAS;
void _isr_llwu(void) UNHANDLED_ALIAS;
void _isr_watchdog(void) UNHANDLED_ALIAS;
void _isr_random_number_generator(void) UNHANDLED_ALIAS;
void _isr_i2c0(void) UNHANDLED_ALIAS;
void _isr_i2c1(void) UNHANDLED_ALIAS;
void _isr_spi0(void) UNHANDLED_ALIAS;
void _isr_spi1(void) UNHANDLED_ALIAS;
void _isr_spi2(void) UNHANDLED_ALIAS;
void _isr_can0_ored_msg_buffer(void) UNHANDLED_ALIAS;
void _isr_can0_bus_off(void) UNHANDLED_ALIAS;
void _isr_can0_error(void) UNHANDLED_ALIAS;
void _isr_can0_tx_warn(void) UNHANDLED_ALIAS;
void _isr_can0_rx_warn(void) UNHANDLED_ALIAS;
void _isr_can0_wake_up(void) UNHANDLED_ALIAS;
void _isr_i2s0_tx(void) UNHANDLED_ALIAS;
void _isr_i2s0_rx(void) UNHANDLED_ALIAS;
void _isr_can1_ored_msg_buffer(void) UNHANDLED_ALIAS;
void _isr_can1_bus_off(void) UNHANDLED_ALIAS;
void _isr_can1_error(void) UNHANDLED_ALIAS;
void _isr_can1_tx_warn(void) UNHANDLED_ALIAS;
void _isr_can1_rx_warn(void) UNHANDLED_ALIAS;
void _isr_can1_wake_up(void) UNHANDLED_ALIAS;
/* void _isr_reserved(void) UNHANDLED_ALIAS; */
void _isr_uart0_lon(void) UNHANDLED_ALIAS;
void _isr_uart0_status(void) UNHANDLED_ALIAS;
void _isr_uart0_error(void) UNHANDLED_ALIAS;
void _isr_uart1_status(void) UNHANDLED_ALIAS;
void _isr_uart1_error(void) UNHANDLED_ALIAS;
void _isr_uart2_status(void) UNHANDLED_ALIAS;
void _isr_uart2_error(void) UNHANDLED_ALIAS;
void _isr_uart3_status(void) UNHANDLED_ALIAS;
void _isr_uart3_error(void) UNHANDLED_ALIAS;
void _isr_uart4_status(void) UNHANDLED_ALIAS;
void _isr_uart4_error(void) UNHANDLED_ALIAS;
/* void _isr_reserved(void) UNHANDLED_ALIAS; */
/* void _isr_reserved(void) UNHANDLED_ALIAS; */
void _isr_adc0(void) UNHANDLED_ALIAS;
void _isr_adc1(void) UNHANDLED_ALIAS;
void _isr_cmp0(void) UNHANDLED_ALIAS;
void _isr_cmp1(void) UNHANDLED_ALIAS;
void _isr_cmp2(void) UNHANDLED_ALIAS;
void _isr_ftm0(void) UNHANDLED_ALIAS;
void _isr_ftm1(void) UNHANDLED_ALIAS;
void _isr_ftm2(void) UNHANDLED_ALIAS;
void _isr_cmt(void) UNHANDLED_ALIAS;
void _isr_rtc_alarm(void) UNHANDLED_ALIAS;
void _isr_rtc_seconds(void) UNHANDLED_ALIAS;
void _isr_pit0(void) UNHANDLED_ALIAS;
void _isr_pit1(void) UNHANDLED_ALIAS;
void _isr_pit2(void) UNHANDLED_ALIAS;
void _isr_pit3(void) UNHANDLED_ALIAS;
void _isr_pdb(void) UNHANDLED_ALIAS;
void _isr_usb_otg(void) UNHANDLED_ALIAS;
void _isr_usb_charger_detect(void) UNHANDLED_ALIAS;
void _isr_enet_1588_timer(void) UNHANDLED_ALIAS;
void _isr_enet_tx(void) UNHANDLED_ALIAS;
void _isr_enet_rx(void) UNHANDLED_ALIAS;
void _isr_enet_error_misc(void) UNHANDLED_ALIAS;
/* void _isr_reserved(void) UNHANDLED_ALIAS; */
void _isr_sdhc(void) UNHANDLED_ALIAS;
void _isr_dac0(void) UNHANDLED_ALIAS;
/* void _isr_reserved(void) UNHANDLED_ALIAS; */
void _isr_tsi(void) UNHANDLED_ALIAS;
void _isr_mcg(void) UNHANDLED_ALIAS;
void _isr_lpt(void) UNHANDLED_ALIAS;
/* void _isr_reserved(void) UNHANDLED_ALIAS; */
void _isr_porta_pin_detect(void) UNHANDLED_ALIAS;
void _isr_portb_pin_detect(void) UNHANDLED_ALIAS;
void _isr_portc_pin_detect(void) UNHANDLED_ALIAS;
void _isr_portd_pin_detect(void) UNHANDLED_ALIAS;
void _isr_porte_pin_detect(void) UNHANDLED_ALIAS;
/* void _isr_reserved(void) UNHANDLED_ALIAS; */
/* void _isr_reserved(void) UNHANDLED_ALIAS; */
void _isr_software(void) UNHANDLED_ALIAS;

/* This is the value of the stack pointer at power on. This is not a real function. */
void __stack_top(void);

typedef void (*ISR_func)(void);

const ISR_func isr_vector[111] ISR_VECTOR_SECTION =
  {
    /* ARM Cortex defined interrupt vectors */
    __stack_top,
    reset_handler,
    _isr_nmi,
    _isr_hardfault,
    _isr_memmanage,
    _isr_busfault,
    _isr_usagefault,
    _isr_reserved,
    _isr_reserved,
    _isr_reserved,
    _isr_reserved,
    _isr_svcall,
    _isr_debugmonitor,
    _isr_reserved,
    _isr_pendsv,
    _isr_systick,

    /* Device-specific (Freescale defined) interrupt vectors */
    _isr_dma0_complete,
    _isr_dma1_complete,
    _isr_dma2_complete,
    _isr_dma3_complete,
    _isr_dma4_complete,
    _isr_dma5_complete,
    _isr_dma6_complete,
    _isr_dma7_complete,
    _isr_dma8_complete,
    _isr_dma9_complete,
    _isr_dma10_complete,
    _isr_dma11_complete,
    _isr_dma12_complete,
    _isr_dma13_complete,
    _isr_dma14_complete,
    _isr_dma15_complete,
    _isr_dma_error,
    _isr_mcm,
    _isr_flash_command_complete,
    _isr_flash_read_collision,
    _isr_low_voltage,
    _isr_llwu,
    _isr_watchdog,
    _isr_random_number_generator,
    _isr_i2c0,
    _isr_i2c1,
    _isr_spi0,
    _isr_spi1,
    _isr_spi2,
    _isr_can0_ored_msg_buffer,
    _isr_can0_bus_off,
    _isr_can0_error,
    _isr_can0_tx_warn,
    _isr_can0_rx_warn,
    _isr_can0_wake_up,
    _isr_i2s0_tx,
    _isr_i2s0_rx,
    _isr_can1_ored_msg_buffer,
    _isr_can1_bus_off,
    _isr_can1_error,
    _isr_can1_tx_warn,
    _isr_can1_rx_warn,
    _isr_can1_wake_up,
    _isr_reserved,
    _isr_uart0_lon,
    _isr_uart0_status,
    _isr_uart0_error,
    _isr_uart1_status,
    _isr_uart1_error,
    _isr_uart2_status,
    _isr_uart2_error,
    _isr_uart3_status,
    _isr_uart3_error,
    _isr_uart4_status,
    _isr_uart4_error,
    _isr_reserved,
    _isr_reserved,
    _isr_adc0,
    _isr_adc1,
    _isr_cmp0,
    _isr_cmp1,
    _isr_cmp2,
    _isr_ftm0,
    _isr_ftm1,
    _isr_ftm2,
    _isr_cmt,
    _isr_rtc_alarm,
    _isr_rtc_seconds,
    _isr_pit0,
    _isr_pit1,
    _isr_pit2,
    _isr_pit3,
    _isr_pdb,
    _isr_usb_otg,
    _isr_usb_charger_detect,
    _isr_enet_1588_timer,
    _isr_enet_tx,
    _isr_enet_rx,
    _isr_enet_error_misc,
    _isr_reserved,
    _isr_sdhc,
    _isr_dac0,
    _isr_reserved,
    _isr_tsi,
    _isr_mcg,
    _isr_lpt,
    _isr_reserved,
    _isr_porta_pin_detect,
    _isr_portb_pin_detect,
    _isr_portc_pin_detect,
    _isr_portd_pin_detect,
    _isr_porte_pin_detect,
    _isr_reserved,
    _isr_reserved,
    _isr_software
  };

void
_isr_nmi(void)
{
  while(1);
}


static void
unhandled_interrupt(void)
{
  while(1);
}

/**
 * Default handler of Hard Faults
 *
 * This function is only an assembly language wrapper for the function
 * hard_fault_handler_c, defined in fault-handlers.c
 */
static void
dHardFault_handler(void)
{
  __asm volatile
  (
    "tst lr, #4\n"
    "ite eq\n"
    "mrseq r0, msp\n"
    "mrsne r0, psp\n"
    "b hard_fault_handler_c\n"
  );
  while(1);
}

/**
 * Default handler of Usage Fault
 */
static void
dUsageFault_handler(void)
{
  while(1);
}

/**
 * Default handler of MemManage Fault
 */
static void
dMemManage_handler(void)
{
  while(1);
}

/**
 * Default handler of Bus Fault
 */
static void
dBusFault_handler(void)
{
  while(1);
}


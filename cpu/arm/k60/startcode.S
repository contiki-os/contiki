/*******************************************************************************
*
* startcode.s
*
* Rob Laswick
*
* Startcode for the Freescale Kinetis K60 / ARM Cortex-M4.
* Includes CRT setup.
*
* Copyright (C) 2012 www.laswick.net
*
* This program is free software.  It comes without any warranty, to the extent
* permitted by applicable law.  You can redistribute it and/or modify it under
* the terms of the WTF Public License (WTFPL), Version 2, as published by
* Sam Hocevar.  See http://sam.zoy.org/wtfpl/COPYING for more details.
*
*******************************************************************************/
    .syntax unified
    .thumb

    .extern _stack_start                      /* Defined in our linker script */
    .extern _stack_end
    .extern _data_start
    .extern _data_end
    .extern _bss_start
    .extern _bss_end
    .extern _heap_start
    .extern _heap_end

    .weak main               /* User defined entry point (in a separate file) */

    .text

    /*
     * Before defining the vector table, we need to define "weak" symbols
     * and default handlers for EVERY exception.
     *
     * "weak" symbols can be overridden by user software without causing
     * conflicts.
     */

                                             /* ARM Cortex Defined Exceptions */
    .weak _reset_handler
    .weak _nmi_handler
    .weak _hard_fault_handler
    .weak _memory_management_fault_handler
    .weak _bus_fault_handler
    .weak _usage_fault_handler
    .weak _svc_handler
    .weak _debug_monitor_handler
    .weak _pend_sv_handler
    .weak _systick_handler

                            /* Vendor Specific (Freescale) Defined Exceptions */
    .weak _isr_dma0
    .weak _isr_dma1
    .weak _isr_dma2
    .weak _isr_dma3
    .weak _isr_dma4
    .weak _isr_dma5
    .weak _isr_dma6
    .weak _isr_dma7
    .weak _isr_dma8
    .weak _isr_dma9
    .weak _isr_dma10
    .weak _isr_dma11
    .weak _isr_dma12
    .weak _isr_dma13
    .weak _isr_dma14
    .weak _isr_dma15
    .weak _isr_dma_error
    .weak _isr_mcm
    .weak _isr_flash_cmd_complete
    .weak _isr_flash_read_collision
    .weak _isr_mode_controller
    .weak _isr_low_leakage_wakeup
    .weak _isr_watchdog
    .weak _isr_random_number_generator
    .weak _isr_i2c0
    .weak _isr_i2c1
    .weak _isr_spi0
    .weak _isr_spi1
    .weak _isr_spi2
    .weak _isr_can0_ored_msg_buffer
    .weak _isr_can0_buffer_off
    .weak _isr_can0_error
    .weak _isr_can0_tx_warning
    .weak _isr_can0_rx_warning
    .weak _isr_can0_wake_up
    .weak _isr_can0_imeu
    .weak _isr_can0_lost_rx
    .weak _isr_can1_ored_msg_buffer
    .weak _isr_can1_buffer_off
    .weak _isr_can1_error
    .weak _isr_can1_tx_warning
    .weak _isr_can1_rx_warning
    .weak _isr_can1_wake_up
    .weak _isr_can1_imeu
    .weak _isr_can1_lost_rx
    .weak _isr_uart0_status_sources
    .weak _isr_uart0_error_sources
    .weak _isr_uart1_status_sources
    .weak _isr_uart1_error_sources
    .weak _isr_uart2_status_sources
    .weak _isr_uart2_error_sources
    .weak _isr_uart3_status_sources
    .weak _isr_uart3_error_sources
    .weak _isr_uart4_status_sources
    .weak _isr_uart4_error_sources
    .weak _isr_uart5_status_sources
    .weak _isr_uart5_error_sources
    .weak _isr_adc0
    .weak _isr_adc1
    .weak _isr_cmp0
    .weak _isr_cmp1
    .weak _isr_cmp2
    .weak _isr_ftm0
    .weak _isr_ftm1
    .weak _isr_ftm2
    .weak _isr_cmt
    .weak _isr_rtc_alarm
    .weak _isr_rtc_seconds
    .weak _isr_pit0
    .weak _isr_pit1
    .weak _isr_pit2
    .weak _isr_pit3
    .weak _isr_pdb
    .weak _isr_usb_otg
    .weak _isr_usb_charger_detect
    .weak _isr_ethernet_mac_1588_timer
    .weak _isr_ethernet_mac_tx
    .weak _isr_ethernet_mac_rx
    .weak _isr_ethernet_mac_error
    .weak _isr_i2s
    .weak _isr_sdhc
    .weak _isr_dac0
    .weak _isr_dac1
    .weak _isr_tsi
    .weak _isr_mcg
    .weak _isr_low_power_timer
    .weak _isr_gpio_a
    .weak _isr_gpio_b
    .weak _isr_gpio_c
    .weak _isr_gpio_d
    .weak _isr_gpio_e
    .weak _isr_software


    /*
     * Ensure all symbols are globally visible.
     */

    .global _reset_handler
    .global _nmi_handler
    .global _hard_fault_handler
    .global _memory_management_fault_handler
    .global _bus_fault_handler
    .global _usage_fault_handler
    .global _svc_handler
    .global _debug_monitor_handler
    .global _pend_sv_handler
    .global _systick_handler

    .global _isr_dma0
    .global _isr_dma1
    .global _isr_dma2
    .global _isr_dma3
    .global _isr_dma4
    .global _isr_dma5
    .global _isr_dma6
    .global _isr_dma7
    .global _isr_dma8
    .global _isr_dma9
    .global _isr_dma10
    .global _isr_dma11
    .global _isr_dma12
    .global _isr_dma13
    .global _isr_dma14
    .global _isr_dma15
    .global _isr_dma_error
    .global _isr_mcm
    .global _isr_flash_cmd_complete
    .global _isr_flash_read_collision
    .global _isr_mode_controller
    .global _isr_low_leakage_wakeup
    .global _isr_watchdog
    .global _isr_random_number_generator
    .global _isr_i2c0
    .global _isr_i2c1
    .global _isr_spi0
    .global _isr_spi1
    .global _isr_spi2
    .global _isr_can0_ored_msg_buffer
    .global _isr_can0_buffer_off
    .global _isr_can0_error
    .global _isr_can0_tx_warning
    .global _isr_can0_rx_warning
    .global _isr_can0_wake_up
    .global _isr_can0_imeu
    .global _isr_can0_lost_rx
    .global _isr_can1_ored_msg_buffer
    .global _isr_can1_buffer_off
    .global _isr_can1_error
    .global _isr_can1_tx_warning
    .global _isr_can1_rx_warning
    .global _isr_can1_wake_up
    .global _isr_can1_imeu
    .global _isr_can1_lost_rx
    .global _isr_uart0_status_sources
    .global _isr_uart0_error_sources
    .global _isr_uart1_status_sources
    .global _isr_uart1_error_sources
    .global _isr_uart2_status_sources
    .global _isr_uart2_error_sources
    .global _isr_uart3_status_sources
    .global _isr_uart3_error_sources
    .global _isr_uart4_status_sources
    .global _isr_uart4_error_sources
    .global _isr_uart5_status_sources
    .global _isr_uart5_error_sources
    .global _isr_adc0
    .global _isr_adc1
    .global _isr_cmp0
    .global _isr_cmp1
    .global _isr_cmp2
    .global _isr_ftm0
    .global _isr_ftm1
    .global _isr_ftm2
    .global _isr_cmt
    .global _isr_rtc_alarm
    .global _isr_rtc_seconds
    .global _isr_pit0
    .global _isr_pit1
    .global _isr_pit2
    .global _isr_pit3
    .global _isr_pdb
    .global _isr_usb_otg
    .global _isr_usb_charger_detect
    .global _isr_ethernet_mac_1588_timer
    .global _isr_ethernet_mac_tx
    .global _isr_ethernet_mac_rx
    .global _isr_ethernet_mac_error
    .global _isr_i2s
    .global _isr_sdhc
    .global _isr_dac0
    .global _isr_dac1
    .global _isr_tsi
    .global _isr_mcg
    .global _isr_low_power_timer
    .global _isr_gpio_a
    .global _isr_gpio_b
    .global _isr_gpio_c
    .global _isr_gpio_d
    .global _isr_gpio_e
    .global _isr_software


    /*
     * Assign Default Handlers (defined below).
     */

    .thumb_set _reset_handler,                  _default_reset_handler
    .thumb_set _nmi_handler,                    _default_nmi_handler
    .thumb_set _hard_fault_handler,             _default_fault_handler
    .thumb_set _memory_management_fault_handler,_default_fault_handler
    .thumb_set _bus_fault_handler,              _default_fault_handler
    .thumb_set _usage_fault_handler,            _default_fault_handler
    .thumb_set _svc_handler,                    _default_irq_handler
    .thumb_set _debug_monitor_handler,          _default_irq_handler
    .thumb_set _pend_sv_handler,                _default_irq_handler
    .thumb_set _systick_handler,                _default_irq_handler

    .thumb_set _isr_dma0,                       _default_irq_handler
    .thumb_set _isr_dma1,                       _default_irq_handler
    .thumb_set _isr_dma2,                       _default_irq_handler
    .thumb_set _isr_dma3,                       _default_irq_handler
    .thumb_set _isr_dma4,                       _default_irq_handler
    .thumb_set _isr_dma5,                       _default_irq_handler
    .thumb_set _isr_dma6,                       _default_irq_handler
    .thumb_set _isr_dma7,                       _default_irq_handler
    .thumb_set _isr_dma8,                       _default_irq_handler
    .thumb_set _isr_dma9,                       _default_irq_handler
    .thumb_set _isr_dma10,                      _default_irq_handler
    .thumb_set _isr_dma11,                      _default_irq_handler
    .thumb_set _isr_dma12,                      _default_irq_handler
    .thumb_set _isr_dma13,                      _default_irq_handler
    .thumb_set _isr_dma14,                      _default_irq_handler
    .thumb_set _isr_dma15,                      _default_irq_handler
    .thumb_set _isr_dma_error,                  _default_irq_handler
    .thumb_set _isr_mcm,                        _default_irq_handler
    .thumb_set _isr_flash_cmd_complete,         _default_irq_handler
    .thumb_set _isr_flash_read_collision,       _default_irq_handler
    .thumb_set _isr_mode_controller,            _default_irq_handler
    .thumb_set _isr_low_leakage_wakeup,         _default_irq_handler
    .thumb_set _isr_watchdog,                   _default_irq_handler
    .thumb_set _isr_random_number_generator,    _default_irq_handler
    .thumb_set _isr_i2c0,                       _default_irq_handler
    .thumb_set _isr_i2c1,                       _default_irq_handler
    .thumb_set _isr_spi0,                       _default_irq_handler
    .thumb_set _isr_spi1,                       _default_irq_handler
    .thumb_set _isr_spi2,                       _default_irq_handler
    .thumb_set _isr_can0_ored_msg_buffer,       _default_irq_handler
    .thumb_set _isr_can0_buffer_off,            _default_irq_handler
    .thumb_set _isr_can0_error,                 _default_irq_handler
    .thumb_set _isr_can0_tx_warning,            _default_irq_handler
    .thumb_set _isr_can0_rx_warning,            _default_irq_handler
    .thumb_set _isr_can0_wake_up,               _default_irq_handler
    .thumb_set _isr_can0_imeu,                  _default_irq_handler
    .thumb_set _isr_can0_lost_rx,               _default_irq_handler
    .thumb_set _isr_can1_ored_msg_buffer,       _default_irq_handler
    .thumb_set _isr_can1_buffer_off,            _default_irq_handler
    .thumb_set _isr_can1_error,                 _default_irq_handler
    .thumb_set _isr_can1_tx_warning,            _default_irq_handler
    .thumb_set _isr_can1_rx_warning,            _default_irq_handler
    .thumb_set _isr_can1_wake_up,               _default_irq_handler
    .thumb_set _isr_can1_imeu,                  _default_irq_handler
    .thumb_set _isr_can1_lost_rx,               _default_irq_handler
    .thumb_set _isr_uart0_status_sources,       _default_irq_handler
    .thumb_set _isr_uart0_error_sources,        _default_irq_handler
    .thumb_set _isr_uart1_status_sources,       _default_irq_handler
    .thumb_set _isr_uart1_error_sources,        _default_irq_handler
    .thumb_set _isr_uart2_status_sources,       _default_irq_handler
    .thumb_set _isr_uart2_error_sources,        _default_irq_handler
    .thumb_set _isr_uart3_status_sources,       _default_irq_handler
    .thumb_set _isr_uart3_error_sources,        _default_irq_handler
    .thumb_set _isr_uart4_status_sources,       _default_irq_handler
    .thumb_set _isr_uart4_error_sources,        _default_irq_handler
    .thumb_set _isr_uart5_status_sources,       _default_irq_handler
    .thumb_set _isr_uart5_error_sources,        _default_irq_handler
    .thumb_set _isr_adc0,                       _default_irq_handler
    .thumb_set _isr_adc1,                       _default_irq_handler
    .thumb_set _isr_cmp0,                       _default_irq_handler
    .thumb_set _isr_cmp1,                       _default_irq_handler
    .thumb_set _isr_cmp2,                       _default_irq_handler
    .thumb_set _isr_ftm0,                       _default_irq_handler
    .thumb_set _isr_ftm1,                       _default_irq_handler
    .thumb_set _isr_ftm2,                       _default_irq_handler
    .thumb_set _isr_cmt,                        _default_irq_handler
    .thumb_set _isr_rtc_alarm,                  _default_irq_handler
    .thumb_set _isr_rtc_seconds,                _default_irq_handler
    .thumb_set _isr_pit0,                       _default_irq_handler
    .thumb_set _isr_pit1,                       _default_irq_handler
    .thumb_set _isr_pit2,                       _default_irq_handler
    .thumb_set _isr_pit3,                       _default_irq_handler
    .thumb_set _isr_pdb,                        _default_irq_handler
    .thumb_set _isr_usb_otg,                    _default_irq_handler
    .thumb_set _isr_usb_charger_detect,         _default_irq_handler
    .thumb_set _isr_ethernet_mac_1588_timer,    _default_irq_handler
    .thumb_set _isr_ethernet_mac_tx,            _default_irq_handler
    .thumb_set _isr_ethernet_mac_rx,            _default_irq_handler
    .thumb_set _isr_ethernet_mac_error,         _default_irq_handler
    .thumb_set _isr_i2s,                        _default_irq_handler
    .thumb_set _isr_sdhc,                       _default_irq_handler
    .thumb_set _isr_dac0,                       _default_irq_handler
    .thumb_set _isr_dac1,                       _default_irq_handler
    .thumb_set _isr_tsi,                        _default_irq_handler
    .thumb_set _isr_mcg,                        _default_irq_handler
    .thumb_set _isr_low_power_timer,            _default_irq_handler
    .thumb_set _isr_gpio_a,                     _default_irq_handler
    .thumb_set _isr_gpio_b,                     _default_irq_handler
    .thumb_set _isr_gpio_c,                     _default_irq_handler
    .thumb_set _isr_gpio_d,                     _default_irq_handler
    .thumb_set _isr_gpio_e,                     _default_irq_handler
    .thumb_set _isr_software,                   _default_irq_handler

    /*
     * We can now define the vector table.
     */

    .text
    .section .vector_table,"a",%progbits
    .align 2
    .thumb

_vector_table:
                                             /* ARM Cortex Defined Exceptions */
    .word _stack_start
    .word _reset_handler
    .word _nmi_handler
    .word _hard_fault_handler
    .word _memory_management_fault_handler
    .word _bus_fault_handler
    .word _usage_fault_handler
    .word 0x00000000
    .word 0x00000000
    .word 0x00000000
    .word 0x00000000
    .word _svc_handler
    .word _debug_monitor_handler
    .word 0x00000000
    .word _pend_sv_handler
    .word _systick_handler

                            /* Vendor Specific (Freescale) Defined Exceptions */
    .word _isr_dma0
    .word _isr_dma1
    .word _isr_dma2
    .word _isr_dma3
    .word _isr_dma4
    .word _isr_dma5
    .word _isr_dma6
    .word _isr_dma7
    .word _isr_dma8
    .word _isr_dma9
    .word _isr_dma10
    .word _isr_dma11
    .word _isr_dma12
    .word _isr_dma13
    .word _isr_dma14
    .word _isr_dma15
    .word _isr_dma_error
    .word _isr_mcm
    .word _isr_flash_cmd_complete
    .word _isr_flash_read_collision
    .word _isr_mode_controller
    .word _isr_low_leakage_wakeup
    .word _isr_watchdog
    .word _isr_random_number_generator
    .word _isr_i2c0
    .word _isr_i2c1
    .word _isr_spi0
    .word _isr_spi1
    .word _isr_spi2
    .word _isr_can0_ored_msg_buffer
    .word _isr_can0_buffer_off
    .word _isr_can0_error
    .word _isr_can0_tx_warning
    .word _isr_can0_rx_warning
    .word _isr_can0_wake_up
    .word _isr_can0_imeu
    .word _isr_can0_lost_rx
    .word _isr_can1_ored_msg_buffer
    .word _isr_can1_buffer_off
    .word _isr_can1_error
    .word _isr_can1_tx_warning
    .word _isr_can1_rx_warning
    .word _isr_can1_wake_up
    .word _isr_can1_imeu
    .word _isr_can1_lost_rx
    .word _isr_uart0_status_sources
    .word _isr_uart0_error_sources
    .word _isr_uart1_status_sources
    .word _isr_uart1_error_sources
    .word _isr_uart2_status_sources
    .word _isr_uart2_error_sources
    .word _isr_uart3_status_sources
    .word _isr_uart3_error_sources
    .word _isr_uart4_status_sources
    .word _isr_uart4_error_sources
    .word _isr_uart5_status_sources
    .word _isr_uart5_error_sources
    .word _isr_adc0
    .word _isr_adc1
    .word _isr_cmp0
    .word _isr_cmp1
    .word _isr_cmp2
    .word _isr_ftm0
    .word _isr_ftm1
    .word _isr_ftm2
    .word _isr_cmt
    .word _isr_rtc_alarm
    .word _isr_rtc_seconds
    .word _isr_pit0
    .word _isr_pit1
    .word _isr_pit2
    .word _isr_pit3
    .word _isr_pdb
    .word _isr_usb_otg
    .word _isr_usb_charger_detect
    .word _isr_ethernet_mac_1588_timer
    .word _isr_ethernet_mac_tx
    .word _isr_ethernet_mac_rx
    .word _isr_ethernet_mac_error
    .word _isr_i2s
    .word _isr_sdhc
    .word _isr_dac0
    .word _isr_dac1
    .word _isr_tsi
    .word _isr_mcg
    .word _isr_low_power_timer
    .word 0x00000000
    .word _isr_gpio_a
    .word _isr_gpio_b
    .word _isr_gpio_c
    .word _isr_gpio_d
    .word _isr_gpio_e
    .word 0x00000000
    .word 0x00000000
    .word _isr_software

    /*
     * Default Reset Handler
     */

    .text
    .section .default_handlers,"ax",%progbits
    .align 2
    .thumb_func

_default_reset_handler:

unlock_watchdog:
    ldr r6, =0x4005200e
    ldr r0, =0xc520
    strh r0, [r6]
    ldr r0, =0xd928
    strh r0, [r6]

disable_watchdog:
    ldr r6, =0x40052000
    ldr r0, =0x01d2
    strh r0, [r6]

    /*
     * Note: The preceding code must complete at speed before we can start
     *       setting breakpoints and single stepping, hence the provided
     *       label below "first_break" (i.e. (gdb) tb first_break).
     */

first_break:

set_stack_pointer:

    /*
     * The main stack pointer is automatically set to the value stored in
     * address 0x00000000 (which is the first element in the vector_table) by
     * the hardware. The next three lines are required in case this image
     * is not located at 0x0 (i.e. it is being launched from a bootloader).
     */

    ldr r1, =_vector_table
    ldr r2, [r1]
    mov	sp,r2

    /*
     * Relocate the .text section from FLASH to SRAM only if the load
     * address and the start address are not the same. This code supports
     * running an entire image out of SRAM.
     */

    ldr r0, =_text_start
    ldr r1, =_text_end
    ldr r2, =_text_load

    cmp r0, r2
    beq end_text_loop
text_loop:
    cmp r0, r1
    ittt    lt
    ldrlt   r3, [r2], #4
    strlt   r3, [r0], #4
    blt     text_loop
end_text_loop:


    /*
     * Relocate the .ramcode section from FLASH to SRAM.
     */

    ldr r0, =_ramcode_start
    ldr r1, =_ramcode_end
    ldr r2, =_ramcode_load

ramcode_loop:
    cmp r0, r1
    ittt    lt
    ldrlt   r3, [r2], #4
    strlt   r3, [r0], #4
    blt     ramcode_loop

    /*
     * Relocate vector table to SRAM.
     */

     ldr r0, =_vector_ram_start
     ldr r1, =_vector_ram_end
     ldr r2, =_vector_rom
vector_loop:
    cmp r0, r1
    ittt    lt
    ldrlt   r3, [r2], #4
    strlt   r3, [r0], #4
    blt     vector_loop

    /*
     * Point to the SRAM vector table.
     */

    ldr r1,=0xe000ed08
    ldr r0,=_vector_ram_start
    str r0,[r1]


c_runtime_setup:

    /*
     * In order to execute C code, the startcode is expected to relocate
     * the .text and .data sections from FLASH to RAM, and zero out the .bss
     * section in RAM.
     *
     * Typically in microcontroller based applications, like this one,
     * the .text section is not relocated and the code executes from FLASH.
     */

    /*
     * Relocate the .data section from FLASH to SRAM
     *
     * Note: _data_start contains the SRAM (destination) address and
     *       _text_end contains the (source) address of the .data section
     *       in FLASH.
     */

    ldr r0, =_data_start
    ldr r1, =_data_end
    ldr r2, =_data_load

data_loop:
    cmp r0, r1
    ittt    lt
    ldrlt   r3, [r2], #4
    strlt   r3, [r0], #4
    blt     data_loop

    /*
     * Zero out the .bss section in SRAM.
     */

    ldr r0, =_bss_start
    ldr r1, =_bss_end
    ldr r2, =0x00000000

bss_loop:
    cmp r0, r1
    itt     lt
    strlt   r2, [r0], #4
    blt     bss_loop

    /*
     * Zero out heap.
     */

    ldr r0, =_heap_start
    ldr r1, =_heap_end

heap_loop:
    cmp r0, r1
    itt     lt
    strlt   r2, [r0], #4
    blt     heap_loop

    /*
     * Watermark the stack.
     */

/*
 * WRONG DIRECTION!!! Don't forget the stack grows DOWN!!!!
    ldr r0, =_stack_start
    ldr r1, =_stack_end
    ldr r2, =0xefefefef
*/

    ldr r0, =_stack_end
    ldr r1, =_stack_start
    ldr r2, =0xefefefef

stack_loop:
    cmp r0, r1
    itt     lt
    strlt   r2, [r0], #4
    blt     stack_loop

call_user_asm_code:
    bl main

    /*
     * Epilogue processing would go here.
     */

end_loop:
    b end_loop


    /*
     * Default Exception & IRQ Handlers
     *
     * It's a good idea, especially during early development, to set a
     * breakpoint on each of the default handlers.  This way if/when an
     * unexpected exception occurs the debugger will halt immediately
     * making it obvious an exception has occurred (this can save
     * enormous amounts of time wondering why your software isn't working).
     *
     * Knowing an exception occurred is useful ... but what caused it???
     * Being able to identify the exact line of code that caused in the
     * exception is gold.
     *
     * When an exception occurs the ARM Cortex M3/4 stacks the contents
     * of 8 key registers: R0, R1, R2 ,R3, R12, LR, PC, PSR.  The stacked PC
     * contains the address of the instruction that caused the exception.
     * You can look up this address in the disassembly file (which is
     * produced automatically by the makefile (out.s)).
     *
     * If/when the debugger breaks on one of the exceptions below examine the
     * registers to determine the current address of the stack pointer
     * (i.e. (gdb) info reg).  Display 8 words of memory starting at the
     * stack pointer address to view the stacked exception context
     * (i.e. (gdb) x/8x <addr>, or more simply (gdb) x/8x $sp).
     * The 7th word is the stacked PC, the offending instruction address.
     */

    .align 2
    .thumb_func

_default_nmi_handler:
    b _default_nmi_handler

    .align 2
    .thumb_func

_default_fault_handler:
    b _default_fault_handler

    .align 2
    .thumb_func

_default_irq_handler:
    b _default_irq_handler

    .end

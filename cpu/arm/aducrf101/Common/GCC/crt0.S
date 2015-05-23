/**
 * Copyright (c) 2014, Analog Devices, Inc.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted (subject to the limitations in the
 * disclaimer below) provided that the following conditions are met:
 *
 * - Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 *
 * - Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in the
 *   documentation and/or other materials provided with the
 *   distribution.
 *
 * - Neither the name of Analog Devices, Inc. nor the names of its
 *   contributors may be used to endorse or promote products derived
 *   from this software without specific prior written permission.
 *
 * NO EXPRESS OR IMPLIED LICENSES TO ANY PARTY'S PATENT RIGHTS ARE
 * GRANTED BY THIS LICENSE.  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT
 * HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
 * IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

.equ SCB_VTOR, 0xE000ED08

/* Vector table */
        .macro handler name
        .long \name\()
        .weak \name\()
        .set \name\(), unhandled_vector
        .endm

        .macro handler_reserved
        .long 0
        .endm

        .section .vectors, "a", %progbits
vectors:
        .long stack_end
        .long Reset_Handler

        /* Cortex-M3 core interrupts */
        handler NMI_Handler
        handler HardFault_Handler
        handler MemManage_Handler
        handler BusFault_Handler
        handler UsageFault_Handler
        handler_reserved
        handler_reserved
        handler_reserved
        handler_reserved
        handler SVC_Handler
        handler DebugMon_Handler
        handler_reserved
        handler PendSV_Handler
        handler SysTick_Handler

        /* ADuCRF101 external interrupts */
        handler WakeUp_Int_Handler
        handler Ext_Int0_Handler
        handler Ext_Int1_Handler
        handler Ext_Int2_Handler
        handler Ext_Int3_Handler
        handler Ext_Int4_Handler
        handler Ext_Int5_Handler
        handler Ext_Int6_Handler
        handler Ext_Int7_Handler
        handler Ext_Int8_Handler
        handler WDog_Tmr_Int_Handler
        handler_reserved
        handler GP_Tmr0_Int_Handler
        handler GP_Tmr1_Int_Handler
        handler ADC0_Int_Handler
        handler Flsh_Int_Handler
        handler UART_Int_Handler
        handler SPI0_Int_Handler
        handler SPI1_Int_Handler
        handler I2C0_Slave_Int_Handler
        handler I2C0_Master_Int_Handler
        handler_reserved
        handler_reserved
        handler DMA_Err_Int_Handler
        handler DMA_SPI1_TX_Int_Handler
        handler DMA_SPI1_RX_Int_Handler
        handler DMA_UART_TX_Int_Handler
        handler DMA_UART_RX_Int_Handler
        handler DMA_I2C0_STX_Int_Handler
        handler DMA_I2C0_SRX_Int_Handler
        handler DMA_I2C0_MTX_Int_Handler
        handler DMA_I2C0_MRX_Int_Handler
        handler_reserved
        handler_reserved
        handler_reserved
        handler DMA_ADC_Int_Handler
        handler DMA_SPI0_TX_Int_Handler
        handler DMA_SPI0_RX_Int_Handler
        handler PWMTrip_Int_Handler
        handler PWM0_Int_Handler
        handler PWM1_Int_Handler
        handler PWM2_Int_Handler
        handler PWM3_Int_Handler

/* Reset handler */
        .section .text
        .syntax unified
        .code 16
        .global Reset_Handler
        .thumb_func
Reset_Handler:
        /* Set up some basics, in case we came here from a call
        rather than system reset. */

        /* Disable interrupts */
        cpsid i

        /* Privileged mode, main stack, no floating point */
        mov r0, #0
        msr control, r0
        isb

        /* Point vector table to the right place */
        ldr r0, =__vectors_start
        ldr r1, =SCB_VTOR
        str r0, [r1]

        /* Load initial stack pointer */
        ldr r0, =stack_end
        mov sp, r0
        isb

        /* Clear BSS */
        mov r0, #0
        ldr r1, =__bss_start
        ldr r2, =__bss_end
zero_bss_loop:
        cmp r1, r2
        it lt
        strlt r0, [r1], #4
        blt zero_bss_loop

        /* Copy initialized data from flash to RAM */
        ldr r0, =__data_flash_start
        ldr r1, =__data_start
        ldr r2, =__data_end
copy_data_loop:
        ldr r3, [r0], #4
        cmp r1, r2
        it lt
        strlt r3, [r1], #4
        blt copy_data_loop

        /* We can run C code now */
        bl main

        /* If main returned, just loop */
        b .

/* Handler for otherwise unhandled vectors */
        .section .text,"ax",%progbits
        .thumb_func
unhandled_vector:
        b unhandled_vector

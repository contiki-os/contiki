; Copyright (c) 2014, Analog Devices, Inc.  All rights reserved.
;
; Redistribution and use in source and binary forms, with or without
; modification, are permitted (subject to the limitations in the
; disclaimer below) provided that the following conditions are met:
;
; - Redistributions of source code must retain the above copyright
;   notice, this list of conditions and the following disclaimer.
;
; - Redistributions in binary form must reproduce the above copyright
;   notice, this list of conditions and the following disclaimer in the
;   documentation and/or other materials provided with the
;   distribution.
;
; - Neither the name of Analog Devices, Inc. nor the names of its
;   contributors may be used to endorse or promote products derived
;   from this software without specific prior written permission.
;
; NO EXPRESS OR IMPLIED LICENSES TO ANY PARTY'S PATENT RIGHTS ARE
; GRANTED BY THIS LICENSE.  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT
; HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED
; WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
; MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
; DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
; LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
; CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
; SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
; BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
; WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
; OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
; IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

;    Module       : startup_ADuCRF101.s
;    Description  : Cortex-M3 startup file - ADuCRF101 - EWARM Version
;    Date         : 15 February 2012
;    Version      : v1.00
;    Changelog    : v1.00 Initial



        MODULE  ?cstartup

        ;; Forward declaration of sections.
        SECTION CSTACK:DATA:NOROOT(3)

        SECTION .intvec:CODE:NOROOT(2)

        EXTERN  __iar_program_start
        PUBLIC  __vector_table

        DATA
__vector_table
        DCD     sfe(CSTACK)
        DCD     __iar_program_start

        DCD     Nmi_Handler                                 ; The NMI handler        
        DCD     Fault_Handler                              ; The hard fault handler 
        DCD     MemManage_Handler                     ; The MPU fault handler  
        DCD     BusFault_Handler                      ; The bus fault handler  
        DCD     UsageFault_Handler                    ; The usage fault handler
        DCD     0                                     ; Reserved               
        DCD     0                                     ; Reserved               
        DCD     0                                     ; Reserved               
        DCD     0                                     ; Reserved               
        DCD     SVC_Handler                           ; SVCall handler         
        DCD     DebugMon_Handler                      ; Debug monitor handler  
        DCD     0                                     ; Reserved               
        DCD     PendSV_Handler                        ; The PendSV handler     
        DCD     SysTick_Handler                       ; The SysTick handler    


         ; External Interrupts
	DCD     WakeUp_Int_Handler                    ;  Wake Up Timer              [ 0]
	DCD     Ext_Int0_Handler                      ;  External Interrupt 0       [ 1]
	DCD     Ext_Int1_Handler                      ;  External Interrupt 1       [ 2]
	DCD     Ext_Int2_Handler                      ;  External Interrupt 2       [ 3]
	DCD     Ext_Int3_Handler                      ;  External Interrupt 3       [ 4]
	DCD     Ext_Int4_Handler                      ;  External Interrupt 4       [ 5]
	DCD     Ext_Int5_Handler                      ;  External Interrupt 5       [ 6]
	DCD     Ext_Int6_Handler                      ;  External Interrupt 6       [ 7]
	DCD     Ext_Int7_Handler                      ;  External Interrupt 7       [ 8]
	DCD     Ext_Int8_Handler                      ;  External Interrupt 8       [ 9]
	DCD     WDog_Tmr_Int_Handler                  ;  Watchdog timer handler     [10]
	DCD     UnUsed_Handler                        ;  Reserved                   [11]
	DCD     GP_Tmr0_Int_Handler                   ;  General purpose timer 0    [12]
	DCD     GP_Tmr1_Int_Handler                   ;  General purpose timer 1    [13]
	DCD     ADC0_Int_Handler                      ;  ADC Interrupt              [14]
	DCD     Flsh_Int_Handler                      ;  Flash IRQ                  [15]
	DCD     UART_Int_Handler                      ;  UART0                      [16]
	DCD     SPI0_Int_Handler                      ;  SPI 0                      [17]
	DCD     SPI1_Int_Handler                      ;  SPI 1                      [18]
	DCD     I2C0_Slave_Int_Handler                ;  I2C0 Slave                 [19]
	DCD     I2C0_Master_Int_Handler               ;  I2C0 Master                [20]
	DCD     UnUsed_Handler                        ;  Reserved                   [21]
	DCD     UnUsed_Handler                        ;  Reserved                   [22]
	DCD     DMA_Err_Int_Handler                   ;  DMA Error interrupt        [23]
	DCD     DMA_SPI1_TX_Int_Handler               ;  DMA SPI1 TX                [24]
	DCD     DMA_SPI1_RX_Int_Handler               ;  DMA SPI1 RX                [25]
	DCD     DMA_UART_TX_Int_Handler               ;  DMA UART TX                [26]
	DCD     DMA_UART_RX_Int_Handler               ;  DMA UART RX                [27]
	DCD     DMA_I2C0_STX_Int_Handler              ;  DMA I2C0 Slave TX          [28]
	DCD     DMA_I2C0_SRX_Int_Handler              ;  DMA I2C0 Slave RX          [29]
	DCD     DMA_I2C0_MTX_Int_Handler              ;  DMA I2C0 Master TX         [30]
	DCD     DMA_I2C0_MRX_Int_Handler              ;  DMA I2C0 Master RX         [31]
	DCD     UnUsed_Handler                        ;  Reserved                   [32]
	DCD     UnUsed_Handler                        ;  Reserved                   [33]
	DCD     UnUsed_Handler                        ;  Reserved                   [34]
	DCD     DMA_ADC_Int_Handler                   ;  DMA ADC                    [35]
	DCD     DMA_SPI0_TX_Int_Handler               ;  DMA SPI0 TX                [36]
	DCD     DMA_SPI0_RX_Int_Handler               ;  DMA SPI0 RX                [37]
	DCD     PWMTrip_Int_Handler                   ;  PWM Trip                   [38]
	DCD     PWM0_Int_Handler                      ;  PWM 0                      [39]
	DCD     PWM1_Int_Handler                      ;  PWM 1                      [40]
	DCD     PWM2_Int_Handler                      ;  PWM 2                      [41]
	DCD     PWM3_Int_Handler                      ;  PWM 3                      [42]
	DCD     UnUsed_Handler                        ;  Unused                     [43]

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;; Default interrupt handlers.
;;
	    PUBWEAK SysTick_Handler
	    PUBWEAK Nmi_Handler
	    PUBWEAK Fault_Handler
	    PUBWEAK MemManage_Handler
	    PUBWEAK BusFault_Handler
	    PUBWEAK UsageFault_Handler
	    PUBWEAK SVC_Handler
	    PUBWEAK DebugMon_Handler
	    PUBWEAK PendSV_Handler
	    PUBWEAK WakeUp_Int_Handler
	    PUBWEAK Ext_Int0_Handler
	    PUBWEAK Ext_Int1_Handler
	    PUBWEAK Ext_Int2_Handler
	    PUBWEAK Ext_Int3_Handler
	    PUBWEAK Ext_Int4_Handler
	    PUBWEAK Ext_Int5_Handler
	    PUBWEAK Ext_Int6_Handler
	    PUBWEAK Ext_Int7_Handler
	    PUBWEAK Ext_Int8_Handler
	    PUBWEAK WDog_Tmr_Int_Handler
	    PUBWEAK GP_Tmr0_Int_Handler
	    PUBWEAK GP_Tmr1_Int_Handler
	    PUBWEAK ADC0_Int_Handler
	    PUBWEAK Flsh_Int_Handler
	    PUBWEAK UART_Int_Handler
	    PUBWEAK SPI0_Int_Handler
	    PUBWEAK SPI1_Int_Handler
	    PUBWEAK I2C0_Slave_Int_Handler
	    PUBWEAK I2C0_Master_Int_Handler
	    PUBWEAK DMA_Err_Int_Handler
	    PUBWEAK DMA_SPI1_TX_Int_Handler
	    PUBWEAK DMA_SPI1_RX_Int_Handler
	    PUBWEAK DMA_UART_TX_Int_Handler
	    PUBWEAK DMA_UART_RX_Int_Handler
	    PUBWEAK DMA_I2C0_STX_Int_Handler
	    PUBWEAK DMA_I2C0_SRX_Int_Handler
	    PUBWEAK DMA_I2C0_MTX_Int_Handler
	    PUBWEAK DMA_I2C0_MRX_Int_Handler
	    PUBWEAK DMA_ADC_Int_Handler
	    PUBWEAK DMA_SPI0_TX_Int_Handler
	    PUBWEAK DMA_SPI0_RX_Int_Handler
	    PUBWEAK PWMTrip_Int_Handler
	    PUBWEAK PWM0_Int_Handler
	    PUBWEAK PWM1_Int_Handler
	    PUBWEAK PWM2_Int_Handler
	    PUBWEAK PWM3_Int_Handler

	    PUBWEAK UnUsed_Handler



        THUMB
        SECTION .text:CODE:REORDER:NOROOT(1)
Nmi_Handler
Fault_Handler
MemManage_Handler
BusFault_Handler
UsageFault_Handler
SVC_Handler
DebugMon_Handler
PendSV_Handler
SysTick_Handler
WakeUp_Int_Handler
Ext_Int0_Handler
Ext_Int1_Handler
Ext_Int2_Handler
Ext_Int3_Handler
Ext_Int4_Handler
Ext_Int5_Handler
Ext_Int6_Handler
Ext_Int7_Handler
Ext_Int8_Handler
WDog_Tmr_Int_Handler
GP_Tmr0_Int_Handler
GP_Tmr1_Int_Handler
ADC0_Int_Handler
Flsh_Int_Handler
UART_Int_Handler
SPI0_Int_Handler
SPI1_Int_Handler
I2C0_Slave_Int_Handler
I2C0_Master_Int_Handler
DMA_Err_Int_Handler
DMA_SPI1_TX_Int_Handler
DMA_SPI1_RX_Int_Handler
DMA_UART_TX_Int_Handler
DMA_UART_RX_Int_Handler
DMA_I2C0_STX_Int_Handler
DMA_I2C0_SRX_Int_Handler
DMA_I2C0_MTX_Int_Handler
DMA_I2C0_MRX_Int_Handler
DMA_ADC_Int_Handler
DMA_SPI0_TX_Int_Handler
DMA_SPI0_RX_Int_Handler
PWMTrip_Int_Handler
PWM0_Int_Handler
PWM1_Int_Handler
PWM2_Int_Handler
PWM3_Int_Handler
UnUsed_Handler
        B UnUsed_Handler

        END

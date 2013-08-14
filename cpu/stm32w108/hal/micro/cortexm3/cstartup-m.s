/**************************************************
 *
 * Part one of the system initialization code, contains low-level
 * initialization, plain thumb variant.
 *

 * Customized by St Corporation for STM32W
 *<!--(C) COPYRIGHT 2010 STMicroelectronics. All rights reserved.        -->
 *
 **************************************************/

;
; The modules in this file are included in the libraries, and may be replaced
; by any user-defined modules that define the PUBLIC symbol _program_start or
; a user defined start symbol.
; To override the cstartup defined in the library, simply add your modified
; version to the workbench project.
;
; The vector table is normally located at address 0.
; When debugging in RAM, it can be located in RAM, aligned to at least 2^6.
; The name "__vector_table" has special meaning for C-SPY:
; it is where the SP start value is found, and the NVIC vector
; table register (VTOR) is initialized to this address if != 0.
;
; Cortex-M version
;

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

        ;; Standard Cortex-M3 Vectors
        DCD     NMI_Handler          ;;NMI Handler
        DCD     HardFault_Handler    ;;Hard Fault Handler
        DCD     MemManage_Handler    ;;Memory Fault Handler 
        DCD     BusFault_Handler     ;;Bus Fault Handler
        DCD     UsageFault_Handler   ;;Usage Fault Handler
        DCD     0                    ;;Reserved
        DCD     0                    ;;Reserved
        DCD     0                    ;;Reserved
        DCD     0                    ;;Reserved
        DCD     SVC_Handler          ;;SVCall Handler
        DCD     DebugMon_Handler     ;;Debug Monitor Handler
        DCD     0                    ;;Reserved
        DCD     PendSV_Handler       ;;PendSV Handler
        DCD     SysTick_Handler      ;;SysTick Handler

        ;; STM32W Vectors
        DCD     halTimer1Isr         ;;Timer 1 Handler
        DCD     halTimer2Isr         ;;Timer 2 Handler
        DCD     halManagementIsr     ;;Management Handler
        DCD     halBaseBandIsr       ;;BaseBand Handler
        DCD     halSleepTimerIsr     ;;Sleep Timer Handler
        DCD     halSc1Isr            ;;SC1 Handler
        DCD     halSc2Isr            ;;SC2 Handler
        DCD     halSecurityIsr       ;;Security Handler
        DCD     halStackMacTimerIsr  ;;MAC Timer Handler
        DCD     stmRadioTransmitIsr   ;;MAC TX Handler
        DCD     stmRadioReceiveIsr    ;;MAC RX Handler
        DCD     halAdcIsr            ;;ADC Handler
        DCD     halIrqAIsr           ;;GPIO IRQA Handler
        DCD     halIrqBIsr           ;;GPIO IRQB Handler
        DCD     halIrqCIsr           ;;GPIO IRQC Handler
        DCD     halIrqDIsr           ;;GPIO IRQD Handler
        DCD     halDebugIsr          ;;Debug Handler


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;; Default interrupt handlers.
;;

        PUBWEAK NMI_Handler
        PUBWEAK HardFault_Handler
        PUBWEAK MemManage_Handler
        PUBWEAK BusFault_Handler
        PUBWEAK UsageFault_Handler
        PUBWEAK SVC_Handler
        PUBWEAK DebugMon_Handler
        PUBWEAK PendSV_Handler
        PUBWEAK SysTick_Handler

        PUBWEAK halTimer1Isr        
        PUBWEAK halTimer2Isr        
        PUBWEAK halManagementIsr    
        PUBWEAK halBaseBandIsr      
        PUBWEAK halSleepTimerIsr    
        PUBWEAK halSc1Isr           
        PUBWEAK halSc2Isr           
        PUBWEAK halSecurityIsr      
        PUBWEAK halStackMacTimerIsr 
        PUBWEAK stmRadioTransmitIsr  
        PUBWEAK stmRadioReceiveIsr   
        PUBWEAK halAdcIsr           
        PUBWEAK halIrqAIsr          
        PUBWEAK halIrqBIsr          
        PUBWEAK halIrqCIsr          
        PUBWEAK halIrqDIsr          
        PUBWEAK halDebugIsr         






        SECTION .text:CODE:REORDER(1)
        THUMB

NMI_Handler
HardFault_Handler
MemManage_Handler
BusFault_Handler
UsageFault_Handler
SVC_Handler
DebugMon_Handler
PendSV_Handler
SysTick_Handler
halTimer1Isr       
halTimer2Isr       
halManagementIsr   
halBaseBandIsr     
halSleepTimerIsr   
halSc1Isr          
halSc2Isr          
halSecurityIsr     
halStackMacTimerIsr
stmRadioTransmitIsr 
stmRadioReceiveIsr  
halAdcIsr          
halIrqAIsr         
halIrqBIsr         
halIrqCIsr         
halIrqDIsr         
halDebugIsr        
Default_Handler
        B Default_Handler

        END

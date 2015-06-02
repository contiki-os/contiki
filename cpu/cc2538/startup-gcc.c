/*
 *  Copyright (C) 2012 Texas Instruments Incorporated - http://www.ti.com/
 *
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *
 *    Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 *    Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 *    Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 *  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
/**
 * \addtogroup cc2538
 * @{
 *
 * \file
 * Startup code for the cc2538 chip, to be used when building with gcc
 */
#include "contiki.h"
#include "reg.h"
#include "flash-cca.h"
#include "sys-ctrl.h"
#include "rom-util.h"

#include <stdint.h>
/*---------------------------------------------------------------------------*/
extern int main(void);
/*---------------------------------------------------------------------------*/
#define WEAK_ALIAS(x) __attribute__ ((weak, alias(#x)))
/*---------------------------------------------------------------------------*/
/* System handlers provided here */
void reset_handler(void);
void nmi_handler(void);
void default_handler(void);

/* System Handler and ISR prototypes implemented elsewhere */
void clock_isr(void); /* SysTick Handler */
void gpio_port_a_isr(void);
void gpio_port_b_isr(void);
void gpio_port_c_isr(void);
void gpio_port_d_isr(void);
void rtimer_isr(void);
void cc2538_rf_rx_tx_isr(void);
void cc2538_rf_err_isr(void);
void udma_isr(void);
void udma_err_isr(void);
void usb_isr(void) WEAK_ALIAS(default_handler);
void uart0_isr(void) WEAK_ALIAS(default_handler);
void uart1_isr(void) WEAK_ALIAS(default_handler);
void crypto_isr(void);

/* Boot Loader Backdoor selection */
#if FLASH_CCA_CONF_BOOTLDR_BACKDOOR
/* Backdoor enabled */

#if FLASH_CCA_CONF_BOOTLDR_BACKDOOR_ACTIVE_HIGH
#define FLASH_CCA_BOOTLDR_CFG_ACTIVE_LEVEL FLASH_CCA_BOOTLDR_CFG_ACTIVE_HIGH
#else
#define FLASH_CCA_BOOTLDR_CFG_ACTIVE_LEVEL 0
#endif

#if ((FLASH_CCA_CONF_BOOTLDR_BACKDOOR_PORT_A_PIN < 0) || (FLASH_CCA_CONF_BOOTLDR_BACKDOOR_PORT_A_PIN > 7))
#error Invalid boot loader backdoor pin. Please set FLASH_CCA_CONF_BOOTLDR_BACKDOOR_PORT_A_PIN between 0 and 7 (indicating PA0 - PA7).
#endif

#define FLASH_CCA_BOOTLDR_CFG (FLASH_CCA_BOOTLDR_CFG_ENABLE \
  | FLASH_CCA_BOOTLDR_CFG_ACTIVE_LEVEL \
  | (FLASH_CCA_CONF_BOOTLDR_BACKDOOR_PORT_A_PIN << FLASH_CCA_BOOTLDR_CFG_PORT_A_PIN_S))
#else
#define FLASH_CCA_BOOTLDR_CFG FLASH_CCA_BOOTLDR_CFG_DISABLE
#endif
/*---------------------------------------------------------------------------*/
/* Allocate stack space */
static unsigned long stack[512] __attribute__ ((section(".stack")));
/*---------------------------------------------------------------------------*/
/* Linker construct indicating .text section location */
extern uint8_t _text[0];
/*---------------------------------------------------------------------------*/
__attribute__ ((section(".flashcca"), used))
const flash_cca_lock_page_t __cca = {
  FLASH_CCA_BOOTLDR_CFG,          /* Boot loader backdoor configuration */
  FLASH_CCA_IMAGE_VALID,         /* Image valid */
  &_text,                        /* Vector table located at the start of .text */
  /* Unlock all pages and debug */
  { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF }
};
/*---------------------------------------------------------------------------*/
__attribute__ ((section(".vectors"), used))
void(*const vectors[])(void) =
{
  (void (*)(void))((unsigned long)stack + sizeof(stack)),   /* Stack pointer */
  reset_handler,              /* Reset handler */
  nmi_handler,                /* The NMI handler */
  default_handler,            /* The hard fault handler */
  default_handler,            /* 4 The MPU fault handler */
  default_handler,            /* 5 The bus fault handler */
  default_handler,            /* 6 The usage fault handler */
  0,                          /* 7 Reserved */
  0,                          /* 8 Reserved */
  0,                          /* 9 Reserved */
  0,                          /* 10 Reserved */
  default_handler,            /* 11 SVCall handler */
  default_handler,            /* 12 Debug monitor handler */
  0,                          /* 13 Reserved */
  default_handler,            /* 14 The PendSV handler */
  clock_isr,                  /* 15 The SysTick handler */
  gpio_port_a_isr,            /* 16 GPIO Port A */
  gpio_port_b_isr,            /* 17 GPIO Port B */
  gpio_port_c_isr,            /* 18 GPIO Port C */
  gpio_port_d_isr,            /* 19 GPIO Port D */
  0,                          /* 20 none */
  uart0_isr,                  /* 21 UART0 Rx and Tx */
  uart1_isr,                  /* 22 UART1 Rx and Tx */
  default_handler,            /* 23 SSI0 Rx and Tx */
  default_handler,            /* 24 I2C Master and Slave */
  0,                          /* 25 Reserved */
  0,                          /* 26 Reserved */
  0,                          /* 27 Reserved */
  0,                          /* 28 Reserved */
  0,                          /* 29 Reserved */
  default_handler,            /* 30 ADC Sequence 0 */
  0,                          /* 31 Reserved */
  0,                          /* 32 Reserved */
  0,                          /* 33 Reserved */
  default_handler,            /* 34 Watchdog timer, timer 0 */
  default_handler,            /* 35 Timer 0 subtimer A */
  default_handler,            /* 36 Timer 0 subtimer B */
  default_handler,            /* 37 Timer 1 subtimer A */
  default_handler,            /* 38 Timer 1 subtimer B */
  default_handler,            /* 39 Timer 2 subtimer A */
  default_handler,            /* 40 Timer 2 subtimer B */
  default_handler,            /* 41 Analog Comparator 0 */
  default_handler,            /* 42 RFCore Rx/Tx (Alternate) */
  default_handler,            /* 43 RFCore Error (Alternate) */
  default_handler,            /* 44 System Control */
  default_handler,            /* 45 FLASH Control */
  default_handler,            /* 46 AES (Alternate) */
  default_handler,            /* 47 PKA (Alternate) */
  default_handler,            /* 48 SM Timer (Alternate) */
  default_handler,            /* 49 MacTimer (Alternate) */
  default_handler,            /* 50 SSI1 Rx and Tx */
  default_handler,            /* 51 Timer 3 subtimer A */
  default_handler,            /* 52 Timer 3 subtimer B */
  0,                          /* 53 Reserved */
  0,                          /* 54 Reserved */
  0,                          /* 55 Reserved */
  0,                          /* 56 Reserved */
  0,                          /* 57 Reserved */
  0,                          /* 58 Reserved */
  0,                          /* 59 Reserved */
  0,                          /* 60 Reserved */
  0,                          /* 61 Reserved */
  udma_isr,                   /* 62 uDMA */
  udma_err_isr,               /* 63 uDMA Error */
  0,                          /* 64 64-155 are not in use */
  0,                          /* 65 */
  0,                          /* 66 */
  0,                          /* 67 */
  0,                          /* 68 */
  0,                          /* 69 */
  0,                          /* 70 */
  0,                          /* 71 */
  0,                          /* 72 */
  0,                          /* 73 */
  0,                          /* 74 */
  0,                          /* 75 */
  0,                          /* 76 */
  0,                          /* 77 */
  0,                          /* 78 */
  0,                          /* 79 */
  0,                          /* 80 */
  0,                          /* 81 */
  0,                          /* 82 */
  0,                          /* 83 */
  0,                          /* 84 */
  0,                          /* 85 */
  0,                          /* 86 */
  0,                          /* 87 */
  0,                          /* 88 */
  0,                          /* 89 */
  0,                          /* 90 */
  0,                          /* 91 */
  0,                          /* 92 */
  0,                          /* 93 */
  0,                          /* 94 */
  0,                          /* 95 */
  0,                          /* 96 */
  0,                          /* 97 */
  0,                          /* 98 */
  0,                          /* 99 */
  0,                          /* 100 */
  0,                          /* 101 */
  0,                          /* 102 */
  0,                          /* 103 */
  0,                          /* 104 */
  0,                          /* 105 */
  0,                          /* 106 */
  0,                          /* 107 */
  0,                          /* 108 */
  0,                          /* 109 */
  0,                          /* 110 */
  0,                          /* 111 */
  0,                          /* 112 */
  0,                          /* 113 */
  0,                          /* 114 */
  0,                          /* 115 */
  0,                          /* 116 */
  0,                          /* 117 */
  0,                          /* 118 */
  0,                          /* 119 */
  0,                          /* 120 */
  0,                          /* 121 */
  0,                          /* 122 */
  0,                          /* 123 */
  0,                          /* 124 */
  0,                          /* 125 */
  0,                          /* 126 */
  0,                          /* 127 */
  0,                          /* 128 */
  0,                          /* 129 */
  0,                          /* 130 */
  0,                          /* 131 */
  0,                          /* 132 */
  0,                          /* 133 */
  0,                          /* 134 */
  0,                          /* 135 */
  0,                          /* 136 */
  0,                          /* 137 */
  0,                          /* 138 */
  0,                          /* 139 */
  0,                          /* 140 */
  0,                          /* 141 */
  0,                          /* 142 */
  0,                          /* 143 */
  0,                          /* 144 */
  0,                          /* 145 */
  0,                          /* 146 */
  0,                          /* 147 */
  0,                          /* 148 */
  0,                          /* 149 */
  0,                          /* 150 */
  0,                          /* 151 */
  0,                          /* 152 */
  0,                          /* 153 */
  0,                          /* 154 */
  0,                          /* 155 */
  usb_isr,                    /* 156 USB */
  cc2538_rf_rx_tx_isr,        /* 157 RFCORE RX/TX */
  cc2538_rf_err_isr,          /* 158 RFCORE Error */
  crypto_isr,                 /* 159 AES */
  default_handler,            /* 160 PKA */
  rtimer_isr,                 /* 161 SM Timer */
  default_handler,            /* 162 MACTimer */
};
/*---------------------------------------------------------------------------*/
/* Linker constructs indicating .data and .bss segment locations */
extern uint8_t _ldata;
extern uint8_t _data;
extern uint8_t _edata;
extern uint8_t _bss;
extern uint8_t _ebss;
/*---------------------------------------------------------------------------*/
/* Weak interrupt handlers. */
void
nmi_handler(void)
{
  reset_handler();
  while(1);
}
/*---------------------------------------------------------------------------*/
void
default_handler(void)
{
  while(1);
}
/*---------------------------------------------------------------------------*/
void
reset_handler(void)
{
  REG(SYS_CTRL_EMUOVR) = 0xFF;

  /* Copy the data segment initializers from flash to SRAM. */
  rom_util_memcpy(&_data, &_ldata, &_edata - &_data);

  /* Zero-fill the bss segment. */
  rom_util_memset(&_bss, 0, &_ebss - &_bss);

  /* call the application's entry point. */
  main();

  /* End here if main () returns */
  while(1);
}
/*---------------------------------------------------------------------------*/

/** @} */

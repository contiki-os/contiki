/*
 * Copyright (c) 2014, Texas Instruments Incorporated - http://www.ti.com/
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 */
/*---------------------------------------------------------------------------*/
/**
 * \addtogroup cc26xx-uart
 * @{
 *
 * \file
 * Implementation of the CC13xx/CC26xx UART driver.
 */
/*---------------------------------------------------------------------------*/
#include "contiki-conf.h"
#include "cc26xx-uart.h"
#include "hw_types.h"
#include "hw_memmap.h"
#include "sys_ctrl.h"
#include "prcm.h"
#include "ioc.h"
#include "uart.h"
#include "lpm.h"
#include "ti-lib.h"
#include "sys/energest.h"

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
/*---------------------------------------------------------------------------*/
/* Which events to trigger a UART interrupt */
#define CC26XX_UART_RX_INTERRUPT_TRIGGERS (UART_INT_RX | UART_INT_RT)

/* All interrupt masks */
#define CC26XX_UART_INTERRUPT_ALL (UART_INT_OE | UART_INT_BE | UART_INT_PE | \
                                   UART_INT_FE | UART_INT_RT | UART_INT_TX | \
                                   UART_INT_RX | UART_INT_CTS)
/*---------------------------------------------------------------------------*/
#define cc26xx_uart_isr UART0IntHandler
/*---------------------------------------------------------------------------*/
static int (*input_handler)(unsigned char c);

//*****************************************************************************
#if UART_TXBUFSIZE

#include <lib/ringbuf16index.h>

typedef struct uart_tx_s{
    //uintptr_t   io_base;
    struct {
        struct ringbuf16index   idx;
        char                    data[UART_TXBUFSIZE];
    }           buf;
} uart_tx_t;
uart_tx_t   uart_txbuf;
#define UIO_BASE(buf)   UART0_BASE

#if (~(UART_TXBUFSIZE-1) & UART_TXBUFSIZE) != UART_TXBUFSIZE
#error UART_TXBUFSIZE MUST be power 2
#endif
void cc26xx_uart_buf_init(void){
    uart_tx_t* tx = &uart_txbuf;
    ringbuf16index_init(&tx->buf.idx, UART_TXBUFSIZE);
}

//* \return = 0 - buffer is empty
int cc26xx_uart_bufsend(void /*struct uart_tx_t* buf*/)
{
    uart_tx_t* tx = &uart_txbuf;
    while (!ringbuf16index_empty(&tx->buf.idx)){
        char tmp = tx->buf.data[tx->buf.idx.get_ptr];
        if (ti_lib_uart_char_put_non_blocking(UIO_BASE(tx), tmp) == (true) ){
            if ( ringbuf16index_get(&tx->buf.idx) >= 0)
                {;}
            else
                //* buffer empty
                break;
        }
        else
            return 1;
    }
    return 0;
}

int cc26xx_uart_send_empty(void){
    uart_tx_t* tx = &uart_txbuf;
    return ringbuf16index_empty(&tx->buf.idx);
}

int cc26xx_uart_buf_free(void){
    uart_tx_t* tx = &uart_txbuf;
    return UART_TXBUFSIZE - ringbuf16index_elements(&tx->buf.idx);
}

int cc26xx_uart_write_bufs(/*struct uart_tx_t* buf*/ const void* data, unsigned len)
{
    uart_tx_t* tx = &uart_txbuf;
    if (cc26xx_uart_send_empty()){
        if (len == 1) {
            if ( ti_lib_uart_char_put_non_blocking(UIO_BASE(tx), *(const char*)data) ){
            return 1;
    }
        }
    }
    //* buf index operations should be atomic
    bool interrupts_disabled = ti_lib_int_master_disable();
    ti_lib_uart_int_enable(UIO_BASE(tx), UART_INT_TX);
    unsigned res = 0;

    if (ringbuf16index_full(&tx->buf.idx)){
        // mark buffer overrun
#ifdef UART_TXBUF_OVERMARK
        unsigned lastp = (tx->buf.idx.put_ptr-1) & tx->buf.idx.mask;
        tx->buf.data[lastp] = UART_TXBUF_OVERMARK;
#endif
    }
    else {

    if(ringbuf16index_empty(&tx->buf.idx)) {
        //* reset buffer, if empty, to have maximum linear free block
        tx->buf.idx.put_ptr = 0;
        tx->buf.idx.get_ptr = 0;
    }
    if (len == 1) {
        tx->buf.data[tx->buf.idx.put_ptr] = *(const char*)data;
    ringbuf16index_put(&tx->buf.idx);
        res = 1;
    }
    else {
        unsigned avail = ringbuf16index_put_free(&tx->buf.idx);
        if (avail >= len)
            avail = len;
        memcpy(tx->buf.data+tx->buf.idx.put_ptr, data, avail);
        res  = avail;
        len -= avail;
        ringbuf16index_putn(&tx->buf.idx, avail);
        if (len > 0){
            avail = ringbuf16index_put_free(&tx->buf.idx);
            if (avail > 0){
            const char* b = (const char*)data;
            if (avail >= len)
                avail = len;
            memcpy(tx->buf.data+tx->buf.idx.put_ptr, b+res, avail);
            res += avail;
                ringbuf16index_putn(&tx->buf.idx, avail);
            }
            else{
#ifdef UART_TXBUF_OVERMARK
                unsigned lastp = (tx->buf.idx.put_ptr-1) & tx->buf.idx.mask;
                // mark buffer overrun
                tx->buf.data[lastp] = '@';
#endif
                ;
            }
        }
    }
    } // else if (ringbuf16index_full(&tx->buf.idx))

    cc26xx_uart_bufsend();
    if(!interrupts_disabled) {
      ti_lib_int_master_enable();
    }

    return res;
}


int cc26xx_uart_write_buf(/*struct uart_tx_t* buf*/ char x){
    uart_tx_t* tx = &uart_txbuf;
    while (ringbuf16index_full(&tx->buf.idx)){
            //volatile uart_tx_t* vtx = (volatile uart_tx_t*)tx;
            while (ringbuf16index_full(&tx->buf.idx)) ;
    }

    unsigned tmp = x;
    return cc26xx_uart_write_bufs(&tmp, 1);
}

#else

#define cc26xx_uart_buf_init(x)
#define cc26xx_uart_bufsend(x) 0
#define cc26xx_uart_send_empty(x) 1

inline
int cc26xx_uart_write_buf(char x){
    ti_lib_uart_char_put(UART0_BASE, x);
    return 1;
}

#define cc26xx_uart_buf_free(x) 0

inline
int cc26xx_uart_write_bufs(const void* data, unsigned len){
    return ti_lib_uart_char_put_non_blocking(UART0_BASE, *(const char*)data);
}

#endif //UART_TXBUFSIZE
/*---------------------------------------------------------------------------*/
static bool
usable_rx(void)
{
  if(BOARD_IOID_UART_RX == IOID_UNUSED ||
     CC26XX_UART_CONF_ENABLE == 0) {
    return false;
  }

  return true;
}
/*---------------------------------------------------------------------------*/
static bool
usable_tx(void)
{
  if(BOARD_IOID_UART_TX == IOID_UNUSED ||
     CC26XX_UART_CONF_ENABLE == 0) {
    return false;
  }

  return true;
}
/*---------------------------------------------------------------------------*/
static void
power_and_clock(void)
{
  /* Power on the SERIAL PD */
  ti_lib_prcm_power_domain_on(PRCM_DOMAIN_SERIAL);
  while(ti_lib_prcm_power_domain_status(PRCM_DOMAIN_SERIAL)
        != PRCM_DOMAIN_POWER_ON);

  /* Enable UART clock in active mode */
  ti_lib_prcm_peripheral_run_enable(PRCM_PERIPH_UART0);
  ti_lib_prcm_peripheral_sleep_enable(PRCM_PERIPH_UART0);
  ti_lib_prcm_load_set();
  while(!ti_lib_prcm_load_get());
}
/*---------------------------------------------------------------------------*/
/*
 * Returns 0 if either the SERIAL PD is off, or the PD is on but the run mode
 * clock is gated. If this function would return 0, accessing UART registers
 * will return a precise bus fault. If this function returns 1, it is safe to
 * access UART registers.
 *
 * This function only checks the 'run mode' clock gate, since it can only ever
 * be called with the MCU in run mode.
 */
static bool
accessible(void)
{
  /* First, check the PD */
  if(ti_lib_prcm_power_domain_status(PRCM_DOMAIN_SERIAL)
     != PRCM_DOMAIN_POWER_ON) {
    return false;
  }

  /* Then check the 'run mode' clock gate */
  if(!(HWREG(PRCM_BASE + PRCM_O_UARTCLKGR) & PRCM_UARTCLKGR_CLK_EN)) {
    return false;
  }

  return true;
}
/*---------------------------------------------------------------------------*/
static void
disable_interrupts(void)
{
  /* Acknowledge UART interrupts */
  ti_lib_int_disable(INT_UART0_COMB);

  /* Disable all UART module interrupts */
  ti_lib_uart_int_disable(UART0_BASE, CC26XX_UART_INTERRUPT_ALL);

  /* Clear all UART interrupts */
  ti_lib_uart_int_clear(UART0_BASE, CC26XX_UART_INTERRUPT_ALL);
}
/*---------------------------------------------------------------------------*/
static void
enable_interrupts(void)
{
  /* Clear all UART interrupts */
  ti_lib_uart_int_clear(UART0_BASE, CC26XX_UART_INTERRUPT_ALL);

#if UART_TXBUFSIZE
  if (!cc26xx_uart_send_empty())
      ti_lib_uart_int_enable(UIO_BASE(tx), UART_INT_TX);
#endif

  /* Enable RX-related interrupts only if we have an input handler */
  if(input_handler) {
    /* Configure which interrupts to generate: FIFO level or after RX timeout */
    ti_lib_uart_int_enable(UART0_BASE, CC26XX_UART_RX_INTERRUPT_TRIGGERS);
  }

  /* Acknowledge UART interrupts */
  ti_lib_int_enable(INT_UART0_COMB);
}
/*---------------------------------------------------------------------------*/
static void
configure(void)
{
  uint32_t ctl_val = UART_CTL_UARTEN | UART_CTL_TXE;
  /*
   * Make sure the TX pin is output / high before assigning it to UART control
   * to avoid falling edge glitches
   */
  ti_lib_ioc_pin_type_gpio_output(BOARD_IOID_UART_TX);
  ti_lib_gpio_set_dio(BOARD_IOID_UART_TX);

  /*
   * Map UART signals to the correct GPIO pins and configure them as
   * hardware controlled.
   */
  ti_lib_ioc_pin_type_uart(UART0_BASE, BOARD_IOID_UART_RX, BOARD_IOID_UART_TX,
                           BOARD_IOID_UART_CTS, BOARD_IOID_UART_RTS);

  /* Configure the UART for 115,200, 8-N-1 operation. */
  ti_lib_uart_config_set_exp_clk(UART0_BASE, ti_lib_sys_ctrl_clock_get(),
                                 CC26XX_UART_CONF_BAUD_RATE,
                                 (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE |
                                  UART_CONFIG_PAR_NONE));

  /*
   * Generate an RX interrupt at FIFO 1/2 full.
   * Generate TX interrupt as rare as possible
   */
  ti_lib_uart_fifo_level_set(UART0_BASE, UART_FIFO_TX1_8, UART_FIFO_RX4_8);

  /* Enable FIFOs */
  HWREG(UART0_BASE + UART_O_LCRH) |= UART_LCRH_FEN;

  if(input_handler) {
    ctl_val += UART_CTL_RXE;
  }

  /* Enable TX, RX (conditionally), and the UART. */
  HWREG(UART0_BASE + UART_O_CTL) = ctl_val;
}
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
static uint8_t
lpm_request(void)
{
    if(usable_tx() == false) {
        return LPM_MODE_MAX_SUPPORTED;
    }

    if(accessible() == false) {
        return LPM_MODE_MAX_SUPPORTED;
    }

    if(cc26xx_uart_send_empty()) {
        if(!ti_lib_uart_busy(UART0_BASE))
            return LPM_MODE_MAX_SUPPORTED;
        else
            // when TX buffer empty, TX IRQ disabled. So, only way to handle
            //  end of transmition is poling uart BUSY state.
            return LPM_MODE_AWAKE;
    }

    //* have some data buffered, wait until tx buffer empty
    return LPM_MODE_SLEEP;
}

static void
lpm_drop_handler(uint8_t mode)
{
  /*
   * First, wait for any outstanding TX to complete. If we have an input
   * handler, the SERIAL PD will be kept on and the UART module clock will
   * be enabled under sleep as well as deep sleep. In theory, this means that
   * we shouldn't lose any outgoing bytes, but we actually do on occasion.
   * This byte loss may (or may not) be related to the freezing of IO latches
   * between MCU and AON when we drop to deep sleep. This here is essentially a
   * workaround
   */
  if(accessible() == true) {
    while(ti_lib_uart_busy(UART0_BASE));
  }

  /*
   * If we have a registered input_handler then we need to retain RX
   * capability. Thus, if this is not a shutdown notification and we have an
   * input handler, we do nothing
   */
  if((mode != LPM_MODE_SHUTDOWN) && (input_handler != NULL)) {
    return;
  }

  /*
   * If we reach here, we either don't care about staying awake or we have
   * received a shutdown notification
   *
   * Only touch UART registers if the module is powered and clocked
   */
  if(accessible() == true) {
    /* Disable the module */
    ti_lib_uart_disable(UART0_BASE);

    /* Disable all UART interrupts and clear all flags */
    disable_interrupts();
  }

  /*
   * Always stop the clock in run mode. Also stop in Sleep and Deep Sleep if
   * this is a request for full shutdown
   */
  ti_lib_prcm_peripheral_run_disable(PRCM_PERIPH_UART0);
  if(mode == LPM_MODE_SHUTDOWN) {
    ti_lib_prcm_peripheral_sleep_disable(PRCM_PERIPH_UART0);
    ti_lib_prcm_peripheral_deep_sleep_disable(PRCM_PERIPH_UART0);
  }
  ti_lib_prcm_load_set();
  while(!ti_lib_prcm_load_get());

  /* Set pins to low leakage configuration in preparation for deep sleep */
  lpm_pin_set_default_state(BOARD_IOID_UART_TX);
  lpm_pin_set_default_state(BOARD_IOID_UART_RX);
  lpm_pin_set_default_state(BOARD_IOID_UART_CTS);
  lpm_pin_set_default_state(BOARD_IOID_UART_RTS);
}
/*---------------------------------------------------------------------------*/
/* Declare a data structure to register with LPM. */
LPM_MODULE(uart_module, lpm_request, lpm_drop_handler, NULL, LPM_DOMAIN_NONE);
/*---------------------------------------------------------------------------*/
static void
enable(void)
{
  power_and_clock();

  /* Make sure the peripheral is disabled */
  ti_lib_uart_disable(UART0_BASE);

  /* Disable all UART interrupts and clear all flags */
  disable_interrupts();

  /* Setup pins, Baud rate and FIFO levels */
  configure();

  /* Enable UART interrupts */
  enable_interrupts();
}
/*---------------------------------------------------------------------------*/
void
cc26xx_uart_init()
{
  bool interrupts_disabled;

  /* Return early if disabled by user conf or if ports are misconfigured */
  if(!usable_rx() && !usable_tx()) {
    return;
  }

  cc26xx_uart_buf_init();

  /* Disable Interrupts */
  interrupts_disabled = ti_lib_int_master_disable();

  /* Register ourselves with the LPM module */
  lpm_register_module(&uart_module);

  /* Only TX and EN to start with. RX will be enabled only if needed */
  input_handler = NULL;

  /*
   * init() won't actually fire up the UART. We turn it on only when (and if)
   * it gets requested, either to enable input or to send out a character
   *
   * Thus, we simply re-enable processor interrupts here
   */
  if(!interrupts_disabled) {
    ti_lib_int_master_enable();
  }
  lpm_drop_handler(LPM_MODE_SHUTDOWN);
}
/*---------------------------------------------------------------------------*/
void
cc26xx_uart_write_byte(uint8_t c)
{
  /* Return early if disabled by user conf or if ports are misconfigured */
  if(usable_tx() == false) {
    return;
  }

  if(accessible() == false) {
    enable();
  }

  cc26xx_uart_write_buf(c);
}
/*---------------------------------------------------------------------------*/
/**
 * \brief nonblocking put bytes to UART
 * \param data The characters to transmit
 * \param len  amount of characters to transmit
 * \return number of writen characters
 */
int cc26xx_uart_write_bytes(const void* data, unsigned len){
    /* Return early if disabled by user conf or if ports are misconfigured */
    if(usable_tx() == false) {
      return -1;
    }

    if(accessible() == false) {
      enable();
    }
    return cc26xx_uart_write_bufs(data, len);
}

/*---------------------------------------------------------------------------*/
void
cc26xx_uart_set_input(int (*input)(unsigned char c))
{
  input_handler = input;

  /* Return early if disabled by user conf or if ports are misconfigured */
  if(usable_rx() == false) {
    return;
  }

  if(input == NULL) {
    /* Let the SERIAL PD power down */
    uart_module.domain_lock = LPM_DOMAIN_NONE;

    /* Disable module clocks under sleep and deep sleep */
    ti_lib_prcm_peripheral_sleep_disable(PRCM_PERIPH_UART0);
    ti_lib_prcm_peripheral_deep_sleep_disable(PRCM_PERIPH_UART0);
  } else {
    /* Request the SERIAL PD to stay on during deep sleep */
    uart_module.domain_lock = LPM_DOMAIN_SERIAL;

    /* Enable module clocks under sleep and deep sleep */
    ti_lib_prcm_peripheral_sleep_enable(PRCM_PERIPH_UART0);
    ti_lib_prcm_peripheral_deep_sleep_enable(PRCM_PERIPH_UART0);
  }

  ti_lib_prcm_load_set();
  while(!ti_lib_prcm_load_get());

  enable();

  return;
}
/*---------------------------------------------------------------------------*/
uint8_t
cc26xx_uart_busy(void)
{
  /* Return early if disabled by user conf or if ports are misconfigured */
  if(usable_tx() == false) {
    return UART_IDLE;
  }

  /* If the UART is not accessible, it is not busy */
  if(accessible() == false) {
    return UART_IDLE;
  }

  return ti_lib_uart_busy(UART0_BASE);
}
/*---------------------------------------------------------------------------*/
/**
 * \brief Returns the UART full status
 * \return 0  - UART if full, and blocks for write
 * \return >0 - UART can write bytes without block
 *
 * ti_lib_uart_busy() will access UART registers. It is our responsibility
 * to first make sure the UART is accessible before calling it. Hence this
 * wrapper.
 *
 * Return values are defined in CC26xxware's uart.h:UARTSpaceAvail
 */
int cc26xx_uart_space_avail(void){
    /* Return early if disabled by user conf or if ports are misconfigured */
    if(usable_tx() == false) {
      return -1;
    }

    /* If the UART is not accessible, it is not busy */
    if(accessible() == false) {
      return -1;
    }

    unsigned res = cc26xx_uart_buf_free();
    if (res > 0)
        return res;
    if(ti_lib_uart_space_avail(UART0_BASE))
        return 1;
    if (!ti_lib_uart_busy(UART0_BASE))
        return 1;
    return 0;
}
/*---------------------------------------------------------------------------*/
void
cc26xx_uart_isr(void)
{
  char the_char;
  uint32_t flags;

  ENERGEST_ON(ENERGEST_TYPE_IRQ);

  if(accessible() == false)
  power_and_clock();

  /* Read out the masked interrupt status */
  flags = ti_lib_uart_int_status(UART0_BASE, true);

  /* Clear all UART interrupt flags */
  ti_lib_uart_int_clear(UART0_BASE, CC26XX_UART_INTERRUPT_ALL);

  if((flags & CC26XX_UART_RX_INTERRUPT_TRIGGERS) != 0) {
    /*
     * If this was a FIFO RX or an RX timeout, read all bytes available in the
     * RX FIFO.
     */
    while(ti_lib_uart_chars_avail(UART0_BASE)) {
      the_char = ti_lib_uart_char_get_non_blocking(UART0_BASE);

      if(input_handler != NULL) {
        input_handler((unsigned char)the_char);
      }
    }
  }

  if ((flags & UART_INT_TX) != 0) {
      if (cc26xx_uart_bufsend() == 0)
          ti_lib_uart_int_disable(UART0_BASE, UART_INT_TX);
  }

  ENERGEST_OFF(ENERGEST_TYPE_IRQ);
}
/*---------------------------------------------------------------------------*/
/** @} */

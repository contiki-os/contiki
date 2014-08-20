/**
 * \file
 * \author
 */

#include "K60.h"
#include "power-modes.h"
#include "llwu.h"
#include "lib/list.h"

#include <stddef.h>

#define DEBUG 0
#if DEBUG
#include "stdio.h"
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif

LIST(llwu);
static char allow_deep_sleep = 1;

static void update_llwu();

volatile uint8_t llwu_inhibit_lls_sema = 0;
volatile uint8_t llwu_inhibit_vlps_sema = 0;
volatile uint8_t llwu_inhibit_stop_sema = 0;

/* TODO(henrik) Add callbacks before entering deep sleep. */
/*---------------------------------------------------------------------------*/
void
llwu_init()
{
  list_init(llwu);
  /* Setup Low Leakage Wake-up Unit (LLWU) */
  SIM_SCGC4 |= SIM_SCGC4_LLWU_MASK;   /* Enable LLWU clock gate */

  power_modes_init();

  update_llwu();
  /** \todo Symbolic names for NVIC IRQ flags */
  NVICISER0 |= NVIC_ISER_SETENA(1 << 21); /* Enable LLWU interrupt */
}
/*---------------------------------------------------------------------------*/
void
llwu_register(llwu_control_t *c)
{
  list_add(llwu, c);
  PRINTF("LLWU: new controller\n");
}
/*---------------------------------------------------------------------------*/
void
llwu_set_allow(llwu_control_t *c, char allow)
{
  c->allow_llwu = allow;
  update_llwu();
  PRINTF("LLWU: allow LLWU %d\n", allow_deep_sleep);
}
/*---------------------------------------------------------------------------*/
static void
update_llwu()
{
  llwu_control_t *n;
  allow_deep_sleep = 1;
  for(n = list_head(llwu); n != NULL;) {
    if(!n->allow_llwu) {
      allow_deep_sleep = 0;
      return;
    }
    n = list_item_next(n);
  }
}
/*---------------------------------------------------------------------------*/
void
llwu_sleep(void)
{
  PRINTF("LLWU: sleep %u....\n", allow_deep_sleep);
  /* Check if any UARTs are currently receiving data, if we go to STOP mode we
   * will lose the byte in progress. */
  /* It is necessary to check the SIM_SCGCx registers to avoid hardfaulting when
   * we try to read a disabled peripheral */
  if(!allow_deep_sleep ||
     ((SIM_SCGC4 & SIM_SCGC4_UART0_MASK) && (UART0_S2 & UART_S2_RAF_MASK)) ||
     ((SIM_SCGC4 & SIM_SCGC4_UART1_MASK) && (UART1_S2 & UART_S2_RAF_MASK)) ||
     ((SIM_SCGC4 & SIM_SCGC4_UART2_MASK) && (UART2_S2 & UART_S2_RAF_MASK)) ||
     ((SIM_SCGC4 & SIM_SCGC4_UART3_MASK) && (UART3_S2 & UART_S2_RAF_MASK)) ||
     ((SIM_SCGC1 & SIM_SCGC1_UART4_MASK) && (UART4_S2 & UART_S2_RAF_MASK)) ||
     ((SIM_SCGC1 & SIM_SCGC1_UART5_MASK) && (UART5_S2 & UART_S2_RAF_MASK))) {
    power_mode_wait();
  }
  /* FIXME: Do we need to disable interrupts here? */
  else if(llwu_inhibit_stop_sema != 0) {
    /* STOP inhibited, use WAIT instead */
    PRINTF("LLWU: STOP inhibited\n");
    power_mode_wait();
  } else if(llwu_inhibit_vlps_sema != 0) {
    /* VLPS inhibited, use STOP */
    power_mode_stop();
  } else if(llwu_inhibit_lls_sema != 0) {
    /* LLS inhibited, use VLPS */
    power_mode_vlps();
  } else {
    /* free to stop everything */
    power_mode_lls();
  }
}
/*---------------------------------------------------------------------------*/
void
llwu_enable_wakeup_module(const llwu_wakeup_module_t module)
{
  if(module < LLWU_WAKEUP_MODULE_END) {
    LLWU_ME |= (1 << (const uint8_t)module);
    PRINTF("LLWU_ME 0x%02x\n", LLWU_ME);
  }
}
void
llwu_disable_wakeup_module(const llwu_wakeup_module_t module)
{
  if(module < LLWU_WAKEUP_MODULE_END) {
    LLWU_ME = (LLWU_ME & ~(1 << (const uint8_t)module));
    PRINTF("LLWU_ME 0x%02x\n", LLWU_ME);
  }
}
void
llwu_set_wakeup_pin(const llwu_wakeup_pin_t pin, const llwu_wakeup_edge_t edge)
{
  uint8_t tmp;
  if(pin >= LLWU_WAKEUP_PIN_END) {
    /* invalid pin number */
    asm("bkpt #1\n");
    return;
  }
  /* LLWU pin enable registers are sequential in the address space */

  tmp = *(&LLWU_PE1 + (((const uint8_t)pin) / LLWU_WAKEUP_PIN_REG_SIZE));
  /* Clear the bits in the field we want to modify */
  tmp = (tmp & ~(LLWU_WAKEUP_EDGE_MASK << ((pin % LLWU_WAKEUP_PIN_REG_SIZE) *
                                           LLWU_WAKEUP_EDGE_WIDTH)));
  /* Set the new value */
  tmp |= ((edge & LLWU_WAKEUP_EDGE_MASK) << ((pin % LLWU_WAKEUP_PIN_REG_SIZE) *
                                             LLWU_WAKEUP_EDGE_WIDTH));
  *(&LLWU_PE1 + (((const uint8_t)pin) / LLWU_WAKEUP_PIN_REG_SIZE)) = tmp;
  PRINTF("LLTU 1 0x%02x\n", LLWU_PE1);
  PRINTF("LLTU 2 0x%02x\n", LLWU_PE2);
  PRINTF("LLTU 3 0x%02x\n", LLWU_PE3);
  PRINTF("LLTU 4 0x%02x\n", LLWU_PE4);
}
void __attribute__((interrupt))
_isr_llwu(void)
{
  /* TODO(henrik) Dont know if this is really the correct way to handle the flags. */
  /* Clear LLWU flags */
  LLWU_F1 = 0xFF;
  LLWU_F2 = 0xFF;
  LLWU_F3 = 0xFF;
}

/**
 * \file
 * \author
 */

#include "K60.h"
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

// TODO(henrik) Add callbacks before entering deep sleep.
/*---------------------------------------------------------------------------*/
void
llwu_init()
{
  list_init(llwu);
  /* Setup Low Leakage Wake-up Unit (LLWU) */
  SIM_SCGC4 |= SIM_SCGC4_LLWU_MASK;   /* Enable LLWU clock gate */

#if K60_CPU_REV == 1

  /* Select low power mode Low Leakage Stop (LLS) */

  /* Clear LLS protection */
  MC_PMPROT |= MC_PMPROT_ALLS_MASK;

  /* Enable Low Power Wake Up Interrupt and LLS */
  MC_PMCTRL = MC_PMCTRL_LPWUI_MASK | MC_PMCTRL_LPLLSM(0b011);
#else
  /* Mode Controller changed name to System Mode Controller (SMC) in rev 2 */

  /* Clear LLS protection */
  SMC_PMPROT |= SMC_PMPROT_ALLS_MASK;

  /* Enable Low Power Wake Up Interrupt and LLS */
  SMC_PMCTRL = SMC_PMCTRL_LPWUI_MASK | SMC_PMCTRL_STOPM(0b011);
#endif

  update_llwu();
  /** \todo Symbolic names for NVIC IRQ flags */
  NVICISER0 |= NVIC_ISER_SETENA(1 << 21); /* Enable LLWU interrupt */
}
/*---------------------------------------------------------------------------*/
void
llwu_register(llwu_control_t* c)
{
  list_add(llwu, c);
  PRINTF("LLWU: new controller\n");
}
/*---------------------------------------------------------------------------*/
void
llwu_set_allow(llwu_control_t* c, char allow)
{
  c->allow_llwu = allow;
  update_llwu();
  PRINTF("LLWU: allow LLWU %d\n", allow_deep_sleep);
}
/*---------------------------------------------------------------------------*/
static void
update_llwu()
{
  llwu_control_t* n;
  allow_deep_sleep = 1;
  for (n = list_head(llwu); n != NULL;)
  {
    if (!n->allow_llwu)
    {
      allow_deep_sleep = 0;
      return;
    }
    n = list_item_next(n);
  }
}
/*---------------------------------------------------------------------------*/
void
llwu_sleep(void) {
  PRINTF("LLWU: sleep %u....\n", allow_deep_sleep);
  SCB_SCR = (SCB_SCR & ~SCB_SCR_SLEEPDEEP_MASK) | (allow_deep_sleep << SCB_SCR_SLEEPDEEP_SHIFT);
  asm("WFI");
}
/*---------------------------------------------------------------------------*/
void
llwu_enable_wakeup_source(enum LLWU_WAKEUP_SOURCE s)
{
  if (s < 49)
  {
    *(&LLWU_PE1 + ((s-1)/12)) |= ((s-1)%3+1) << ((int)(((s%12)-1)/3)*2);
  }
  else if (s == 49)
  {
    LLWU_ME |= 1; // LPT
  }
  PRINTF("LLTU 1 %X\n", LLWU_PE1);
  PRINTF("LLTU 2 %X\n", LLWU_PE2);
  PRINTF("LLTU 3 %X\n", LLWU_PE3);
  PRINTF("LLTU 4 %X\n", LLWU_PE4);
}
/*---------------------------------------------------------------------------*/
void
llwu_enable_disable_source(enum LLWU_WAKEUP_SOURCE s)
{
  LLWU_ME = LLWU_ME & (~s & 0xFF);
}

void __attribute__((interrupt)) _isr_llwu(void)
{
  LPTMR0_CSR |= 0x80;
  // TODO(henrik) Dont know if this is really the correct way to handle the flags.
  LLWU_F1 = 0xFF;
  LLWU_F2 = 0xFF;
  LLWU_F3 = 0xFF;
}


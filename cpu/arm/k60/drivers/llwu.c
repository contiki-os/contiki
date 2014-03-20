/**
 * \file
 * \author
 */

#include "K60.h"
#include "llwu.h"
#include "lib/list.h"

#include <stddef.h>

#define DEBUG 1
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
  // Setup Low Leakage Wake-up Unit (LLWU)
  SIM_SCGC4 |= SIM_SCGC4_LLWU_MASK;   // Enable LLWU clock gate

  // Select low power mode Low Leakage Stop (LLS)
  MC_PMPROT = 0x10;       // Clear LLS protection
  MC_PMCTRL = 0x83;       // LLS
  update_llwu();
  NVICISER0 |= 0x00200000;        // Enable LLWU interrupt
}
/*---------------------------------------------------------------------------*/
void
llwu_register(llwu_control* c)
{
  list_add(llwu, c);
  PRINTF("LLWU: new controller\n");
}
/*---------------------------------------------------------------------------*/
void
llwu_set_allow(llwu_control* c, char allow)
{
  c->allow_llwu = allow;
  update_llwu();
  PRINTF("LLWU: allow LLWU %d\n", allow_deep_sleep);
}
/*---------------------------------------------------------------------------*/
static void
update_llwu()
{
  llwu_control* n;
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
  SCB_SCR   = (SCB_SCR & ~0x04) | (allow_deep_sleep << 2);
  asm("WFI");
}
/*---------------------------------------------------------------------------*/
void
llwu_enable_wakeup_source(enum LLWU_WAKEUP_SOURCE s)
{
  LLWU_ME |= s;
}
/*---------------------------------------------------------------------------*/
void
llwu_enable_disable_source(enum LLWU_WAKEUP_SOURCE s)
{
  LLWU_ME = LLWU_ME & (~s & 0xFF);
}

void __attribute__((interrupt( irq ))) _isr_low_leakage_wakeup(void)
{
  LLWU_F1 |= 0x80;
  PRINTF("LLWU: interrupt\n");
}

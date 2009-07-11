#include <AT91SAM7S64.h>
#include <stdint.h>

#define USED __attribute__((used))
#define USED_NAKED __attribute__((used,naked))
#define USED_INT(type) __attribute__((used,interrupt(#type)))
#if MCK > 30000000
#define FLASH_CYCLES AT91C_MC_FWS_1FWS
#else
#define FLASH_CYCLES AT91C_MC_FWS_0FWS
#endif

#ifndef MAIN_OSC_FREQ
#define MAIN_OSC_FREQ 18432000
#endif

#if MAIN_OSC_FREQ != 18432000
#error Unsupported main oscilator frequency
#endif

#if MCK == 23961600
#define PLL_DIV 5
#define PLL_MUL 26
#define PLL_USBDIV_EXP 1
#define MCK_DIV_EXP 2
#elif MCK == 47923200
#define PLL_DIV 5
#define PLL_MUL 26
#define PLL_USBDIV_EXP 1
#define MCK_DIV_EXP 1
#else
#error "Unsupported main clock frequency"
#endif

#define PLL_FREQ ((MAIN_OSC_FREQ * PLL_MUL) / PLL_DIV) 

#if PLL_FREQ > 180000000
#error "PLL frequency too high"
#elif PLL_FREQ < 80000000
#error "PLL frequency too low"
#endif

#if PLL_FREQ > 155000000
#define PLL_RANGE AT91C_CKGR_OUT_2
#else
#define PLL_RANGE AT91C_CKGR_OUT_0
#endif

#if PLL_USBDIV > 2
#error "PLL frequency too high for USB"
#endif

#define USB_FREQ (PLL_FREQ / (1<<PLL_USBDIV_EXP))
#if USB_FREQ > 48120000 || USB_FREQ < 47880000
#warning "USB frequency outside limits"
#endif

#if MCK * (1<<MCK_DIV_EXP) != PLL_FREQ
#error PLL frequency is not a the correct multiple of the main clock
#endif

/* CPU modes */
#define Mode_USR 0x10
#define Mode_FIQ 0x11
#define Mode_IRQ 0x12
#define Mode_SVC 0x13
#define Mode_ABT 0x17
#define Mode_UND 0x1B
#define Mode_SYS 0x1F

/* IRQ disable bit */
#define I_Bit 0x80
/* FIQ disable bit */
#define F_Bit 0x40

#define SET_MODE_STACK(mode, stack_end) \
asm("msr CPSR_c, %0\nldr sp, =" #stack_end "\n" ::"i" ((mode)|I_Bit|F_Bit))

#define ENABLE_INTS() \
asm("mrs  r0, cpsr\nand r0, %0\nmsr cpsr_c, r0\n"::"i" (~(I_Bit|F_Bit)):"r0")


extern void *USR_Stack_End;
extern void *UND_Stack_End;
extern void *ABT_Stack_End;
extern void *FIQ_Stack_End;
extern void *IRQ_Stack_End;
extern void *SVC_Stack_End;

extern uint8_t _data[];
extern uint8_t _etext[];
extern uint8_t _edata[];

extern uint8_t __bss_start[];
extern uint8_t __bss_end[];

extern int
main(int argc, char *argv[]);

static void
copy_initialized(void)
{
  uint8_t *ram = _data;
  uint8_t *rom = _etext;
  while(ram < _edata) {
    *ram++ = *rom++;
  }
}

static void
clear_bss(void)
{
  uint8_t *m = __bss_start;
  while(m < __bss_end) {
    *m++ = 0;
  }
}

static void
Reset_handler(void) USED_NAKED;

static void
SPU_handler(void) __attribute__((interrupt("IRQ")));

static void
Reset_handler(void)
{
  /* Setup flash timing */
  *AT91C_MC_FMR = FLASH_CYCLES | (MCK / 666666 + 1);

  /* Disable watchdog */
  *AT91C_WDTC_WDMR = AT91C_WDTC_WDDIS;

  /* Setup reset controller */
  *AT91C_RSTC_RMR = (0xa5<<24) | AT91C_RSTC_URSTS;
  
  /* Start main oscilator */
  *AT91C_CKGR_MOR = AT91C_CKGR_MOSCEN | (6<<8);

  /* Wait for oscillator to start */
  while(!(*AT91C_PMC_SR & AT91C_PMC_MOSCS));

  /* Setup PLL */
  *AT91C_CKGR_PLLR = ((PLL_USBDIV_EXP << 28) | ((PLL_MUL-1)<<16) | PLL_RANGE
		      | (28<<8) | PLL_DIV);

  /* Wait for PLL to lock */
  while(!(*AT91C_PMC_SR & AT91C_PMC_LOCK));

  *AT91C_PMC_MCKR = (MCK_DIV_EXP << 2);
  while(!(*AT91C_PMC_SR & AT91C_PMC_MCKRDY));
  *AT91C_PMC_MCKR |= AT91C_PMC_CSS_PLL_CLK;
  while(!(*AT91C_PMC_SR & AT91C_PMC_MCKRDY));
  SET_MODE_STACK(Mode_UND, UND_Stack_End);
  SET_MODE_STACK(Mode_ABT, ABT_Stack_End);
  SET_MODE_STACK(Mode_FIQ, FIQ_Stack_End);
  SET_MODE_STACK(Mode_IRQ, IRQ_Stack_End);
  SET_MODE_STACK(Mode_SVC, SVC_Stack_End);
#ifdef RUN_AS_SYSTEM
  SET_MODE_STACK(Mode_SYS, USR_Stack_End);
#else
  SET_MODE_STACK(Mode_USR, USR_Stack_End);
#endif
  copy_initialized();
  clear_bss();
  
  *AT91C_AIC_SPU = (uint32_t)SPU_handler;
  ENABLE_INTS();
  main(0,0);
  while(1);
}

static void
Undef_handler(void) USED_INT(UNDEF);

static void
Undef_handler(void)
{
}

static void
SWI_handler(void) USED_INT(SWI);

static void
SWI_handler(void)
{
}

static void
PAbt_handler(void) USED_INT(ABORT);

static void
PAbt_handler(void)
{
}

static void
DAbt_handler(void) USED_INT(ABORT);

static void
DAbt_handler(void) 
{
}


static void
SPU_handler(void)
{
  *AT91C_AIC_EOICR = 0;
}

static void
Vectors(void) __attribute__ ((naked, section(".vectrom")));

static void
Vectors(void)
{
  asm("ldr pc, =Reset_handler\n");
  asm("ldr pc, =Undef_handler\n");
  asm("ldr pc, =SWI_handler\n");
  asm("ldr pc, =PAbt_handler\n");
  asm("ldr pc, =DAbt_handler\n");
  asm("nop\n");  
  asm("ldr pc,[pc,#-0xf20]\n"); /* Vector From AIC_IVR */
  asm("ldr pc,[pc,#-0xf20]\n"); /* Vector From AIC_FVR */
}



void foo_dummy()
{
  Vectors();
}

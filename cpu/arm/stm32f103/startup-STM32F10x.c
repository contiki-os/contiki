#include <stdint.h>
#include <stm32f10x_map.h>


extern int main(void);

typedef void (*ISR_func)(void);

  
#define SECTION(x) __attribute__ ((section(#x)))
#define ISR_VECTOR_SECTION SECTION(.isr_vector)
static void sys_reset(void) __attribute__((naked));
void NMI_handler(void) __attribute__((interrupt));
void HardFault_handler(void) __attribute__((interrupt));
void MemManage_handler(void) __attribute__((interrupt));
void BusFault_handler(void) __attribute__((interrupt));
void UsageFault_handler(void) __attribute__((interrupt));

static void unhandled_int(void) __attribute__((interrupt));

#define UNHANDLED_ALIAS __attribute__((weak, alias("unhandled_int")));
void Main_Stack_End(void);
void HardFault_handler(void)__attribute__((weak, alias("dHardFault_handler")));
void MemManage_handler(void)__attribute__((weak, alias("dMemManage_handler")));
void BusFault_handler(void) __attribute__((weak, alias("dBusFault_handler")));
void UsageFault_handler(void)__attribute__((weak, alias("dUsageFault_handler")));
 void Reserved_handler(void) UNHANDLED_ALIAS;
 void SVCall_handler(void) UNHANDLED_ALIAS;
 void DebugMonitor_handler(void) UNHANDLED_ALIAS;
 void PendSV_handler(void) UNHANDLED_ALIAS;
 void SysTick_handler(void) UNHANDLED_ALIAS;
 void WWDG_handler(void) UNHANDLED_ALIAS;
 void PVD_handler(void) UNHANDLED_ALIAS;
 void TAMPER_handler(void) UNHANDLED_ALIAS;
 void RTC_handler(void) UNHANDLED_ALIAS;
 void FLASH_handler(void) UNHANDLED_ALIAS;
 void RCC_handler(void) UNHANDLED_ALIAS;
 void EXTI0_handler(void) UNHANDLED_ALIAS;
 void EXTI1_handler(void) UNHANDLED_ALIAS;
 void EXTI2_handler(void) UNHANDLED_ALIAS;
 void EXTI3_handler(void) UNHANDLED_ALIAS;
 void EXTI4_handler(void) UNHANDLED_ALIAS;
 void DMA1_Channel1_handler(void) UNHANDLED_ALIAS;
 void DMA1_Channel2_handler(void) UNHANDLED_ALIAS;
 void DMA1_Channel3_handler(void) UNHANDLED_ALIAS;
 void DMA1_Channel4_handler(void) UNHANDLED_ALIAS;
 void DMA1_Channel5_handler(void) UNHANDLED_ALIAS;
 void DMA1_Channel6_handler(void) UNHANDLED_ALIAS;
 void DMA1_Channel7_handler(void) UNHANDLED_ALIAS;
 void ADC1_2_handler(void) UNHANDLED_ALIAS;
 void USB_HP_CAN_TX_handler(void) UNHANDLED_ALIAS;
 void USB_LP_CAN_RX0_handler(void) UNHANDLED_ALIAS;
 void CAN_RX1_handler(void) UNHANDLED_ALIAS;
 void CAN_SCE_handler(void) UNHANDLED_ALIAS;
 void EXTI9_5_handler(void) UNHANDLED_ALIAS;
 void TIM1_BRK_handler(void) UNHANDLED_ALIAS;
 void TIM1_UP_handler(void) UNHANDLED_ALIAS;
 void TIM1_TRG_COM_handler(void) UNHANDLED_ALIAS;
 void TIM1_CC_handler(void) UNHANDLED_ALIAS;
 void TIM2_handler(void) UNHANDLED_ALIAS;
 void TIM3_handler(void) UNHANDLED_ALIAS;
 void TIM4_handler(void) UNHANDLED_ALIAS;
 void I2C1_EV_handler(void) UNHANDLED_ALIAS;
 void I2C1_ER_handler(void) UNHANDLED_ALIAS;
 void I2C2_EV_handler(void) UNHANDLED_ALIAS;
 void I2C2_ER_handler(void) UNHANDLED_ALIAS;
 void SPI1_handler(void) UNHANDLED_ALIAS;
 void SPI2_handler(void) UNHANDLED_ALIAS;
 void USART1_handler(void) UNHANDLED_ALIAS;
 void USART2_handler(void) UNHANDLED_ALIAS;
 void USART3_handler(void) UNHANDLED_ALIAS;
 void EXTI15_10_handler(void) UNHANDLED_ALIAS;
 void RTCAlarm_handler(void) UNHANDLED_ALIAS;
 void USBWakeup_handler(void) UNHANDLED_ALIAS; 
 void TIM8_BRK_handler(void) UNHANDLED_ALIAS;
 void TIM8_UP_handler(void) UNHANDLED_ALIAS;
 void TIM8_TRG_COM_handler(void) UNHANDLED_ALIAS;
 void TIM8_CC_handler(void) UNHANDLED_ALIAS;
 void ADC3_handler(void) UNHANDLED_ALIAS;
 void FSMC_handler(void) UNHANDLED_ALIAS;
 void SDIO_handler(void) UNHANDLED_ALIAS;
 void TIM5_handler(void) UNHANDLED_ALIAS;
 void SPI3_handler(void) UNHANDLED_ALIAS;
 void UART4_handler(void) UNHANDLED_ALIAS;
 void UART5_handler(void) UNHANDLED_ALIAS;
 void TIM6_handler(void) UNHANDLED_ALIAS;
 void TIM7_handler(void) UNHANDLED_ALIAS;
 void DMA2_Channel1_handler(void) UNHANDLED_ALIAS;
 void DMA2_Channel2_handler(void) UNHANDLED_ALIAS;
 void DMA2_Channel3_handler(void) UNHANDLED_ALIAS;
 void DMA2_Channel4_5_handler(void) UNHANDLED_ALIAS;

const ISR_func isr_vector[76] ISR_VECTOR_SECTION =
  {
    Main_Stack_End,
    sys_reset,
    NMI_handler,
    HardFault_handler,
    MemManage_handler,
    BusFault_handler,
    UsageFault_handler,
    Reserved_handler,
    Reserved_handler,
    Reserved_handler,
    Reserved_handler,
    SVCall_handler,
    DebugMonitor_handler,
    Reserved_handler,
    PendSV_handler,
    SysTick_handler,
    WWDG_handler,
    PVD_handler,
    TAMPER_handler,
    RTC_handler,
    FLASH_handler,
    RCC_handler,
    EXTI0_handler,
    EXTI1_handler,
    EXTI2_handler,
    EXTI3_handler,
    EXTI4_handler,
    DMA1_Channel1_handler,
    DMA1_Channel2_handler,
    DMA1_Channel3_handler,
    DMA1_Channel4_handler,
    DMA1_Channel5_handler,
    DMA1_Channel6_handler,
    DMA1_Channel7_handler,
    ADC1_2_handler,
    USB_HP_CAN_TX_handler,
    USB_LP_CAN_RX0_handler,
    CAN_RX1_handler,
    CAN_SCE_handler,
    EXTI9_5_handler,
    TIM1_BRK_handler,
    TIM1_UP_handler,
    TIM1_TRG_COM_handler,
    TIM1_CC_handler,
    TIM2_handler,
    TIM3_handler,
    TIM4_handler,
    I2C1_EV_handler,
    I2C1_ER_handler,
    I2C2_EV_handler,
    I2C2_ER_handler,
    SPI1_handler,
    SPI2_handler,
    USART1_handler,
    USART2_handler,
    USART3_handler,
    EXTI15_10_handler,
    RTCAlarm_handler,
    USBWakeup_handler, 
    TIM8_BRK_handler,
    TIM8_UP_handler,
    TIM8_TRG_COM_handler,
    TIM8_CC_handler,
    ADC3_handler,
    FSMC_handler,
    SDIO_handler,
    TIM5_handler,
    SPI3_handler,
    UART4_handler,
    UART5_handler,
    TIM6_handler,
    TIM7_handler,
    DMA2_Channel1_handler,
    DMA2_Channel2_handler,
    DMA2_Channel3_handler,
    DMA2_Channel4_5_handler
  };




extern uint8_t _data[];
extern uint8_t _etext[];
extern uint8_t _edata[];

static void
copy_initialized(void)
{
  uint8_t *ram = _data;
  uint8_t *rom = _etext;
  while(ram < _edata) {
    *ram++ = *rom++;
  }
}

extern uint8_t __bss_start[];
extern uint8_t __bss_end[];

static void
clear_bss(void)
{
  uint8_t *m = __bss_start;
  while(m < __bss_end) {
    *m++ = 0;
  }
}

static void
start_hse_clock(void)
{
  /* Start external oscillator */
  RCC->CR |= RCC_CR_HSEON;
  /* Wait for oscillator to stabilize */
  while(!(RCC->CR & RCC_CR_HSERDY));
}

static void
use_pll()
{
   RCC->CFGR = (RCC_CFGR_MCO_NOCLOCK
		| RCC_CFGR_PLLMULL6	/* PLL at 48MHz */
		| RCC_CFGR_PLLSRC	/* PLL runs on HSE */
		| RCC_CFGR_PPRE2_DIV1	/* APB2 at 48MHz */
		| RCC_CFGR_PPRE1_DIV2	/* APB1 at 24MHz */
		| RCC_CFGR_HPRE_DIV1	/* AHB at 48 MHz */
		| RCC_CFGR_USBPRE	/* USB clock at same speed as PLL */
		);
   RCC->CR |= RCC_CR_PLLON;
   /* Wait for PLL */
   while(!(RCC->CR & RCC_CR_PLLRDY));
   /* Switch to PLL as system clock */
   MODIFY_REG(RCC->CFGR, RCC_CFGR_SW,RCC_CFGR_SW_PLL);
}

static void
enable_fault_exceptions(void)
{
  SCB->SHCSR |= (SCB_SHCSR_MEMFAULTENA | SCB_SHCSR_BUSFAULTENA
		 | SCB_SHCSR_USGFAULTENA);
}

static void
sys_reset(void)
{
  copy_initialized();
  clear_bss();
  enable_fault_exceptions();
  start_hse_clock();
  use_pll();
  main();
  while(1);
  
}

void
NMI_handler(void)
{
  while(1);
}


static void
unhandled_int(void)
{
  while(1);
}

static void
dHardFault_handler(void)
{
  while(1);
}

static void
dUsageFault_handler(void)
{
  while(1);
}

static void
dMemManage_handler(void)
{
  while(1);
}

static void
dBusFault_handler(void)
{
  while(1);
}

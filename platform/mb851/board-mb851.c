#include PLATFORM_HEADER
#include BOARD_HEADER

void halBoardInit(void)
{
  
  return;
}


void halBoardPowerDown(void)
{
}
void halBoardPowerUp(void)
{
  /* Set everything to input value */
  GPIO_PACFGL = (GPIOCFG_IN            <<PA0_CFG_BIT)|
                (GPIOCFG_IN            <<PA1_CFG_BIT)|
                (GPIOCFG_IN            <<PA2_CFG_BIT)|
                (GPIOCFG_IN            <<PA3_CFG_BIT);
  GPIO_PACFGH = (GPIOCFG_IN            <<PA4_CFG_BIT)|  /* PTI EN */
                (GPIOCFG_IN            <<PA5_CFG_BIT)|  /* PTI_DATA */
                (GPIOCFG_IN            <<PA6_CFG_BIT)|
                (GPIOCFG_IN            <<PA7_CFG_BIT);
  GPIO_PBCFGL = (GPIOCFG_IN            <<PB0_CFG_BIT)|
                (GPIOCFG_OUT_ALT       <<PB1_CFG_BIT)|  /* Uart TX */
                (GPIOCFG_IN            <<PB2_CFG_BIT)|  /* Uart RX */
                (GPIOCFG_IN            <<PB3_CFG_BIT);
  GPIO_PBCFGH = (GPIOCFG_IN            <<PB4_CFG_BIT)|
                (GPIOCFG_IN            <<PB5_CFG_BIT)|
                (GPIOCFG_IN            <<PB6_CFG_BIT)|
                (GPIOCFG_IN            <<PB7_CFG_BIT);
  GPIO_PCCFGL = (GPIOCFG_IN            <<PC0_CFG_BIT)|
                (GPIOCFG_IN            <<PC1_CFG_BIT)|
                (GPIOCFG_IN            <<PC2_CFG_BIT)|
                (GPIOCFG_IN            <<PC3_CFG_BIT);
  GPIO_PCCFGH = (GPIOCFG_IN            <<PC4_CFG_BIT)|
                (GPIOCFG_IN            <<PC5_CFG_BIT)|
#ifdef EMBERZNET_HAL
                (CFG_C6                  <<PC6_CFG_BIT)|  /* OSC32K */
                (CFG_C7                  <<PC7_CFG_BIT);  /* OSC32K */
#else
                (GPIOCFG_IN              <<PC6_CFG_BIT)|  /* OSC32K */
                (GPIOCFG_IN              <<PC7_CFG_BIT);  /* OSC32K */
#endif

}

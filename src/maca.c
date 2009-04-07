#include "maca.h"


void init_phy(void)
{
  volatile uint32_t cnt;
  
  maca_reset = maca_reset_rst;
 
  for(cnt=0; cnt < 100; cnt++); 
  
  maca_reset = maca_reset_cln_on;              
  maca_control = control_seq_nop;              
#define DELAY 400000
  for(cnt=0; cnt < DELAY; cnt++); 

  maca_tmren = maca_start_clk | maca_cpl_clk;   
  maca_divider = gMACA_Clock_DIV_c; 
  maca_warmup = 0x00180012;    
  maca_eofdelay = 0x00000004;  
  maca_ccadelay = 0x001a0022;  
  maca_txccadelay = 0x00000025;
  maca_framesync = 0x000000A7; 
  maca_clk = 0x00000008;       
//  maca_maskirq = 0; //(maca_irq_cm   | maca_irq_acpl | maca_irq_rst  | maca_irq_di | maca_irq_crc | maca_irq_flt );
  maca_maskirq = maca_irq_rst;
  maca_slotoffset = 0x00350000; 
}

void reset_maca(void)
{
	uint32_t tmp;
	MACA_WRITE(maca_control, control_seq_nop);
  do
  {
	  tmp = MACA_READ(maca_status);
  }
  while ((tmp & maca_status_cc_mask) == cc_not_completed);

  /* Clear all interrupts. */
  MACA_WRITE(maca_clrirq,   0xFFFF);
}


/* 
 * Do the ABORT-Wait-NOP-Wait sequence in order to prevent MACA malfunctioning.
 * This seqeunce is synchronous and no interrupts should be triggered when it is done.
 */
void ResumeMACASync(void)
{ 
  uint32_t clk, TsmRxSteps, LastWarmupStep, LastWarmupData, LastWarmdownStep, LastWarmdownData;
//  bool_t tmpIsrStatus;
  volatile uint32_t i;

//  ITC_DisableInterrupt(gMacaInt_c);  
//  AppInterrupts_ProtectFromMACAIrq(tmpIsrStatus); <- Original from MAC code, but not sure how is it implemented

  /* Manual TSM modem shutdown */

  /* read TSM_RX_STEPS */
  TsmRxSteps = (*((volatile uint32_t *)(0x80009204)));
 
  /* isolate the RX_WU_STEPS */
  /* shift left to align with 32-bit addressing */
  LastWarmupStep = (TsmRxSteps & 0x1f) << 2;
  /* Read "current" TSM step and save this value for later */
  LastWarmupData = (*((volatile uint32_t *)(0x80009300 + LastWarmupStep)));

  /* isolate the RX_WD_STEPS */
  /* right-shift bits down to bit 0 position */
  /* left-shift to align with 32-bit addressing */
  LastWarmdownStep = ((TsmRxSteps & 0x1f00) >> 8) << 2;
  /* write "last warmdown data" to current TSM step to shutdown rx */
  LastWarmdownData = (*((volatile uint32_t *)(0x80009300 + LastWarmdownStep)));
  (*((volatile uint32_t *)(0x80009300 + LastWarmupStep))) = LastWarmdownData;

  /* Abort */
  MACA_WRITE(maca_control, 1);
  
  /* Wait ~8us */
  for (clk = maca_clk, i = 0; maca_clk - clk < 3 && i < 300; i++)
    ;
 
  /* NOP */
  MACA_WRITE(maca_control, 0);  
  
  /* Wait ~8us */  
  for (clk = maca_clk, i = 0; maca_clk - clk < 3 && i < 300; i++)
    ;
   

  /* restore original "last warmup step" data to TSM (VERY IMPORTANT!!!) */
  (*((volatile uint32_t *)(0x80009300 + LastWarmupStep))) = LastWarmupData;

  
  
  /* Clear all MACA interrupts - we should have gotten the ABORT IRQ */
  MACA_WRITE(maca_clrirq, 0xFFFF);

//  AppInterrupts_UnprotectFromMACAIrq(tmpIsrStatus);  <- Original from MAC code, but not sure how is it implemented
//  ITC_EnableInterrupt(gMacaInt_c);
}

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
  maca_maskirq = 0; //(maca_irq_cm   | maca_irq_acpl | maca_irq_rst  | maca_irq_di | maca_irq_crc | maca_irq_flt );
  maca_slotoffset = 0x00350000; 
}

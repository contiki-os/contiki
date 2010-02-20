#include <mc1322x.h>
#include <nvm.h>
#include "maca.h"

#define reg(x) (*(volatile uint32_t *)(x))

static uint8_t ram_values[4];

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
  maca_maskirq = (maca_irq_rst | maca_irq_acpl | maca_irq_cm | maca_irq_flt | maca_irq_crc);
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
	004030c4 <SMAC_InitFlybackSettings>:
	4030c4:       4806            ldr     r0, [pc, #24]   (4030e0 <SMAC_InitFlybackSettings+0x1c>) // r0 gets base 0x80009a00
		4030c6:       6881            ldr     r1, [r0, #8]                                             // r1 gets *(0x80009a08)
		4030c8:       4806            ldr     r0, [pc, #24]   (4030e4 <SMAC_InitFlybackSettings+0x20>) // r0 gets 0x0000f7df
		4030ca:       4308            orrs    r0, r1                                                   // or them, r0 has it
		4030cc:       4904            ldr     r1, [pc, #16]   (4030e0 <SMAC_InitFlybackSettings+0x1c>) // r1 gets base 0x80009a00
		4030ce:       6088            str     r0, [r1, #8]     // put r0 into 0x80009a08
		4030d0:       0008            lsls    r0, r1, #0       // r0 gets r1, r0 is the base now
		4030d2:       4905            ldr     r1, [pc, #20]   (4030e8 <SMAC_InitFlybackSettings+0x24>) // r1 gets 0x00ffffff
		4030d4:       60c1            str     r1, [r0, #12]   // put 0x00ffffff into base+12
		4030d6:       0b09            lsrs    r1, r1, #12     // r1 = 0x00ffffff >> 12
		4030d8:       6101            str     r1, [r0, #16]   // put r1 base+16
		4030da:       2110            movs    r1, #16         // r1 gets 16
		4030dc:       6001            str     r1, [r0, #0]    // put r1 in the base
		4030de:       4770            bx      lr              // return
		4030e0:       80009a00        .word   0x80009a00
		4030e4:       0000f7df        .word   0x0000f7df
		4030e8:       00ffffff        .word   0x00ffffff
*/

/* tested and is good */
#define RF_BASE 0x80009a00
void flyback_init(void) {
	uint32_t val8, or;
	
	val8 = *(volatile uint32_t *)(RF_BASE+8);
	or = val8 | 0x0000f7df;
	*(volatile uint32_t *)(RF_BASE+8) = or;
	*(volatile uint32_t *)(RF_BASE+12) = 0x00ffffff;
	*(volatile uint32_t *)(RF_BASE+16) = (((uint32_t)0x00ffffff)>>12);
	*(volatile uint32_t *)(RF_BASE) = 16;
	/* good luck and godspeed */
}

#define MAX_SEQ1 2
const uint32_t addr_seq1[MAX_SEQ1] = {
	0x80003048,      
	0x8000304c,
};

const uint32_t data_seq1[MAX_SEQ1] = {
	0x00000f78,     
	0x00607707,
};


#define MAX_SEQ2 2
const uint32_t addr_seq2[MAX_SEQ2] = {
	0x8000a050,      
	0x8000a054,      
};

const uint32_t data_seq2[MAX_SEQ2] = {
	0x0000047b,
	0x0000007b, 
};

#define MAX_CAL3_SEQ1 3
const uint32_t addr_cal3_seq1[MAX_CAL3_SEQ1] = { 0x80009400,0x80009a04,0x80009a00, };
const uint32_t data_cal3_seq1[MAX_CAL3_SEQ1] = {0x00020017,0x8185a0a4,0x8c900025, };

#define MAX_CAL3_SEQ2 2
const uint32_t addr_cal3_seq2[MAX_CAL3_SEQ2] = { 0x80009a00,0x80009a00,};
const uint32_t data_cal3_seq2[MAX_CAL3_SEQ2] = { 0x8c900021,0x8c900027,};

#define MAX_CAL3_SEQ3 1
const uint32_t addr_cal3_seq3[MAX_CAL3_SEQ3] = { 0x80009a00 };
const uint32_t data_cal3_seq3[MAX_CAL3_SEQ3] = { 0x8c900000 };

#define MAX_CAL5 4
const uint32_t addr_cal5[MAX_CAL5] = { 
	0x80009400,  
	0x8000a050,       
	0x8000a054,  
	0x80003048,
};
const uint32_t data_cal5[MAX_CAL5] = {
	0x00000017,
	0x00000000,            
	0x00000000,
	0x00000f00,
};

#define MAX_DATA 43
const uint32_t addr_reg_rep[MAX_DATA] = { 0x80004118,0x80009204,0x80009208,0x8000920c,0x80009210,0x80009300,0x80009304,0x80009308,0x8000930c,0x80009310,0x80009314,0x80009318,0x80009380,0x80009384,0x80009388,0x8000938c,0x80009390,0x80009394,0x8000a008,0x8000a018,0x8000a01c,0x80009424,0x80009434,0x80009438,0x8000943c,0x80009440,0x80009444,0x80009448,0x8000944c,0x80009450,0x80009460,0x80009464,0x8000947c,0x800094e0,0x800094e4,0x800094e8,0x800094ec,0x800094f0,0x800094f4,0x800094f8,0x80009470,0x8000981c,0x80009828 };

const uint32_t data_reg_rep[MAX_DATA] = { 0x00180012,0x00000605,0x00000504,0x00001111,0x0fc40000,0x20046000,0x4005580c,0x40075801,0x4005d801,0x5a45d800,0x4a45d800,0x40044000,0x00106000,0x00083806,0x00093807,0x0009b804,0x000db800,0x00093802,0x00000015,0x00000002,0x0000000f,0x0000aaa0,0x01002020,0x016800fe,0x8e578248,0x000000dd,0x00000946,0x0000035a,0x00100010,0x00000515,0x00397feb,0x00180358,0x00000455,0x00000001,0x00020003,0x00040014,0x00240034,0x00440144,0x02440344,0x04440544,0x0ee7fc00,0x00000082,0x0000002a };


/* has been tested and it good */
void vreg_init(void) {
	volatile uint32_t i;
	*(volatile uint32_t *)(0x80003000) = 0x00000018; /* set default state */
	*(volatile uint32_t *)(0x80003048) = 0x00000f04; /* bypass the buck */
	for(i=0; i<0x161a8; i++) { continue; } /* wait for the bypass to take */
//	while((((*(volatile uint32_t *)(0x80003018))>>17) & 1) !=1) { continue; } /* wait for the bypass to take */
	*(volatile uint32_t *)(0x80003048) = 0x00000ff8; /* start the regulators */
}

void radio_off(void) {
	/* turn off the radio regulators */
	reg(0x80003048) =  0x00000f00;
	/* hold the maca in reset */
	maca_reset = maca_reset_rst;  
}

void radio_on(void) {
	/* turn the radio regulators back on */
	reg(0x80003048) =  0x00000f78; 
	/* reinitialize the phy */
	init_phy();
}

/* initialized with 0x4c */
uint8_t ctov[16] = {
        0x0b,
        0x0b,
        0x0b,
        0x0a,
        0x0d,
        0x0d,
        0x0c,
        0x0c,
        0x0f,
        0x0e,
        0x0e,
        0x0e,
        0x11,
        0x10,
        0x10,
        0x0f,
};

/* get_ctov thanks to Umberto */

#define _INIT_CTOV_WORD_1       0x00dfbe77
#define _INIT_CTOV_WORD_2       0x023126e9
uint8_t get_ctov( uint32_t r0, uint32_t r1 )
{

        r0 = r0 * _INIT_CTOV_WORD_1;
        r0 += ( r1 << 22 );
        r0 += _INIT_CTOV_WORD_2;

        r0 = (uint32_t)(((int32_t)r0) >> 25);

        return (uint8_t)r0;
}


/* radio_init has been tested to be good */
void radio_init(void) {
	volatile uint32_t i;
	/* sequence 1 */
	for(i=0; i<MAX_SEQ1; i++) {
		*(volatile uint32_t *)(addr_seq1[i]) = data_seq1[i];
	}
	/* seq 1 delay */
	for(i=0; i<0x161a8; i++) { continue; }
	/* sequence 2 */
	for(i=0; i<MAX_SEQ2; i++) {
		*(volatile uint32_t *)(addr_seq2[i]) = data_seq2[i];
	}
	/* modem val */
	*(volatile uint32_t *)0x80009000 = 0x80050100;
	/* cal 3 seq 1*/
	for(i=0; i<MAX_CAL3_SEQ1; i++) {
		*(volatile uint32_t *)(addr_cal3_seq1[i]) = data_cal3_seq1[i];
	}
	/* cal 3 delay */
	for(i=0; i<0x11194; i++) { continue; }
	/* cal 3 seq 2*/
	for(i=0; i<MAX_CAL3_SEQ2; i++) {
		*(volatile uint32_t *)(addr_cal3_seq2[i]) = data_cal3_seq2[i];
	}
	/* cal 3 delay */
	for(i=0; i<0x11194; i++) { continue; }
	/* cal 3 seq 3*/
	for(i=0; i<MAX_CAL3_SEQ3; i++) {
		*(volatile uint32_t *)(addr_cal3_seq3[i]) = data_cal3_seq3[i];
	}
	/* cal 5 */
	for(i=0; i<MAX_CAL5; i++) {
		*(volatile uint32_t *)(addr_cal5[i]) = data_cal5[i];
	}
	/*reg replacment */
	for(i=0; i<MAX_DATA; i++) {
		*(volatile uint32_t *)(addr_reg_rep[i]) = data_reg_rep[i];
	}
	
	puts("initfromflash\n\r");

	*(volatile uint32_t *)(0x80003048) = 0x00000f04; /* bypass the buck */
	for(i=0; i<0x161a8; i++) { continue; } /* wait for the bypass to take */
//	while((((*(volatile uint32_t *)(0x80003018))>>17) & 1) !=1) { continue; } /* wait for the bypass to take */
	*(volatile uint32_t *)(0x80003048) = 0x00000fa4; /* start the regulators */
	for(i=0; i<0x161a8; i++) { continue; } /* wait for the bypass to take */

	init_from_flash(0x1F000);

	puts("ram_values:\n\r");
	for(i=0; i<4; i++) {
		puts("  0x");
		put_hex(ram_values[i]);
		puts("\n\r");
	}

        puts("radio_init: ctov parameter 0x");
	put_hex(ram_values[3]);
	puts("\n\r");
        for(i=0; i<16; i++) {
                ctov[i] = get_ctov(i,ram_values[3]);
                puts("radio_init: ctov[");
		put_hex(i);
		puts("] = 0x");
		put_hex(ctov[i]);
		puts("\n\r");
        }


}

const uint32_t PSMVAL[19] = {
	0x0000080f,
	0x0000080f,
	0x0000080f,
	0x0000080f,
	0x0000081f,
	0x0000081f,
	0x0000081f,
	0x0000080f,
	0x0000080f,
	0x0000080f,
	0x0000001f,
	0x0000000f,
	0x0000000f,
	0x00000816,
	0x0000001b,
	0x0000000b,
	0x00000802,
	0x00000817,
	0x00000003,
};

const uint32_t PAVAL[19] = {
	0x000022c0,
	0x000022c0,
	0x000022c0,
	0x00002280,
	0x00002303,
	0x000023c0,
	0x00002880,
	0x000029f0,
	0x000029f0,
	0x000029f0,
	0x000029c0,
	0x00002bf0,
	0x000029f0,
	0x000028a0,
	0x00002800,
	0x00002ac0,
	0x00002880,
	0x00002a00,
	0x00002b00,
};

const uint32_t AIMVAL[19] = {
	0x000123a0,
	0x000163a0,
	0x0001a3a0,
	0x0001e3a0,
	0x000223a0,
	0x000263a0,
	0x0002a3a0,
	0x0002e3a0,
	0x000323a0,
	0x000363a0,
	0x0003a3a0,
	0x0003a3a0,
	0x0003e3a0,
	0x000423a0,
	0x000523a0,
	0x000423a0,
	0x0004e3a0,
	0x0004e3a0,
	0x0004e3a0,
};

/* tested and seems to be good */
#define ADDR_POW1 0x8000a014
#define ADDR_POW2 ADDR_POW1 + 12
#define ADDR_POW3 ADDR_POW1 + 64
void set_power(uint8_t power) {
	reg(ADDR_POW1) = PSMVAL[power];
	reg(ADDR_POW2) = (ADDR_POW1>>18) | PAVAL[power];
	reg(ADDR_POW3) = AIMVAL[power];
}

const uint8_t VCODivI[16] = {
	0x2f,
	0x2f,
	0x2f,
	0x2f,
	0x2f,
	0x2f,
	0x2f,
	0x2f,
	0x30,
	0x30,
	0x30,
	0x2f,
	0x30,
	0x30,
	0x30,
	0x30,
};

const uint32_t VCODivF[16] = {
	0x00355555,
	0x006aaaaa,
	0x00a00000,
	0x00d55555,
	0x010aaaaa,
	0x01400000,
	0x01755555,
	0x01aaaaaa,
	0x01e00000,
	0x00155555,
	0x004aaaaa,
	0x00800000,
	0x00b55555,
	0x00eaaaaa,
	0x01200000,
	0x01555555,		
};

/* tested good */
#define ADDR_CHAN1 0x80009800
#define ADDR_CHAN2 (ADDR_CHAN1+12)
#define ADDR_CHAN3 (ADDR_CHAN1+16)
#define ADDR_CHAN4 (ADDR_CHAN1+48)
void set_channel(uint8_t chan) {
	volatile uint32_t tmp;

	tmp = reg(ADDR_CHAN1);
	tmp = tmp & 0xbfffffff;
	reg(ADDR_CHAN1) = tmp;

	reg(ADDR_CHAN2) = VCODivI[chan];
	reg(ADDR_CHAN3) = VCODivF[chan];

	tmp = reg(ADDR_CHAN4);
	tmp = tmp | 2;
	reg(ADDR_CHAN4) = tmp;

	tmp = reg(ADDR_CHAN4);
	tmp = tmp | 4;
	reg(ADDR_CHAN4) = tmp;

	tmp = tmp & 0xffffe0ff;
	tmp = tmp | (((ctov[chan])<<8)&0x1F00);
	reg(ADDR_CHAN4) = tmp;
	/* duh! */
}

#define ROM_END 0x0013ffff
#define ENTRY_EOF 0x00000e0f
/* processes up to 4 words of initialization entries */
/* returns the number of words processed */
uint32_t exec_init_entry(uint32_t *entries, uint8_t *valbuf) 
{
	volatile uint32_t i;
	if(entries[0] <= ROM_END) {
		if (entries[0] == 0) {
			/* do delay command*/
			puts("init_entry: delay ");
			put_hex32(entries[1]);
			puts("\n\r");
			for(i=0; i<entries[1]; i++) { continue; }
			return 2;
		} else if (entries[0] == 1) {
			/* do bit set/clear command*/
			puts("init_entry: bit set clear ");
			put_hex32(entries[1]);
			putc(' ');
			put_hex32(entries[2]);
			putc(' ');
			put_hex32(entries[3]);
			puts("\n\r");
			reg(entries[2]) = (reg(entries[2]) & ~entries[1]) | (entries[3] & entries[1]);
			return 4;
		} else if ((entries[0] >= 16) &&
			   (entries[0] < 0xfff1)) {
			/* store bytes in valbuf */
			puts("init_entry: store in valbuf ");
			put_hex(entries[1]);
			puts(" position ");
			put_hex((entries[0]>>4)-1);
			puts("\n\r");
			valbuf[(entries[0]>>4)-1] = entries[1];
			return 2;
		} else if (entries[0] == ENTRY_EOF) {
			puts("init_entry: eof ");
			return 0;
		} else {
			/* invalid command code */
			puts("init_entry: invaild code ");
			put_hex32(entries[0]);
			puts("\n\r");
			return 0;
		}
	} else { /* address isn't in ROM space */   
		 /* do store value in address command  */
		puts("init_entry: address value pair - *0x");
		put_hex32(entries[0]);
		puts(" = ");
		put_hex32(entries[1]);
		puts("\n\r");
		reg(entries[0]) = entries[1];
		return 2;
	}
}


#define FLASH_INIT_MAGIC 0x00000abc
uint32_t init_from_flash(uint32_t addr) {
	nvmType_t type=0;
	nvmErr_t err;	
	volatile uint32_t buf[8];
	volatile uint16_t len;
	volatile uint32_t i=0,j;
	err = nvm_detect(gNvmInternalInterface_c, &type);
	puts("nvm_detect returned type ");
	put_hex32(type);
	puts(" err ");
	put_hex(err);
	puts("\n\r");
		
	nvm_setsvar(0);
	err = nvm_read(gNvmInternalInterface_c, type, (uint8_t *)buf, addr, 8);
	i+=8;
	puts("nvm_read returned: 0x");
	put_hex(err);
	puts("\n\r");
	
	for(j=0; j<4; j++) {
		put_hex32(buf[j]);
		puts("\n\r");
	}

	if(buf[0] == FLASH_INIT_MAGIC) {
		len = buf[1] & 0x0000ffff;
		while(i<len-4) {
			volatile uint32_t ret;
			err = nvm_read(gNvmInternalInterface_c, type, (uint8_t *)buf, addr+i, 32);
			i += 4*exec_init_entry(buf, ram_values);
		}
		return i;
	} else {
		return 0;
	}
 	
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

#include <mc1322x.h>
#include <types.h>

void default_vreg_init(void) {
	volatile uint32_t i;
	*CRM_SYS_CNTL = 0x00000018; /* set default state */
	*CRM_VREG_CNTL = 0x00000f04; /* bypass the buck */
	for(i=0; i<0x161a8; i++) { continue; } /* wait for the bypass to take */
//	while((((*(volatile uint32_t *)(0x80003018))>>17) & 1) !=1) { continue; } /* wait for the bypass to take */
	*CRM_VREG_CNTL = 0x00000ff8; /* start the regulators */
}

void uart1_init(uint16_t inc, uint16_t mod, uint8_t samp) {
		
        /* UART must be disabled to set the baudrate */
	*UART1_UCON = 0;
	*UART1_UBRCNT = ( inc << 16 ) | mod; 

	/* see Section 11.5.1.2 Alternate Modes */
	/* you must enable the peripheral first BEFORE setting the function in GPIO_FUNC_SEL */
	/* From the datasheet: "The peripheral function will control operation of the pad IF */
	/* THE PERIPHERAL IS ENABLED. */
	*UART1_UCON = (1 << 0) | (1 << 1); /* enable receive, transmit */
	if(samp == UCON_SAMP_16X) 
		set_bit(*UART1_UCON,UCON_SAMP);
	*GPIO_FUNC_SEL0 = ( (0x01 << (14*2)) | (0x01 << (15*2)) ); /* set GPIO15-14 to UART (UART1 TX and RX)*/

	/* interrupt when 28 bytes are free */
	*UART1_UTXCON = 28;

	u1_head = 0; u1_tail = 0;
	enable_irq(UART1);
}

#include <crm.h>

void default_vreg_init(void) {
	volatile uint32_t i;
	*CRM_SYS_CNTL = 0x00000018; /* set default state */
	*CRM_VREG_CNTL = 0x00000f04; /* bypass the buck */
	for(i=0; i<0x161a8; i++) { continue; } /* wait for the bypass to take */
//	while((((*(volatile uint32_t *)(0x80003018))>>17) & 1) !=1) { continue; } /* wait for the bypass to take */
	*CRM_VREG_CNTL = 0x00000ff8; /* start the regulators */
}

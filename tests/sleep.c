#define GPIO_PAD_DIR0   0x80000000
#define GPIO_DATA0      0x80000008
#define CRM_WU_CNTL     0x80003004
#define CRM_SLEEP_CNTL  0x80003008
#define CRM_STATUS      0x80003018
#define GPIO_PAD_PU_EN0 0x80000010
#define GPIO_PAD_PU_EN1 0x80000014
#define ADC_CONTROL     0x80000018
#define CRM_XTAL_CNTL   0x80000040

#define DELAY 400000

#include "embedded_types.h"
#include "isr.h"
#include "utils.h"

__attribute__ ((section ("startup"))) void main(void) {
	reg32(GPIO_PAD_DIR0) = 0x00000100;

	reg32(GPIO_DATA0) = 0x00000100;

	/* disable all pullups */
	/* seems to make a slight difference (2.0uA vs 1.95uA)*/
	reg32(GPIO_PAD_PU_EN0) = 0;
	reg32(GPIO_PAD_PU_EN1) = 0;
	reg16(ADC_CONTROL) = 0; /* internal Vref2 */

//	reg16(CRM_XTAL_CNTL) = 0x052; /* default is 0xf52 */ /* doesn't anything w.r.t. power */

	/* go to sleep */
//	reg32(CRM_WU_CNTL) = 0x1; /* enable wakeup from wakeup timer */
	reg32(CRM_WU_CNTL) = 0; /* don't wake up */

//	reg32(CRM_SLEEP_CNTL) = 1; /* hibernate, RAM page 0 only, don't retain state, don't power GPIO */ /* approx. 2.0uA */
//	reg32(CRM_SLEEP_CNTL) = 0x41; /* hibernate, RAM page 0 only, retain state, don't power GPIO */ /* approx. 10.0uA */
//	reg32(CRM_SLEEP_CNTL) = 0x51; /* hibernate, RAM page 0&1 only, retain state, don't power GPIO */ /* approx. 11.7uA */
//	reg32(CRM_SLEEP_CNTL) = 0x61; /* hibernate, RAM page 0&1 only, retain state, don't power GPIO */ /* approx. 13.9uA */
//	reg32(CRM_SLEEP_CNTL) = 0x71; /* hibernate, RAM page 0&1 only, retain state, don't power GPIO */ /* approx. 16.1uA - possibly with periodic refresh*/
//	reg32(CRM_SLEEP_CNTL) = 0xf1; /* hibernate, RAM page 0&1 only, retain state,       power GPIO */ /* approx. 16.1uA - possibly with periodic refresh*/

//	reg32(CRM_SLEEP_CNTL) = 2; /* doze     , RAM page 0 only, don't retain state, don't power GPIO */ /* approx. 69.2 uA */
//	reg32(CRM_SLEEP_CNTL) = 0x42; /* doze     , RAM page 0 only, retain state, don't power GPIO */ /* approx. 77.3uA */
//	reg32(CRM_SLEEP_CNTL) = 0x52; /* doze     , RAM page 0&1 only, retain state, don't power GPIO */ /* approx. 78.9uA */
//	reg32(CRM_SLEEP_CNTL) = 0x62; /* doze     , RAM page 0&1 only, retain state, don't power GPIO */ /* approx. 81.2uA */
//	reg32(CRM_SLEEP_CNTL) = 0x72; /* doze     , RAM page 0&1 only, retain state, don't power GPIO */ /* approx. 83.4uA - possibly with periodic refresh*/
	reg32(CRM_SLEEP_CNTL) = 0xf2; /* doze     , RAM page 0&1 only, retain state,       power GPIO */ /* approx. 82.8uA - possibly with periodic refresh*/
	reg32(CRM_STATUS) = 1; /* write 1 to sleep_sync --- this totally powers down */
	

	volatile uint32_t i;
	while(1) {

		reg32(GPIO_DATA0) = 0x00000100;
		
		for(i=0; i<DELAY; i++) { continue; }

		reg32(GPIO_DATA0) = 0x00000000;

		for(i=0; i<DELAY; i++) { continue; }

	};
}

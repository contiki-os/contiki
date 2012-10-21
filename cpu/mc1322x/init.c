#include <stdio.h>

/* debug */
#define DEBUG DEBUG_FULL
#include "net/uip-debug.h"

/* contiki */
#include "sys/process.h"

/* mc1322x */
#include "mc1322x.h"
#include "contiki-maca.h"
#include "config.h"

/* Threshold for buck converter; buck will be disabled if vbatt is below this */
#define MC1322X_BUCK_THRES 2425
/* Hysterisis window around buck threshold */
#define MC1322X_BUCK_WINDOW 150
#define MC1322X_BUCK_THRES_H (MC1322X_BUCK_THRES + MC1322X_BUCK_WINDOW/2)
#define MC1322X_BUCK_THRES_L (MC1322X_BUCK_THRES - MC1322X_BUCK_WINDOW/2)
/* Time between vbatt checks for the buck */
#define MC1322X_BUCK_MONITOR_PERIOD 600 * CLOCK_SECOND

/* periodically poll adc_vbatt and manages the buck appropriately */
static struct etimer et_buck;
PROCESS(buck_monitor, "buck monitor");
PROCESS_THREAD(buck_monitor, ev, data)
{

	PROCESS_BEGIN();
	PRINTF("starting vbatt monitor\n");

	etimer_set(&et_buck, MC1322X_BUCK_MONITOR_PERIOD);

	while (1) {
		PROCESS_WAIT_EVENT();
		if(etimer_expired(&et_buck))
		{
			adc_service();
			PRINTF("buck monitor: vbatt: %d mV\n\r", adc_vbatt);
			if( CRM->VREG_CNTLbits.BUCK_EN == 1 && adc_vbatt < MC1322X_BUCK_THRES_L ) {
				PRINTF("vbatt low, disabling buck\n\r", adc_vbatt);
				CRM->SYS_CNTLbits.PWR_SOURCE = 0;
				CRM->VREG_CNTLbits.BUCK_SYNC_REC_EN = 0;
				CRM->VREG_CNTLbits.BUCK_BYPASS_EN = 1;
				CRM->VREG_CNTLbits.BUCK_EN = 0;
			} else if ( CRM->VREG_CNTLbits.BUCK_EN == 0 && adc_vbatt > MC1322X_BUCK_THRES_H ) {
				PRINTF("vbatt high, enabling buck\n\r", adc_vbatt);
				CRM->SYS_CNTLbits.PWR_SOURCE = 1;
				CRM->VREG_CNTLbits.BUCK_SYNC_REC_EN = 1;
				CRM->VREG_CNTLbits.BUCK_BYPASS_EN = 0;
				CRM->VREG_CNTLbits.BUCK_EN = 1;
			}
			etimer_set(&et_buck, MC1322X_BUCK_MONITOR_PERIOD);
		}
 	}

 	PROCESS_END();

}

void buck_setup(void) {
	nvmType_t type;
	nvmErr_t err;
	volatile int i;

	default_vreg_init();

	while(CRM->STATUSbits.VREG_1P5V_RDY == 0) { continue; }
	while(CRM->STATUSbits.VREG_1P8V_RDY == 0) { continue; }

	/* takes time for the flash supply to fail (if there is no buck) */
	/* spin while this happens doing nvm_detects */
	/* XXX todo: don't probe buck if Vbatt < 2.5V */

	adc_service();
	PRINTF("vbatt: %04u mV\n\r", adc_vbatt);

	type = 1;
	for(i = 0; i < 128 && type != 0; i++) {
		err = nvm_detect(gNvmInternalInterface_c, &type);
	}
	if (type == gNvmType_NoNvm_c)
	{
		PRINTF("NVM failed without buck, trying with buck\n\r");

		if (adc_vbatt < MC1322X_BUCK_THRES_L)
		{
			PRINTF("Vbatt is low, bypassing buck\n\r");
			CRM->SYS_CNTLbits.PWR_SOURCE = 0;
			CRM->VREG_CNTLbits.BUCK_SYNC_REC_EN = 0;
			CRM->VREG_CNTLbits.BUCK_BYPASS_EN = 1;
			CRM->VREG_CNTLbits.BUCK_EN = 0;
		} else {
			CRM->SYS_CNTLbits.PWR_SOURCE = 1;
			CRM->VREG_CNTLbits.BUCK_SYNC_REC_EN = 1;
			CRM->VREG_CNTLbits.BUCK_BYPASS_EN = 0;
			CRM->VREG_CNTLbits.BUCK_EN = 1;
		}

		while(CRM->STATUSbits.VREG_BUCK_RDY == 0) { continue; }
		CRM->VREG_CNTLbits.VREG_1P5V_SEL = 3;
		CRM->VREG_CNTLbits.VREG_1P5V_EN = 3;
		CRM->VREG_CNTLbits.VREG_1P8V_EN = 1;
		while(CRM->STATUSbits.VREG_1P5V_RDY == 0) { continue; }
		while(CRM->STATUSbits.VREG_1P8V_RDY == 0) { continue; }

		type = 1;
		for(i = 0; i < 128 && type != 0; i++) {
			err = nvm_detect(gNvmInternalInterface_c, &type);
		}
		if (type != gNvmType_NoNvm_c) {
			PRINTF("buck ok\n\r");
			/* start a process to monitor vbatt and enable/disable the buck as necessary */
			process_start(&buck_monitor, NULL);
		} else {
			printf("fatal: couldn't detect NVM\n\r");
		}
	} else {
		PRINTF("NVM ok without buck\n\r");
	}
}

/* setup the RTC */
/* try to start the 32kHz xtal */
void rtc_setup(void) {
	volatile uint32_t rtc_count;
	volatile uint32_t i;

	ring_osc_off();
	xtal32_on();
	xtal32_exists();
	rtc_count = CRM->RTC_COUNT;
	PRINTF("trying to start 32kHz xtal\n\r");

	for(i = 0; i < 150000 && CRM->RTC_COUNT == rtc_count; i++) { continue; }
	if(CRM->RTC_COUNT == rtc_count) {
		PRINTF("32xtal failed, using ring osc\n\r");
		CRM->SYS_CNTLbits.XTAL32_EXISTS = 0;
		CRM->XTAL32_CNTLbits.XTAL32_EN = 0;
		ring_osc_on();

		/* Set default tune values from datasheet */
		CRM->RINGOSC_CNTLbits.ROSC_CTUNE = 0x6;
		CRM->RINGOSC_CNTLbits.ROSC_FTUNE = 0x17;

		/* Trigger calibration */
		rtc_calibrate();
		PRINTF("RTC calibrated to %d Hz\r\n", rtc_freq);
	} else {
		PRINTF("32kHz xtal started\n\r");
	}
}

/* call mc1322x_init once to initalize everything with the current config */
void mc1322x_init(void) {

	/* XXX TODO load config from flash */
	/* config should say what uart to use for debug console */
	/* config should also set the baud rate */
	/* for now, just clean up contiki-conf.h */
	/* maybe factor into conf.h -> contiki-conf.h and mc1322x-conf.h platform-conf.h */

	/* print out config in debug */
	/* initialize the uarts */
	uart_init(CONSOLE_UART, CONSOLE_BAUD);
	PRINTF("mc1322x init\n\r");

	adc_init();
	clock_init();
	ctimer_init();
	process_init();
	process_start(&etimer_process, NULL);
	process_start(&contiki_maca_process, NULL);
	buck_setup();
	rtc_setup();

	/* start with a default config */

	mc1322x_config_restore(&mc1322x_config);
	if ( mc1322x_config_valid(&mc1322x_config) != 1 ) {
		PRINTF("flash invalid\n\r");
		/* save the default config to flash */
		mc1322x_config_set_default(&mc1322x_config);
		mc1322x_config_save(&mc1322x_config);
	}

#if DEBUG_FULL
	mc1322x_config_print();
#endif

	/* setup the radio */
	maca_init();
	set_power(mc1322x_config.power);
	set_channel(mc1322x_config.channel);
	set_demodulator_type(mc1322x_config.flags.demod);
	set_prm_mode(mc1322x_config.flags.autoack);


}


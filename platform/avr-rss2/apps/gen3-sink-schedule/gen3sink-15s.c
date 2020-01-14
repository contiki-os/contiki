/**
* \file
*         WIMEA-ICT Gen3 AWS Gateway
* \details
*   ATMEGA256RFR2 RSS2 MOTE with RTC, SD card and Electron 3G uplink
* \author
*         Maximus Byamukama <maximus.byamukama@cedat.mak.ac.ug>
*/
#include <stdio.h>
#include <avr/io.h>
#include <avr/sleep.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "contiki.h"
#include "rss2.h"
#include "netstack.h"
#include "rf230bb.h"
#include "net/rime/rime.h"
#include "net/packetbuf.h"
#include "dev/leds.h"

#include "dev/adc.h"
#include "dev/i2c.h"
#include "dev/ds3231.h"
#include "dev/temp-sensor.h"
#include <dev/watchdog.h>

#include "dev/serial-line.h"
#include <avr/eeprom.h>
#include "dev/powertrace.h"

#define SYS_VLOW 3.30
#define NAME_LENGTH 12

uint16_t EEMEM eemem_name_flag;
uint8_t EEMEM eemem_node_name[NAME_LENGTH];

double v_in = 0, v_mcu = 0;
uint8_t rssi, lqi, seqno = 0, v_low = 1, secs = 0, len = 0;
long int rep_count = 0;
char report[200], sinkrep[150];
static struct timer t;
static int ps = 0; /* Power-save false */
static double up_time = 0;

datetime_t datetime;

struct broadcast_message {
	uint8_t head;
	uint8_t seqno;
	char buf[120];
};

/*--------------------------------------------------------------------------------------------------------- */
//                 The Process Definitions and Auto Start of the Processes
/*--------------------------------------------------------------------------------------------------------- */

PROCESS(init_process, "Init Process");
PROCESS(radio_process, "Radio Process");
PROCESS(sleep_process, "Sleep Process");
PROCESS(serial_command, "Serial Command");

AUTOSTART_PROCESSES(&init_process, &radio_process, &sleep_process, &serial_command);


/*--------------------------------------------------------------------------------------------------------- */
//                     The Broadcast_recv Function.
/*--------------------------------------------------------------------------------------------------------- */

static void
broadcast_recv(struct broadcast_conn *c, const linkaddr_t *from)
{
	struct broadcast_message *msg;
	len = 0;

	msg = packetbuf_dataptr();
	rssi = packetbuf_attr(PACKETBUF_ATTR_RSSI);
	lqi = packetbuf_attr(PACKETBUF_ATTR_LINK_QUALITY);

	ds3231_get_datetime(&datetime);

	len+=sprintf(report, "RTC_T=20%d-%02d-%02d,%02d:%02d:%02d %s [ADDR=%-d.%-d SEQ=%-d TTL=%-u RSSI=%-u LQI=%-u]\n",
	datetime.year, datetime.month, datetime.day, datetime.hours, datetime.mins, datetime.secs, msg->buf,
	from->u8[0], from->u8[1], msg->seqno, msg->head & 0xF, rssi, lqi);

	report[len++] = '\0';
	pwr_pin_on();
	timer_set(&t, CLOCK_SECOND/100); //5ms pulse to wake up electron from sleep
	while(!timer_expired(&t));
	pwr_pin_off();

	leds_on(LEDS_RED);
	printf("%s", report);
	leds_off(LEDS_RED);
	++rep_count;

	if(rep_count%10 == 0) {

		// Get the sink node name
		uint8_t node_name[NAME_LENGTH];
		cli();
		eeprom_read_block((void*)&node_name, (const void*)&eemem_node_name, NAME_LENGTH);
		sei();

		len=0;
		len += sprintf(&sinkrep[len],"RTC_T=20%d-%02d-%02d,%02d:%02d:%02d Station=%s",
		datetime.year,datetime.month, datetime.day, datetime.hours,datetime.mins,secs,node_name);

		v_in = adc_read_v_in();
		len += sprintf(&sinkrep[len]," V_IN=%-4.2f", v_in);

		SENSORS_ACTIVATE(temp_sensor);
		len += sprintf(&sinkrep[len]," T=%-5.2f", (double)(temp_sensor.value(0)*1.0/100));
		SENSORS_DEACTIVATE(temp_sensor);

		if(v_in < SYS_VLOW){
			len += sprintf(&sinkrep[len],"V_LOW=1");
		}
		v_mcu = adc_read_v_mcu();

		len += sprintf(&sinkrep[len]," V_MCU=%-4.2f REPS=%ld UP_TIME=%.2f\n",v_mcu,rep_count,up_time);

		sinkrep[len++] = '\0';
		pwr_pin_on();
	    timer_set(&t, CLOCK_SECOND/100); //5ms pulse to wake up electron from sleep
	    while(!timer_expired(&t));
	    pwr_pin_off();

		leds_on(LEDS_YELLOW);
		printf("%s", sinkrep);
		leds_off(LEDS_YELLOW);
		++rep_count;
	}
}


static const struct broadcast_callbacks broadcast_call = {broadcast_recv};
static struct broadcast_conn broadcast;


/*--------------------------------------------------------------------------------------------------------- */
//                     The Sink Node Init Process. This Handles the Sink's Own Reports
//                                     and Initializes the RTC
/*--------------------------------------------------------------------------------------------------------- */


PROCESS_THREAD(init_process, ev, data)
{
	static struct etimer et;

	PROCESS_EXITHANDLER(broadcast_close(&broadcast));
	PROCESS_BEGIN();
	broadcast_open(&broadcast, 129, &broadcast_call);
	NETSTACK_RADIO.set_value(RADIO_PARAM_CHANNEL, 26);
	/*for rpc change*/
	NETSTACK_RADIO.off();
	rf230_set_rpc(0xFF);
	NETSTACK_RADIO.on();
	pwr_pin_init();
    ds3231_init();
	etimer_set(&et, CLOCK_SECOND * 60);
	while(1) {
		PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
		etimer_reset(&et);
		up_time += (1.0/1440);
	}
	PROCESS_END();
}

/*--------------------------------------------------------------------------------------------------------- */
//             The Radio Process. This Handles the Application Level Radio Duty Cycling
/*--------------------------------------------------------------------------------------------------------- */


PROCESS_THREAD(radio_process, ev, data)
{
	static struct etimer et;
	PROCESS_BEGIN();
	while(1) {
		NETSTACK_RADIO.on();
		ps = 0;
		etimer_set(&et, CLOCK_SECOND * 60);
		PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
		NETSTACK_RADIO.off();
		ps = 1;
		etimer_set(&et, CLOCK_SECOND * 0);
		PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
	}
	PROCESS_END();
}

/*--------------------------------------------------------------------------------------------------------- */
//                     The Sleep Process. This Handles the MCU Sleep
/*--------------------------------------------------------------------------------------------------------- */


PROCESS_THREAD(sleep_process, ev, data)
{
  PROCESS_BEGIN();
  while(1) {
    watchdog_periodic();
    if(ps)
      set_sleep_mode(SLEEP_MODE_PWR_SAVE);
    else
      set_sleep_mode(SLEEP_MODE_IDLE);
    cli();
    sleep_enable();
    sei();
    sleep_cpu();
    sleep_disable();
    sei();
    PROCESS_PAUSE();
  }
  PROCESS_END();
}

/*--------------------------------------------------------------------------------------------------------- */
//                The Serial Line Process. This Handles the Serial Commands interaction
/*--------------------------------------------------------------------------------------------------------- */


PROCESS_THREAD (serial_command, ev, data)
{
	PROCESS_BEGIN();

	char delimiter[]=" ";
	char *command = NULL;
	char * value;

	for(;;) {

		PROCESS_YIELD_UNTIL(ev == serial_line_event_message);
		command = (char*)strtok((char*)data, (const char*)delimiter);

		if(!strncmp(command, "??", 2)) {
			if (strlen(command) == 2){
				display_help_Menu();
			}else {

				printf("Invalid command \*%s\*: Use \*??\* to Display a list of Available Commands\n", command);

			}
		}else if(!strncmp(command, "sinkname", 8)) {
			if(strlen(command)>8){
				value = (char*) malloc(20);
				strlcpy(value, command+9, 20);//(strlen(command)-3)
				change_node_name(value);
				free(value);
			} else {
				display_node_name();
			}
		} else {
			printf("Invalid command \*%s\*: Use \*??\* to Display a list of Available Commands\n", command);
		}
	}
	PROCESS_END();

}

/*--------------------------------------------------------------------------------------------------------- */
//                     Displaying the Help Menu For a list of Commands
/*--------------------------------------------------------------------------------------------------------- */

void display_help_Menu(void) {

	printf("******************* Available Commands ************************************\n\n");

	printf("\t1 - Displays the Help Menu with Different Commands: ??\n\n");

	printf("\t2 - Displays the current Sink Node's Name: sinkname\n\n");

	printf("\t3 - Changes the Sink Node Name to <name>: sinkname <name>\n\n");

	printf("******************* End Of the Commands ************************************\n\n");
}

/*--------------------------------------------------------------------------------------------------------- */
//                     Changing the Sink Node Name
/*--------------------------------------------------------------------------------------------------------- */

void change_node_name(char *value){

	uint16_t name_flag = 1;
	char new_name[NAME_LENGTH];
	uint8_t sinknode_name[NAME_LENGTH];
	strlcpy(new_name, value, NAME_LENGTH);
	cli();
    eeprom_update_block((const void *)&new_name, (void *)&eemem_node_name, NAME_LENGTH);
    eeprom_update_word(&eemem_name_flag, name_flag);
	sei();

	eeprom_read_block((void*)&sinknode_name, (const void*)&eemem_node_name, NAME_LENGTH);
	printf("Sink Node name changed to: %s\n", sinknode_name);

}

/*--------------------------------------------------------------------------------------------------------- */
//                     Displaying the Sink Node Name
/*--------------------------------------------------------------------------------------------------------- */

void display_node_name(){
	uint16_t name_flag;
	uint8_t node_name[NAME_LENGTH];
	cli();
	name_flag = eeprom_read_word(&eemem_name_flag);
	if (name_flag != 1) {

		printf("Sink Node name not set. Use Command \"sinkname\" to set the Name:\n");

	} else {
		eeprom_read_block((void*)&node_name, (const void*)&eemem_node_name, NAME_LENGTH);

		printf("Sink Node name is = %s\n", (char *)node_name);

	}
	sei();
}

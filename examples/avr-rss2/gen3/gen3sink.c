/**
* \file
*         WIMEA-ICT Gen3 Sink Node with RTC and SD card
* \author
*         Maximus Byamukama <maximus.byamukama@cedat.mak.ac.ug>
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <avr/io.h>
#include <avr/sleep.h>
#include "contiki.h"
#include "netstack.h"
#include "rf230bb.h"
#include "net/rime/rime.h"
#include "net/packetbuf.h"
#include "lib/list.h"
#include "dev/leds.h"
#include "dev/adc.h"
#include "dev/i2c.h"
#include "dev/ds3231.h"
#include "dev/diskio.h"
#include "dev/ff.h"
#include "dev/integer.h"
#include <dev/watchdog.h>

#define MAX_BCAST_SIZE 99
#define DEF_TTL 0xF
#define TRANSMIT_INTERVAL 30

char str_t[127], year[4], report [200],e64[16],rtc_date[20];
datetime_t datetime;
float v_in, v_mcu;
int p1,p2,lines=0; //temporary upper and lower parts of floating point variables
int prev_sec=0, curr_sec=0, secs_elapsed=0; /*Sleep function distorts timers. We'll use the rtc time to schedule transmissions after N seconds*/
uint8_t rssi,lqi;
uint16_t err=0,v_low=0; 

struct broadcast_message {
	uint8_t head; 
	uint8_t seqno;
	char buf[MAX_BCAST_SIZE+20];  /* Check for max payload 20 extra to be able to test */

};

uint8_t ttl = DEF_TTL;

FATFS FatFs;	// FatFs work area 
FIL *fp;		// file object 
UINT bw; //function result holder

//LIST(report_list);

/*---------------------------------------------------------------------------*/
PROCESS(sinknode_process, "Sink Node Process");
PROCESS(sleep_process,   "Sleep process");
AUTOSTART_PROCESSES(&sinknode_process, &sleep_process);
/*---------------------------------------------------------------------------*/
/* User Provided RTC Function called by FatFs module       */
/* Used to provide a Timestamp for SDCard files and folders*/
DWORD get_fattime (void)
{
	ds3231_get_datetime(&datetime);
	return (DWORD)(datetime.year +276  ) << 25
	| (DWORD)(datetime.month      ) << 21
	| (DWORD)(datetime.day        ) << 16
	| (DWORD)(datetime.hours      ) << 11
	| (DWORD)(datetime.mins       ) << 5
	| (DWORD)(datetime.secs/2     );
}

static void
broadcast_recv(struct broadcast_conn *c, const linkaddr_t *from)
{
	struct broadcast_message *msg;
	
	msg = packetbuf_dataptr();
	rssi = packetbuf_attr(PACKETBUF_ATTR_RSSI);
	lqi = packetbuf_attr(PACKETBUF_ATTR_LINK_QUALITY);
	
	ds3231_get_datetime(&datetime);

	sprintf(year,"20%d",datetime.year); //get 2017 from 17
	sprintf(report, "%s-%02d-%02d %02d:%02d:%02d &: %s [ADDR=%-d.%-d SEQ=%-d TTL=%-u RSSI=%-u LQI=%-u]\n",
	year,datetime.month,datetime.day, datetime.hours,datetime.mins,datetime.secs,msg->buf,
	from->u8[0], from->u8[1],msg->seqno, msg->head & 0xF, rssi,lqi); //datetime string
	printf("%s",report);

	/*write to sd card*/
	if (f_open(fp, "data.txt", FA_WRITE | FA_OPEN_ALWAYS) == FR_OK) 
	{	// Open existing or create new file
		leds_on(LEDS_RED);
		if (f_lseek(fp, f_size(fp)) == FR_OK) 
		{
			f_write(fp, report, strlen(report), &bw);	// Write data to the file
		} 	
		if (bw == strlen(report))  //we wrote the entire string
		{ 
			++lines;
			leds_off(LEDS_RED);
		}

	}else ++err;
	f_close(fp);// close the file
	
	curr_sec = datetime.secs;
	secs_elapsed = curr_sec - prev_sec;
	if(secs_elapsed <0 ) secs_elapsed += 60;
	if(secs_elapsed >= TRANSMIT_INTERVAL) {
		prev_sec=curr_sec;
		process_post(&sinknode_process, 0x10, NULL);
	}
}


static const struct broadcast_callbacks broadcast_call = {broadcast_recv};
static struct broadcast_conn broadcast;
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(sinknode_process, ev, data)
{
	static uint8_t seqno;
	struct broadcast_message msg;
	
	PROCESS_EXITHANDLER(broadcast_close(&broadcast);)
	PROCESS_BEGIN();
	broadcast_open(&broadcast, 129, &broadcast_call);	
	NETSTACK_RADIO.set_value(RADIO_PARAM_CHANNEL, 26);
	//rf230_set_channel(26);
	NETSTACK_RADIO.off(); //for rpc change
	rf230_set_rpc(0xFF); 
	NETSTACK_RADIO.on();
	/*** init rtc ****/
	ds3231_init();
	//i2c_at24mac_read(e64, 0);
	/*** init sdcard ****/
	f_mount(0, &FatFs);		// Give a work area to the FatFs module 
	fp = (FIL *)malloc(sizeof (FIL)); 
	/******************/

	while(1) {
		
		PROCESS_YIELD_UNTIL(ev==0x10);	
		leds_on(LEDS_YELLOW);
		int len=0;	
		/*get RTC datetime --this is part of the report*/
		ds3231_get_datetime(&datetime);
		sprintf(rtc_date, "20%d-%02d-%02d,%02d:%02d:%02d",datetime.year,datetime.month,
		datetime.day, datetime.hours,datetime.mins,datetime.secs);
		
		len += sprintf(&msg.buf[len],"TXT=mak-gen3 RTC_T=%s",rtc_date);
	
		v_in=adc_read_v_in();
		p1 = (int)v_in; //e.g. 4.93 gives 4
		p2 = (v_in*100)-(p1*100); // =93	
		len += sprintf(&msg.buf[len]," V_IN=%d.%02d",p1,p2);
		
		v_mcu = adc_read_v_mcu();
		p1 = (int)v_mcu; //e.g. 4.93 gives 4
		p2 = (v_mcu*100)-(p1*100); // =93	
		len += sprintf(&msg.buf[len]," V_MCU=%d.%02d SD.ERR=%d REPORTS=%d",p1,p2,err,lines);	
		if(v_in < 2.80){
			v_low=1;
			len += sprintf(&msg.buf[len]," V_LOW=1");
		}
		msg.buf[len++] = '\0';		/*null terminate the string*/
		packetbuf_copyfrom(&msg, len+2);		
		
		msg.head = 1<<4; 
		msg.head |= ttl;
		msg.seqno = seqno;
		
		broadcast_send(&broadcast);
		seqno++;
		leds_off(LEDS_YELLOW);
		printf("&: %s \n\r", msg.buf);
		if(v_low==1)
		{
		  NETSTACK_RADIO.off();
		}
	}

	PROCESS_END();
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(sleep_process, ev, data)
{
	PROCESS_BEGIN();
	while(1) {
		watchdog_periodic(); 
		set_sleep_mode(SLEEP_MODE_PWR_SAVE);
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
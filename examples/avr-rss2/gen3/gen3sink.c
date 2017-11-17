/**
* \file
*         WIMEA-ICT Gen3 Sink Node
* \details
*	ATMEGA256RFR2 RSS2 MOTE with RTC, SD card
* \author
*         Maximus Byamukama <maximus.byamukama@cedat.mak.ac.ug>
*/
#include <stdio.h>
#include <avr/io.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <avr/sleep.h>
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
#include "lib/fs/fat/diskio.h"
#include "lib/fs/fat/ff.h"
#include "lib/fs/fat/integer.h"

#define BATT_VLOW 2.70

double v_in, v_mcu,t_box; 
uint8_t rssi,lqi,seqno=0;
uint8_t p1,p2; /*temporary upper and lower parts of floating point variables*/
uint16_t err=0, batt_ok=1,lines=0,rep_count=0,len=0;
char report[200],sinkrep[200];

datetime_t datetime;

struct broadcast_message {
	uint8_t head; 
	uint8_t seqno;
	char buf[120]; 
};


FATFS FatFs;	// FatFs work area 
FIL *fp;		// file object 
UINT bw; //function result holder


/*---------------------------------------------------------------------------*/
PROCESS(sinknode_process, "Sink Node Process");
AUTOSTART_PROCESSES(&sinknode_process);
/*---------------------------------------------------------------------------*/
/* User Provided RTC Function called by FatFs module       */
/* Used to provide a Timestamp for SDCard files and folders*/
DWORD get_fattime (void)
{
	ds3231_get_datetime(&datetime);
	return (DWORD)(datetime.year + 276  ) << 25
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
	
	sprintf(report, "RTC_T=20%d-%02d-%02d,%02d:%02d:%02d %s [ADDR=%-d.%-d SEQ=%-d TTL=%-u RSSI=%-u LQI=%-u]\n",
	datetime.year,datetime.month,datetime.day, datetime.hours,datetime.mins,datetime.secs,msg->buf,
	from->u8[0], from->u8[1],msg->seqno, msg->head & 0xF, rssi,lqi); //datetime string

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
			++rep_count;
			leds_off(LEDS_RED);
		}

	}else ++err;
	f_close(fp);// close the file

}


static const struct broadcast_callbacks broadcast_call = {broadcast_recv};
static struct broadcast_conn broadcast;
/*---------------------------------------------------------------------------*/
/* sinknode_process handles sink own reports and initializes RTC and SD card */
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(sinknode_process, ev, data)
{
	static struct etimer et;
	struct broadcast_message msg;  //to be sent out
	
	PROCESS_EXITHANDLER(broadcast_close(&broadcast);)
	PROCESS_BEGIN();
	broadcast_open(&broadcast, 129, &broadcast_call);	
	NETSTACK_RADIO.set_value(RADIO_PARAM_CHANNEL, 26);
	/*for rpc change*/
	NETSTACK_RADIO.off(); 
	rf230_set_rpc(0xFF); 
	NETSTACK_RADIO.on();
	
	/*** init rtc ****/
	ds3231_init();
	leds_init();
	
	DDRE |= (1 << PWR_1); //init power pin (uplink switch)
	PORTE |= (1 << PWR_1); //and make sure its off -- pulled high
	
	
	/*** init sdcard ****/
	f_mount(0, &FatFs);		// give a work area to the FatFs module 
	fp = (FIL *)malloc(sizeof (FIL)); 
	/******************/
	
	etimer_set(&et, CLOCK_SECOND * 60);
	while(1) {	
		PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
		etimer_reset(&et);
		NETSTACK_RADIO.off(); //switch off radio to ensure broadcast_recv doesn't try to access sd card			
		leds_on(LEDS_YELLOW);
		len=0;	
		/*get RTC datetime --this is part of the report*/
		ds3231_get_datetime(&datetime);
		len += sprintf(&sinkrep[len],"RTC_T=20%d-%02d-%02d,%02d:%02d:%02d TXT=mak-g3",datetime.year,datetime.month,
		datetime.day, datetime.hours,datetime.mins,datetime.secs);
		
		v_in=adc_read_v_in();
		len += sprintf(&sinkrep[len]," V_IN=%-4.2f", v_in);
		
		
		SENSORS_ACTIVATE(temp_sensor);
		len += sprintf(&sinkrep[len]," T=%-5.2f", (double)(temp_sensor.value(0)*1.0/100));
		SENSORS_DEACTIVATE(temp_sensor);
		
		if(v_in < BATT_VLOW){
			len += sprintf(&sinkrep[len]," V_LOW=1");
		}
		
		v_mcu = adc_read_v_mcu();
		len += sprintf(&sinkrep[len]," V_MCU=%-4.2f SD.ERR=%d REPS=%d\n",v_mcu,err,lines);	
		
		sinkrep[len++] = '\0';	
		
		/*write to sd card*/
		if (f_open(fp, "data.txt", FA_WRITE | FA_OPEN_ALWAYS) == FR_OK) 
		{	// Open existing or create new file
			leds_on(LEDS_RED);
			if (f_lseek(fp, f_size(fp)) == FR_OK) 
			{
				f_write(fp, sinkrep, strlen(sinkrep), &bw);	// Write data to the file
			} 	
			if (bw == strlen(sinkrep))  //we wrote the entire string
			{ 
				++lines;
				leds_off(LEDS_RED);
			}

		}else ++err;
		f_close(fp);// close the file
		
		
		if(rep_count > 1000){ 
			
			//turn on uplink and upload before radio is turned on again
		}
		NETSTACK_RADIO.on(); //enable radio if batt voltage is ok

		//transmit		
		snprintf(msg.buf, len, "%s", (char*)sinkrep);
		msg.buf[len++]='\0';//null terminate report.
		msg.head = (1<<4) | 0xF;
		msg.seqno = ++seqno;
		packetbuf_copyfrom(&msg, len+2);
        broadcast_send(&broadcast);
		
		printf("%s \n", msg.buf);	
	}
	PROCESS_END();
}



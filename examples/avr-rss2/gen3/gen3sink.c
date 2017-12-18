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
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <avr/sleep.h>
#include <avr/power.h>
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

#define SYS_VLOW 2.80
#define UPLINK_VLOW 3.50
#define WATCHDOG_CONF_TIMEOUT WDTO_2S
#define BOOL uint8_t
#define BYTES_TO_READ 1024

double v_in, v_mcu,t_box,up_time=0;
uint8_t rssi,lqi,seqno=0;
BOOL    sd_busy=0, debug=0, upload_complete=0;
uint16_t err=0, v_low=1,len=0; 
static int rep_count=0,id_count=0,up_conns=0,up_timeouts=0,time_to_conn=0;
char report[200],sinkrep[200], rep_id[17], buff[1025];
long filesize=0;

datetime_t datetime;

struct broadcast_message {
	uint8_t head; 
	uint8_t seqno;
	char buf[120]; 
};


FATFS FatFs;    // FatFs work area 
FIL *fp;        // file object 
UINT bw; //function result holder


/*---------------------------------------------------------------------------*/
PROCESS(sinknode_process, "Sink Node Process");
PROCESS(uplink_process, "Uplink Process");
AUTOSTART_PROCESSES(&sinknode_process, &uplink_process);
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
	if(!sd_busy){
		struct broadcast_message *msg;  
		msg = packetbuf_dataptr();
		rssi = packetbuf_attr(PACKETBUF_ATTR_RSSI);
		lqi = packetbuf_attr(PACKETBUF_ATTR_LINK_QUALITY);
		
		ds3231_get_datetime(&datetime);
		
		++id_count; if(id_count>99) id_count=0;
		sprintf(rep_id,"mak%02d%02d%02d%02d%02d%02d%02d",datetime.year,datetime.month,datetime.day, 
		datetime.hours,datetime.mins,datetime.secs,id_count); //-mak17120415475601
		
		sprintf(report, "ID=%s RTC_T=20%d-%02d-%02d,%02d:%02d:%02d %s [ADDR=%-d.%-d SEQ=%-d TTL=%-u RSSI=%-u LQI=%-u]\n",
		rep_id, datetime.year,datetime.month,datetime.day, datetime.hours,datetime.mins,datetime.secs,msg->buf,
		from->u8[0], from->u8[1],msg->seqno, msg->head & 0xF, rssi,lqi);

		/*write to sd card*/
		if (f_open(fp, "sensors.dat", FA_WRITE | FA_OPEN_ALWAYS) == FR_OK) 
		{   // Open existing or create new file
			leds_on(LEDS_RED);
			if (f_lseek(fp, f_size(fp)) == FR_OK) 
			{
				f_write(fp, report, strlen(report), &bw);   // write data to the file
			}   
			if (bw == strlen(report))  //we wrote the entire string
			{
				++rep_count;
				leds_off(LEDS_RED);
			}

		}else ++err;
		f_close(fp);// close the file
		if(debug) printf("%s \n", report);
		
		
		if(err > 10){ /*sd card write has failed 10 times*/
			err=0; 
			watchdog_start();  /*watchdog will elaspse and reset mcu in 2 seconds*/
		}
		
		if(rep_count > 250){ 
			process_post(&uplink_process, 0x10, NULL);
		}
	}
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
	
	/*initialize rtc, leds and watchdog*/
	ds3231_init();
	leds_init();
	watchdog_init(); /*default is wtd disabled*/
	
	DDRE |= (1 << PWR_1); //init power pin (uplink switch)
	PORTE |= (1 << PWR_1); //and make sure its off -- pulled high
	
	
	/*** initialize  sdcard ****/
	f_mount(0, &FatFs);     // give a work area to the FatFs module 
	fp = (FIL *)malloc(sizeof (FIL)); 
	/******************/
	
	etimer_set(&et, CLOCK_SECOND * 60);
	while(1) {  
		PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
		etimer_reset(&et);
		++up_time;
		up_time /= 1440; /*get up_time in fractional days*/
		if(!sd_busy){
			NETSTACK_RADIO.off(); //switch off radio to ensure broadcast_recv doesn't try to access sd card         
			leds_on(LEDS_YELLOW);
			len=0;  
			/*get RTC datetime --this is part of the report*/
			ds3231_get_datetime(&datetime);
			++id_count; if(id_count>99) id_count=0;
			sprintf(rep_id,"mak%02d%02d%02d%02d%02d%02d%02d",datetime.year,datetime.month,datetime.day, 
			datetime.hours,datetime.mins,datetime.secs,id_count); //-mak17112915475600001
			
			len += sprintf(&sinkrep[len],"ID=%s RTC_T=20%d-%02d-%02d,%02d:%02d:%02d TXT=mak-gen3",rep_id,
			datetime.year,datetime.month, datetime.day, datetime.hours,datetime.mins,datetime.secs);
			
			v_in=adc_read_v_in();
			len += sprintf(&sinkrep[len]," V_IN=%-4.2f", v_in); 
			
			SENSORS_ACTIVATE(temp_sensor);
			len += sprintf(&sinkrep[len]," T=%-5.2f", (double)(temp_sensor.value(0)*1.0/100));
			SENSORS_DEACTIVATE(temp_sensor);
			
			if(v_in < SYS_VLOW){
				len += sprintf(&sinkrep[len]," V_LOW=1");
			}
			
			v_mcu = adc_read_v_mcu();
			len += sprintf(&sinkrep[len]," V_MCU=%-4.2f SD.ERR=%d REPS=%d UP_T=%-4.2f\n",v_mcu,err,rep_count,up_time); 

			
			sinkrep[len++] = '\0';  
			
			/*write to sd card if its not currently handled by another thread*/

			if (f_open(fp, "sensors.dat", FA_WRITE | FA_OPEN_ALWAYS) == FR_OK) 
			{   // Open existing or create new file
				leds_on(LEDS_RED);
				if (f_lseek(fp, f_size(fp)) == FR_OK) 
				{
					f_write(fp, sinkrep, strlen(sinkrep), &bw); // Write data to the file
				}   
				if (bw == strlen(sinkrep))  //we wrote the entire string
				{ 
					++rep_count;
					leds_off(LEDS_RED);
				}

			}else ++err;
			
			f_close(fp);// close the file 
			
			if(debug) printf("%s \n", sinkrep);		
			NETSTACK_RADIO.on(); /*enable radio */
			
			/*transmit diagnositic report to nearby nodes */
        snprintf(msg.buf, len, "%s", (char*)sinkrep);
        msg.buf[len++]='\0';//null terminate report.
        msg.head = (1<<4) | 0xF;
        msg.seqno = ++seqno;
        packetbuf_copyfrom(&msg, len+2);
        broadcast_send(&broadcast);
		}
	}
	PROCESS_END();
}

PROCESS_THREAD(uplink_process, ev, data) /*Electron 3G uplink*/
{
	static struct etimer et;
	static int counter=0;  
	double a2=0;	
	struct broadcast_message msg;  //diagnostic msg to be sent out
	
	PROCESS_BEGIN();
	while(1) {
		PROCESS_YIELD_UNTIL(ev==0x10);  
		ev=0;       
		sd_busy=1; //ATTN: We are taking over SD card.
		NETSTACK_RADIO.off(); 
		if(debug) printf("radio is off.\n");
		//power on modem and wait 5 seconds to start polling VA2
		PORTE &= ~ (1 << PWR_1); 
		if(debug) printf("modem power on.\n");  
		int i=0;
		while(1)
		{
			etimer_set(&et, CLOCK_SECOND/5);
			PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));  
			a2=adc_read_a2();
			if(a2 > 2.50)/* uplink is ready to receive data (sets B0-->V_A2-HIGH) */
			{
				up_conns++; /*increment number of succesful connections*/
				time_to_conn=counter/5; /*time it took to connect in seconds*/
				counter=0;
				if(debug) printf("ready to send data\n");
				etimer_set(&et, CLOCK_SECOND); /*after getting signal, wait 1 second to allow stability*/
				PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et)); 
				if (f_open(fp, "sensors.dat", FA_READ) == FR_OK) 
				{	/* read until end of file*/
					for(i=0; f_eof(fp) == 0; i++)
					{
						f_read(fp, buff, BYTES_TO_READ, &bw);
						//f_gets((char*)buff, sizeof(buff), fp);
						printf("%s", buff);					
						etimer_set(&et, CLOCK_SECOND*2);
						PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));  /*416ms write at 38400bps + 1500ms wait for uplink to send*/
					}/*f_eof returns non-zero value at end of file*/
					printf("DISCONNECT");
					etimer_set(&et, CLOCK_SECOND*3);
					PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et)); 
					upload_complete=1;
					f_close(fp);
					break;
				}
			}
			if(a2 < 2.50)  /*or 90 seconds elapsed without response*/
			{
				++counter; 
				if(counter % 5) if(debug) printf("time waiting for uplink =%d seconds.\n",counter/5);
				if(counter >450)
				{
					counter=0;
					up_timeouts++;  /*increment uplink timeouts*/
					time_to_conn=0;
					break;
				}
			}
		}   
		PORTE |= (1 << PWR_1);  //switch off uplink device
		if(debug)printf("Modem power off.\n");  
		/*clear the sd card*/
		if(upload_complete)
		{
			if (f_open(fp, "sensors.dat", FA_WRITE | FA_OPEN_ALWAYS) == FR_OK) 
			{
				f_truncate(fp);
			}
			f_close(fp);// close the file
			upload_complete=0;
			rep_count=0;
		}
		sd_busy=0;
		NETSTACK_RADIO.on();
		if(debug)printf("Radio back on. Process has completed");  
		
		/*get the file size*/
		if (f_open(fp, "sensors.dat", FA_WRITE | FA_OPEN_ALWAYS) == FR_OK) 
			{
				filesize=f_size(fp);
			}
			f_close(fp);// close the file
		
		/*transmit diagnostic report to nearby nodes */
		len=0;
		len += sprintf(&sinkrep[len],"TXT=mak-gen3 UPLINK_CONS=%d TIME_TO_CON=%d TIME_OUTS=%d F_SIZE=%ld\n",up_conns,time_to_conn, up_timeouts, filesize); 
        snprintf(msg.buf, len, "%s", (char*)sinkrep);
        msg.buf[len++]='\0';//null terminate report.
        msg.head = (1<<4) | 0xF;
        msg.seqno = ++seqno;
        packetbuf_copyfrom(&msg, len+2);
        broadcast_send(&broadcast);      		
	}
	PROCESS_END();
}
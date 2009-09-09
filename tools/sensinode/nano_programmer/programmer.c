/*
    NanoStack: MCU software and PC tools for IP-based wireless sensor networking.
		
    Copyright (C) 2006-2007 Sensinode Ltd.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

		Address:
		Sensinode Ltd.
		Teknologiantie 6	
		90570 Oulu, Finland

		E-mail:
		info@sensinode.com
*/


#include <unistd.h>
#include <getopt.h>
#include <strings.h>

#include "port.h"
#include "programmer.h"

#include <stdio.h>

extern int cdi_programmer(conf_opts_t *conf, char *filename);

void usage(char *prg_name)
{
	printf("\nUsage: %s [-d device]\n", prg_name);
	printf("General options:\n");
	printf("	-V/--version	Get programmer version\n");
	printf("	-1/--d100		Use D100 board (default D200)\n");
	printf("Operating modes:\n");
	printf("	-b/--bios to get programmer BIOS version\n");
	printf("	-P/--program [ihex file]	Do a complete programming sequence (write and verify)\n");
	printf("	-v/--verify [ihex file]		Verify against ihex file\n");
	printf("	-r/--read [ihex file]		Read program code into ihex file\n");	
	printf("	-m/--mac	Read device MAC address\n");	
	printf("	-Q/--write-mac [MAC address]	Write device MAC address\n");	
	printf("	-e/--erase			Erase flash (erases MAC address!)\n");
	printf("Programming options:\n");
	printf("	-l/--linear Force linear model for extended addresses (not SDCC file)\n");
	printf("	-s/--sdcc   Force SDCC model for extended addresses (SDCC file)\n");
	printf("Defaults:\n");
#ifndef PLATFORM_WINDOWS
	printf("device /dev/ttyUSB0\n");
#else
	printf("device 0\n");
#endif
}

conf_opts_t conf_opts;

static int option_index = 0;

int do_exit = 0;

#define OPTIONS_STRING "d:ec1lsmVbP:v:r:Q:"
/* long option list */
static struct option long_options[] =
{
  {"device", 1, NULL, 'd'},                  
  {"psoc", 0, NULL, 'p'},                
  {"d100", 0, NULL, '1'},                
  {"erase", 0, NULL, 'e'},
  {"mac", 0, NULL, 'm'},
  {"linear", 0, NULL, 'l'},
  {"sdcc", 0, NULL, 's'},
  {"cdi", 0, NULL, 'c'},
  {"version", 0, NULL, 'V'},
  {"bios", 0, NULL, 'b'},
  {"program", 1, NULL, 'P'},
  {"verify", 1, NULL, 'v'},
  {"read", 1, NULL, 'r'},
  {"write-mac", 1, NULL, 'Q'},
  {0, 0, 0, 0}
};

int parse_opts(int count, char* param[])
{
	int opt;
	int error=0;
	
	conf_opts.target_type = CDI;
  while ((opt = getopt_long(count, param, OPTIONS_STRING,
                            long_options, &option_index)) != -1)
  {
    switch(opt)
		{
			case 'V':
				conf_opts.target_type = VERSION;
				break;

			case '1':
				conf_opts.prg_type = 1;
				break;
				
			case 'c':
				conf_opts.target_type = CDI;
				break;

			case 'd':
#ifdef PLATFORM_WINDOWS
				if (sscanf(optarg, "%d", &conf_opts.device) != 1)
				{
					printf("Device ID must be a positive integer.\n");
					conf_opts.action = ' ';				
				}
#else				
				printf("device:%s\n", optarg);
				strcpy(conf_opts.device, optarg);
#endif
				break;

			case 'P':
				printf("Programming mode.\n");
				conf_opts.action = 'P';
				strcpy(conf_opts.ihex_file, optarg);
				break;

			case 's':
				if (conf_opts.page_mode == PAGE_UNDEFINED)
				{
					conf_opts.page_mode = PAGE_SDCC;
				}
				else
				{
					printf("Only one paging option allowed.\n");
					error = -1;
				}
				break;
				
			case 'l':
				if (conf_opts.page_mode == PAGE_UNDEFINED)
				{
					conf_opts.page_mode = PAGE_LINEAR;
				}
				else
				{
					printf("Only one paging option allowed.\n");
					error = -1;
				}
				break;
				
			case 'e':
				printf("Erase.\n");
				conf_opts.action = 'e';
				break;

			case 'm':
				printf("Get MAC.\n");
				conf_opts.action = 'm';
				break;

			case 'b':
				printf("Get BIOS version\n");
				conf_opts.action = 'b';
				break;
				
			case 'Q':
				printf("Write MAC.\n");
				conf_opts.action = 'Q';
				if (sscanf(optarg, "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx:%hhx:%hhx", 
														&conf_opts.write_mac[0], &conf_opts.write_mac[1],
														&conf_opts.write_mac[2], &conf_opts.write_mac[3],
														&conf_opts.write_mac[4], &conf_opts.write_mac[5],
														&conf_opts.write_mac[6], &conf_opts.write_mac[7]) != 8)
				{
					printf("Invalid MAC.\n");
					conf_opts.action = ' ';				
				}
				break;
				
			case 'v':
				printf("Verify by comparing to ihex file:%s\n", optarg);
				conf_opts.action = 'v';
				strcpy(conf_opts.ihex_file, optarg);
				break;

			case 'r':
				printf("Read program to ihex file:%s\n", optarg);
				conf_opts.action = 'r';
				strcpy(conf_opts.ihex_file, optarg);
				break;

   		case '?':
	 			printf("Duh\n");
				error = -1;
				break;
		}
	}

	if (!error && (conf_opts.target_type == CDI) )
	{
#ifdef PLATFORM_WINDOWS
 		printf("Setup: Device %d.\n", conf_opts.device);
#else
 		printf("Setup: Device %s.\n", conf_opts.device);
#endif
	}
	
	return error;
}
/*
int port_write_8byte_echo(port_t *port, char *string)
{
	int length = 0;
	int total_len;
	int i, j;
	struct pollfd pfds;
	unsigned int nfds = 1;
	int rval = 0;

	int wrbytes = 2;
	unsigned char byte8[wrbytes];

	pfds.fd = (int)(port->handle);
	pfds.events = POLLIN | POLLPRI | POLLERR | POLLHUP | POLLNVAL;
	
	total_len = strlen(string);
	
	while(total_len > length)
	{
		if(total_len - length >= wrbytes)
		{
			i=0;
			port_write(port, (unsigned char *)&string[length], wrbytes);

			if((rval = poll(&pfds, nfds, 100)) == 0)
			{
				printf("Timed out...\n");
				return(-1);
			}
			else
			{
				while(i<wrbytes)
				{
					i += port_read(port, &byte8[i], wrbytes);
				}

				if(i != wrbytes || memcmp(byte8, &string[length], wrbytes) != 0)
				{
					printf("Got wrong length (%d) or wrong bytes back.\n", i);
					for(j=0;j<wrbytes;j++)
					{
						printf("%.2x <-> %.2x\n", string[length+j], byte8[j]);
					}

					return(-1);
				}
				else
				{
					printf("8Bwok - ");
					fflush(stdout);
					length += wrbytes;
				}

			}
		}
		else
		{
			i=0;
			port_write(port, (unsigned char *)&string[length], total_len - length);

			if((rval = poll(&pfds, nfds, 100)) == 0)
			{
				printf("Timed out...\n");
				return(-1);
			}
			else
			{
				while(i<(total_len-length))
				{
					i = port_read(port, &byte8[i], total_len - length);
				}
				if(i != total_len - length || memcmp(byte8, &string[length], total_len - length) != 0)
				{
					printf("Got wrong length or wrong bytes back.\n");
					for(j=0;j<total_len - length;j++)
					{
						printf("%.2x <-> %.2x\n", string[length+j], byte8[j]);
					}
					
					return(-1);
				}
				else
				{
					printf("<8Bwok - \n");
					fflush(stdout);
					length += (total_len - length);
				}
			}
		}
	
		usleep(5000);
	
	}

	return(length);
}
*/
#ifdef PLATFORM_WINDOWS
int programmer_init(int device, port_t **port)
#else
int programmer_init(char *device, port_t **port)
#endif
{
		int error = port_open(port, device);
		uint8_t buffer[8];
		
		buffer[0] = 0;
		
		if (error >= 0)
		{
			if (conf_opts.prg_type == 1)
			{
				int retry = 0;
				port_set_params(*port, 9600, 0);
				// Activate programming...
				port_dtr_clear(*port);
				port_rts_clear(*port);
				sleep(1);
				printf("Select D100.\n");
				port_rts_set(*port);
				sleep(1);
				buffer[0] = '\r';
      	while (retry++ < 3)
				{
					int length;
					port_write_echo(*port, "q\r");
					length = port_readline(*port, buffer, sizeof(buffer), 800);
					if (length)
					{
						if (*buffer == '!')
						{
							printf("D100 found.\n");
							return 0;
						}
					}
				}
				printf("No programmer found.\n");					
				return -1;
			}
			else
			{
				port_set_params(*port, 57600, 0);
				// Activate programming...
				port_dtr_clear(*port);
				port_rts_clear(*port);
				usleep(300000);
				printf("Select D200.\n");
				port_rts_set(*port);
				usleep(200000);
				port_set_params(*port, 57600, 1);
				port_write(*port, (uint8_t *)"\r", 1);
				usleep(100000);
      	if ((port_get(*port, buffer, 500) >= 1) && (buffer[0] == '!'))
				{
					printf("D200 found.\n");					
					return 0;
				}
				printf("No programmer found.\n");					
				return -1;
			}
		}
		return error;
}

void programmer_close(port_t *port)
{
	if (port)
	{
		port_rts_clear(port);
		port_dtr_set(port);
		sleep(1);
		port_close(port);
		printf("Port closed.\n");
	}
}

int main(int argc, char *argv[])
{
	int error = 0;
	
	conf_opts.target_type = 0;
	conf_opts.action = 0;
	conf_opts.prg_type = 2;
		

#ifndef PLATFORM_WINDOWS
	strncpy(conf_opts.device, "/dev/ttyUSB0", 12);
/*    Install a new handler for SIGIO.
 *
 *    According to man 7 signal this signal is by default ignored by most systems.
 *    It seems that pre FC7 this was true for Fedoras also. We have noticed that at least
 *    on some FC7 installations the default action has changed. We avoid abnormal program
 *    exits by defining the SIGIO as SIG_IGN (ignore). - mjs
 */
    if(signal(SIGIO, SIG_IGN) == SIG_ERR)
    {
        printf("%s error: failed to install SIGIO handler. Exit.\n", argv[0]);
        exit(EXIT_FAILURE);
    }
#else
	conf_opts.device = 0;
#endif

	conf_opts.page_mode = PAGE_UNDEFINED;
	
	if ( (argc <= 1) || (error = parse_opts(argc, argv)) )
	{
		usage(argv[0]);
		if (error < 0) return error;
		else return 0;
	}
	
	if(conf_opts.target_type == CDI)
	{	/*CDI*/
		error = cdi_programmer(&conf_opts, conf_opts.ihex_file);
	}
	else
	{
		printf("\nSensinode Nano series programmer "PROGRAMMER_VERSION "\n");
	}
	return error;
}

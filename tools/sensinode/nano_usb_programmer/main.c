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


/*		
	To build use the following gcc statement 
	(assuming you have the d2xx library in the /usr/local/lib directory).
	gcc -o bitmode main.c -L. -lftd2xx -Wl,-rpath /usr/local/lib
*/
#include <unistd.h>
#include <getopt.h>
#include <strings.h>
#include <string.h>
#include <stdlib.h>

#include <stdio.h>
#ifdef PLATFORM_WINDOWS
#include <windows.h>
#endif
#include "prog.h"

#define PRG_VERSION "1.3"

typedef enum
{
	USAGE,
	VERSION,
	SCAN,
	WRITE,
	READ,
	WRITE_MAC,
	READ_MAC,
	ERASE
}mode_t;
	
typedef struct opts_t
{
	int port;
	mode_t mode;
	uint8_t write_mac[8];
	char *filename;
}opts_t;

opts_t opts;
 
void usage(char *prg_name)
{
	printf("\nUsage: %s [-p port] [-h] [-v] [-f file] [-r] [-w] [-m] [-M <mac>]\n", prg_name);
	printf("General options:\n");
	printf("	-p/--port [port]	Select FTDI device\n");
	printf("	-f/--file [filename]	File to read/write\n");
	printf("Operating modes:\n");
	printf("	-d/--devices	Scan available devices\n");
	printf("	-v/--version	Print program version\n");
	printf("	-r/--read	Read program code into ihex file (see -f)\n");	
	printf("	-w/--write	Program firmware from ihex file (see -f)\n");	
	printf("	-m/--read-mac	Read device MAC address\n");	
	printf("	-M/--write-mac xx:xx:xx:xx:xx:xx:xx:xx	Write device MAC address\n");	
	printf("	-e/--erase	Erase flash (erases MAC address!)\n");
	printf("Defaults:\n");
	printf("mode = usage\n");
	printf("port = undefined\n");
	printf("file = stdout\n");
}

int main(int argc, char *argv[])
{
	DWORD dwBytesInQueue = 0;
	FT_STATUS	ftStatus;
	FT_HANDLE	ftHandle;
	unsigned char ucMode = 0x00;
	int i=0;
	unsigned char wr[1] = { 0x30 };
	unsigned char rd[1];
	uint16_t chip_id;

	if (opts_parse(argc, argv) < 0)
	{
		usage(argv[0]);
		return -1;
	}
  switch(opts.mode)
	{
		case VERSION:
			printf("Sensinode Nano USB Programmer version %s\n", PRG_VERSION);
			return 0;

		case USAGE:
			usage(argv[0]);
			return 0;

		case SCAN:
			prog_scan();
			return 0;

		case ERASE:
		case READ:
		case WRITE:
		case READ_MAC:
		case WRITE_MAC:
			break;
	}
	
	printf("Opening programmer.\n");	
	ftHandle = prog_open(opts.port);
	if (ftHandle == 0)
	{
		return (-1);
	}
	
	cdi_start(&chip_id);
	
	printf("Chip ID = %4.4hX.\n", chip_id);
	
	if ((chip_id & 0xFF00) == 0x8500)
	{
		printf("CC2430 chip found.\n");
	}
	else if ((chip_id & 0xFF00) == 0x8900)
	{
		printf("CC2431 chip found.\n");
	}
	else
	{
		printf("Unknown chip found.\n");
		opts.mode = USAGE;
	}
		
  switch(opts.mode)
	{
		case VERSION:
		case USAGE:
			break;

		case ERASE:
			printf("Erase.\n");
			break;

		case READ:
			printf("Read Flash.\n");
			cdi_set_address(0);
			break;

		case WRITE:
		{
			int rval;
			unsigned char page_table[64];
			unsigned char page_buffer[128*1024];
			
			printf("Write Flash.\n");
			
			if((rval = hexfile_build_tables(opts.filename, page_buffer, page_table)) == -1)
			{
				printf("Error\n");
				return(-1);
			}
			else if(rval == 0)
			{
				printf(".ihex file OK but nothing to write...\n");
				return(1);
			}
			
			hexfile_program(page_buffer, page_table);
			
			break;
		}
		case READ_MAC:
			printf("Read MAC: ");
			cdi_set_address(0x1FFF8);
			{
				uint8_t mac[8];
				
				cdi_flash_read(mac, 8);
				for (i=0; i<8; i++)
				{
					if (i) printf(":");
					printf("%2.2X", mac[i]);
				}				
				printf("\n");
			}
 			break;

		case WRITE_MAC:
			printf("Write MAC: ");
/*			cdi_set_address(0x1F800);
			{
				uint8_t block[2048];
				
				memset(block, 0xFF, 2048);
				for (i=0; i<8; i++)
				{
					block[2040+i] = opts.write_mac[i];
				}				
				cdi_flash_write(block, 2048);
				printf("\n");
			}*/
			cdi_flash_write_mac(opts.write_mac);
			printf("\n");
			break;

   	default:
	 		printf("Duh\n");
			break;
	}
	
	
	printf("Closing programmer.\n");	
	prog_close();
}

static int option_index = 0;

int do_exit = 0;

#define OPTIONS_STRING "p:vdhf:rwmM:e"
/* long option list */
static struct option long_options[] =
{
  {"port", 1, NULL, 'p'},                  
  {"version", 0, NULL, 'v'},
  {"devices", 0, NULL, 'd'},
  {"help", 0, NULL, 'h'},                
  {"file", 1, NULL, 'f'},                
  {"read", 0, NULL, 'r'},                
  {"write", 0, NULL, 'w'},                
  {"read-mac", 0, NULL, 'm'},                
  {"write-mac", 1, NULL, 'M'},                
  {"erase", 0, NULL, 'e'},                
  {0, 0, 0, 0}
};

int opts_parse(int count, char* param[])
{
	int opt;
	int error=0;

	opts.mode = USAGE;
	opts.filename = 0;
  while ((opt = getopt_long(count, param, OPTIONS_STRING,
                            long_options, &option_index)) != -1)
  {
		fflush(stdout);
    switch(opt)
		{
			case 'p':
				opts.port = 0;
				if (sscanf(optarg, "%d",	&(opts.port)) != 1)
				{
					if (sscanf(optarg, "/dev/ttyUSB%d",	&(opts.port)) != 1)
					{
						printf("Invalid port.\n");
						opts.mode = USAGE;
						return 0;
					}
				}
				printf("Port %d.\n", opts.port);
				break;
				
			case 'v':
				opts.mode = VERSION;
				return 0;
				
			case 'd':
				opts.mode = SCAN;
				return 0;

			case 'h':
				opts.mode = USAGE;
				return 0;

			case 'e':
				opts.mode = ERASE;
				break;

			case 'f':
				printf("Filename: %s\n", optarg);
				opts.filename = malloc(strlen(optarg)+1);
				strcpy(opts.filename, optarg);
				break;
				
			case 'r':
				opts.mode = READ;
				break;

			case 'w':
				opts.mode = WRITE;
				break;
				
			case 'm':
				opts.mode = READ_MAC;
				break;

			case 'M':
				opts.mode = WRITE_MAC;
				if (sscanf(optarg, "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx:%hhx:%hhx", 
														&opts.write_mac[0], &opts.write_mac[1],
														&opts.write_mac[2], &opts.write_mac[3],
														&opts.write_mac[4], &opts.write_mac[5],
														&opts.write_mac[6], &opts.write_mac[7]) != 8)
				{
					printf("Invalid MAC.\n");
					opts.mode = USAGE;				
				}
				else
				{
					printf("MAC to write: %2.2hhX:%2.2hhX:%2.2hhX:%2.2hhX:%2.2hhX:%2.2hhX:%2.2hhX:%2.2hhX\n", 
														opts.write_mac[0], opts.write_mac[1],
														opts.write_mac[2], opts.write_mac[3],
														opts.write_mac[4], opts.write_mac[5],
														opts.write_mac[6], opts.write_mac[7]);
				}
				break;

   		case '?':
	 			printf("Duh\n");
				error = -1;
				break;
		}
	}
	
	return error;
}


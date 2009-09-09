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
#include "ihex.h"

#include <stdio.h>

extern void crc_add(unsigned char *crc, unsigned char byte);

int cdi_page_write(port_t *port, unsigned long page_addr, unsigned char *page_buffer);
int cdi_write(port_t *port, conf_opts_t *conf, FILE *ihex);

int cdi_programmer(conf_opts_t *conf, char *filename)
{
	int error = 0;
	port_t *port = 0;
	unsigned char buffer[256];
	int length = 0;
	FILE *ihex = 0;

	error = programmer_init(conf->device, &port);
	
	if (error < 0)
	{
		return error;
	}

	if((!error) && (conf->action != 'b'))
	{
		length = port_write_echo(port, "CDI\r");
		bzero(buffer, sizeof(buffer));
		if (length >= 4)
		{
			length = port_readline(port, buffer, sizeof(buffer), 100);
		}
		else length = 0;

		if(memcmp(buffer, "OK", 2) == 0)
		{
			error = 0;
		}
		else 
		{
			printf("Programming mode selection failed.\n");
			error = -1;
		}
	}
	
	if((!error) && (conf->action != 'b'))
	{
		printf("Initialize.\n");
		// Succesfully in mode 1
		sleep(1);
		port_write_echo(port, "i\r");

		bzero(buffer, 256);
		length = port_readline(port, buffer, sizeof(buffer), 100);

		if(memcmp(buffer, "85", 2) == 0)
		{	/*Found CC2430 device*/
			printf("Found CC2430 device revision %c%c.\n", buffer[2], buffer[3]);
		}
		else if (memcmp(buffer, "89", 2) == 0)
		{
			printf("Found CC2431 device revision %c%c.\n", buffer[2], buffer[3]);
		}
		else
		{
			printf("CC2430 not found.\n");
			error = -1;
		}
	}

	if (error) conf->action = ' ';
	
	switch(conf->action)
	{
		case 'e':
			// Erase programming
			port_write_echo(port, "e\r");
			bzero(buffer, 256);
			length = port_readline(port, buffer, sizeof(buffer), 100);

			if(memcmp(buffer, "OK", 2) == 0)
			{
				// Erase successful
				printf("Erase successful.\n");
				error = 0;
			}
			else
			{
				// Erase failed
				printf("Erase failed: %s.\n", buffer);
				error = -1;
			}
			if ((conf->action != 'P') || error)
				break;
		
		case 'P':
		case 'w':
			ihex = fopen(conf->ihex_file, "rb");
			if(ihex == NULL)
			{
				printf("Failed to open ihex file %s.\n", conf->ihex_file);
				error = -1;
			}
			else error = 0;

			if (!error)
			{
				error = cdi_write(port, conf, ihex);
				if (error) printf("Programming failed.\n");
			}
			
	

			if (ihex != NULL) fclose(ihex);							
			break;
		
		case 'b':
			length = port_write_echo(port, "V\r");
			bzero(buffer, sizeof(buffer));
			if (length >= 2)
			{
				length = port_readline(port, buffer, sizeof(buffer), 100);
			}
			else length = 0;

			if(length > 4)
			{
				buffer[length] = 0;
				printf("BIOS: %s\n", buffer);
				error = 0;
			}
			else 
			{
				printf("Failed to get BIOS version. Upgrade recommended.\n");
				error = -1;
			}
			break;

		case 'v':
			break;

		case 'r':
			ihex = fopen(conf->ihex_file, "wb");
			if(ihex == NULL)
			{
				printf("Failed to open ihex file %s.\n", conf->ihex_file);
				error = -1;
			}
			else
			{
				port_write_echo(port, "a000000\r");
				bzero(buffer, sizeof(buffer));
				length = port_readline(port, buffer, sizeof(buffer), 200);
				if (length <0) length = 0;
				if(memcmp(buffer, "OK", 2) == 0)
				{
					uint32_t address = 0;
					uint8_t check = 0;
					for (address = 0; address < 128*1024; address += 64)
					{
						uint8_t i;

						if ((address) && ((address & 0xFFFF)==0))
						{
							fprintf(ihex, ":02000004%4.4X%2.2X\r\n", 
									         (int)(address>>16), (int)(0xFA-(address>>16)));
						}
						port_write_echo(port, "r\r");
						bzero(buffer, 256);
						length = 0;
						while (length < 64)
						{
							length += port_readline(port, &buffer[length], sizeof(buffer)-length, 100);
						}
						for (i=0; i<64; i++)
						{
							if ((i & 0x0F) == 0)
							{
								check = 0;
								check -= 0x10;
								check -= (uint8_t) (address >> 8);
								check -= (uint8_t) (address + i);
								printf("%4.4X", (int) address + i);
								fprintf(ihex, ":10%4.4X00", (int) (address + i) & 0xFFFF);
							}
							fprintf(ihex, "%2.2X", buffer[i]);
							check -= buffer[i];
							if ((i & 0x0F) == 0x0F)
							{
									fprintf(ihex, "%2.2X\r\n", check);
									if (i > 0x30) printf("\n");
									else printf(" ");
							}
						}
					}
					fprintf(ihex, ":00000001FF\r\n");
				}
				else
				{
					printf("Failed to set read address.\n");
					error = -1;
				}
				fclose(ihex);
			}
			break;
		/*skip for error case*/
		case ' ':
			break;
			
		case 'm':
			port_write_echo(port, "a01F800\r");
			bzero(buffer, sizeof(buffer));
			length = port_readline(port, buffer, sizeof(buffer), 200);
			if (length <0) length = 0;
			if(memcmp(buffer, "OK", 2) == 0)
			{
				uint8_t i;
				uint32_t address = 0;

				for (address = 0x01F800; address < 128*1024; address += 64)
				{

					port_write_echo(port, "r\r");
					bzero(buffer, 256);
					length = 0;
					while (length < 64)
					{
						length += port_readline(port, &buffer[length], sizeof(buffer)-length, 100);
					}
					if ((address & 0xff) == 0)
					{	printf(".");
						fflush(stdout);
					}
				}
				printf("\nDevice MAC: ");
				for (i=56; i<64; i++)
				{
					if (i != 56) printf(":");
					printf("%2.2X", buffer[i]);
				}
				printf("\n");
			}
			break;			
			
		case 'Q':
			port_write_echo(port, "a01F800\r");
			bzero(buffer, sizeof(buffer));
			length = port_readline(port, buffer, sizeof(buffer), 200);
			if (length <0) length = 0;
			if(memcmp(buffer, "OK", 2) == 0)
			{
				uint8_t p_buffer[2048];
				int error;
				
				memset(p_buffer, 0xff, sizeof(p_buffer));
				memcpy(&p_buffer[2040], conf->write_mac, 8);
				
				printf("\rWriting MAC: ");
				error = cdi_page_write(port, 0x01F800, p_buffer);
				if (!error)
				{
					printf("Write complete.\n");
				}
				else
				{
					printf("Write failed.\n");
				}
			}
			break;			

		default:
			printf("Unknown CDI action.\n");
			break;
	}
	
	printf("Close programmer.\n");
	usleep(100000);
	port_write_echo(port, "q\r");
	programmer_close(port);
	return error;
}

int cdi_write(port_t *port, conf_opts_t *conf, FILE *ihex)
{
	int error = 0;
	unsigned char buffer[256];
	int length;
	int i;
	int pages;
	
	unsigned long ext_addr=0;
	unsigned short int addr=0;
	unsigned char page_buffer[128*1024];
	unsigned char page_table[64];

	bzero(buffer, sizeof(buffer));

	/*initialize page data*/
	memset(page_table, 0, 64);
	memset(page_buffer, 0xFF, sizeof(page_buffer));
	pages = 0;

	error = 0;
	
	if (conf->page_mode == PAGE_UNDEFINED)
	{
		int retval;
		
		while((!error) && ((retval = fscanf(ihex, "%s", buffer)) == 1) )
		{	
			unsigned char data_len = 0;

			if (memcmp(&buffer[7], "00", 2) == 0)
			{	/*Data record*/
			}
			else if (memcmp(&buffer[7], "01", 2) == 0)
			{	/*end file*/
				printf("\nFile read complete.\n");
				break;
			}
			else if (memcmp(&buffer[7], "04", 2) == 0)
			{
				sscanf((char *)&(buffer[3]),"%4hx", &addr);
				sscanf((char *)&(buffer[9]),"%4lx", &ext_addr);

				if (ext_addr >= 0x0002)
				{
					 conf->page_mode = PAGE_SDCC;
				}
				else
				{
					if (conf->page_mode == PAGE_UNDEFINED) conf->page_mode = PAGE_LINEAR;
				}
			}
		}
		if (retval == -1)
		{
			printf("Read error\n");
			return -1;
		}
		rewind(ihex);
		retval = 0;
		error = 0;
	}
	switch (conf->page_mode)
	{
		case PAGE_SDCC:
			printf("SDCC banked file.\n");
			break;
		case PAGE_LINEAR:
			printf("Linear banked file.\n");
			break;
		case PAGE_UNDEFINED:
			printf("Non-banked file, assuming linear.\n");
			conf->page_mode = PAGE_LINEAR;
			break;
	}
	
	while( (fscanf(ihex, "%s", buffer) == 1) && !error)
	{	
		unsigned char data_len = 0;

		if (memcmp(&buffer[7], "00", 2) == 0)
		{	/*Data record*/
			i=0;
			sscanf((char *)&buffer[1], "%2hhx", &data_len);
			sscanf((char *)&(buffer[3]),"%4hx", &addr);
			while(i<data_len)
			{
				uint32_t absolute_address = ext_addr+addr+i;
				
				if (page_table[absolute_address/2048] == 0) 
				{
					page_table[absolute_address/2048] = 1;
					pages++;
				}
				sscanf((char *)&buffer[2*i+9], "%2hhx", &page_buffer[absolute_address]);
				i++;
			}
		}
		else if (memcmp(&buffer[7], "01", 2) == 0)
		{	/*end file*/
			printf("\nFile read complete.\n");
			printf("Writing %d pages.\n", pages);
			break;
		}
		else if (memcmp(&buffer[7], "04", 2) == 0)
		{
			sscanf((char *)&(buffer[3]),"%4hx", &addr);
			sscanf((char *)&(buffer[9]),"%4lx", &ext_addr);
			if (conf->page_mode == PAGE_SDCC)
			{
				if (ext_addr) ext_addr--;
				ext_addr *= 0x8000;
			}
			else
			{
				ext_addr *= 0x10000;
			}
			printf("\rExtended page address: 0x%8.8lX\r", ext_addr);
		}
	}

	if (pages)
	{
		int retry = 0;
		// Successfully in mode 3 (programming)
		printf("Starting programming.\n");
		error = 0;
		for (i=0; i<64; i++)
		{
			if (page_table[i] != 0)
			{
				ext_addr = 2048*i;
				
				bzero(buffer, sizeof(buffer));

				// Write the start address and check return
				usleep(3000);
				sprintf((char *)buffer, "a%6.6lX\r", ext_addr);
				port_write_echo(port, (char *)buffer);

				if((length = port_readline(port, buffer, sizeof(buffer), 200)) < 0)
				{
					printf("Read from serial timed out without data.\n");
					error = -1;
					break;
				}
				else
				{
					if(strncmp((char *)buffer, "OK\r\n", 4) == 0)
					{
						printf("\r                                                       \r");
						printf("\rWriting @ 0x%6.6lX: ", ext_addr);
						fflush(stdout);
						error = cdi_page_write(port, ext_addr, &page_buffer[ext_addr]);
						if (error)
						{
							usleep(20000);
							port_write_echo(port, "i\r");

							bzero(buffer, 256);
							length = port_readline(port, buffer, sizeof(buffer), 100);

							if(memcmp(buffer, "85", 2) == 0)
							{	/*Found CC2430 device*/
							}
							else
							{
								printf("Reinit failed.\n");
								error = -1;
							}
							if (retry++ < 3)
							{
								error = 0;
								i--;
							}
						}
						else retry = 0;
						fflush(stdout);
						usleep(20000);
					}
					else
					{
						printf("Failed to set CDI programming start address.\n");
						error = -1;
						break;
					}
				}
			}
			if (error) break;
		}
		usleep(200000);
		printf("\n");
	}
	
	return error;
}

int cdi_page_write(port_t *port, unsigned long page_addr, unsigned char *page_buffer)
{
	int error = 0;
	unsigned char buffer[80];
	unsigned char cmd[16];
	unsigned char block, i;
	int length;
	int retry = 0;
	
	// Write page
	port_write_echo(port, "w\r");
	usleep(10000);
	for (block=0; block<(2048/64); block++)
	{
		sprintf((char *)cmd, "%6.6lX", page_addr + (64*block));
		bzero(buffer, sizeof(buffer));
		length = port_readline(port, buffer, sizeof(buffer), 2000);
		if (length <0)
		{ length = 0;
			printf("l!");fflush(stdout);
		}
		buffer[length] = 0;
		if (block & 1)
		{
		}
		if(memcmp(buffer, cmd, 6) == 0)
		{
#define WRITE_SIZE 64
			for (i=0; i<64; i+=WRITE_SIZE)
			{
				port_write(port, &page_buffer[(unsigned int)(block*64)+i], WRITE_SIZE);
				usleep(1250);
			}
			
			bzero(buffer, sizeof(buffer));
			printf(".");
			fflush(stdout);
			length = port_readline(port, buffer, sizeof(buffer), 200);
			if(memcmp(buffer, "OK", 2) == 0)
			{
				retry = 0;
			}
			else
			{
				block--;
				if (retry++ >= 8)
				{
					error = -1;
					break;
				}
				else
				{
					buffer[length] = 0;
					printf("%s",buffer);
					port_rts_clear(port);
					usleep(300000);
					port_rts_set(port);
					bzero(buffer, sizeof(buffer));
					length = port_readline(port, buffer, sizeof(buffer), 800);
					if(memcmp(buffer, "CDI", 3) == 0)
					{
						printf("R");
					}
				}
			}
		}
		else
		{
			error = -1;
			break;
		}
	}

	if (!error)
	{
		printf("w"); fflush(stdout);
		bzero(buffer, sizeof(buffer));
		length = port_readline(port, buffer, sizeof(buffer), 800);
		if(memcmp(buffer, "WROK", 4) == 0)
		{
			error = 0;
		}
		else
		{
			printf("%c%c", buffer[0], buffer[1]);
			error = -1;
		}
	}
	
	if (!error) printf("OK\r");
	return error;
}


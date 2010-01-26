#include <unistd.h>
#include <getopt.h>
#include <string.h>
#include <inttypes.h>

#include "converter.h"

#include <stdio.h>

extern int cdi_programmer(conf_opts_t *conf, char *filename);

void usage(char *prg_name)
{
	printf("\nUsage: %s -f ihex file\n", prg_name);
	printf("General options:\n");
	printf("	-V/--version	Get converter version\n");
}

conf_opts_t conf_opts;

static int option_index = 0;

int do_exit = 0;

#define OPTIONS_STRING "Vhf:"
/* long option list */
static struct option long_options[] =
{
  {"version", 0, NULL, 'V'},
  {"file", 1, NULL, 'f'},
  {"help", 0, NULL, 'h'},
  {0, 0, 0, 0}
};

int parse_opts(int count, char* param[])
{
	int opt;
	int error=0;
	
	conf_opts.target_type = UNDEFINED;
  while ((opt = getopt_long(count, param, OPTIONS_STRING,
                            long_options, &option_index)) != -1)
  {
    switch(opt)
		{
			case 'V':
				conf_opts.target_type = VERSION;
				break;

			case 'h':
				conf_opts.target_type = UNDEFINED;
				break;

   		case 'f':
				strcpy(conf_opts.ihex_file, optarg);
				conf_opts.target_type = CONVERT;
				break;
		}
	}

	if (!error && (conf_opts.target_type == CONVERT) )
	{
 		printf("File: %s.\n", conf_opts.ihex_file);
	}
	
	return error;
}

int main(int argc, char *argv[])
{
	int error = 0;
	
	conf_opts.target_type = 0;
		

	if ( (argc < 1) || (error = parse_opts(argc, argv)) )
	{
		usage(argv[0]);
		if (error < 0) return error;
		else return 0;
	}
	
	if(conf_opts.target_type == CONVERT)
	{	/*Convert*/
		int pages;
		int sdcc_file = 0;
		
		FILE *ihex = 0;	
		unsigned char check = 0;
		unsigned long ext_addr=0;
		unsigned short int addr=0;
		unsigned char page_buffer[128*1024];
		unsigned char page_table[64];
		unsigned char buffer[256];
		int i;
		int retval = 0;
		
		bzero(buffer, sizeof(buffer));

		/*initialize page data*/
		memset(page_table, 0, 64);
		memset(page_buffer, 0xFF, sizeof(page_buffer));
		pages = 0;
		
		ihex = fopen(conf_opts.ihex_file, "rb");
		if (ihex == 0)
		{
			printf("Can't open file.\n");
			return -1;
		}
		error = 0;
		while((!error) && ((retval = fscanf(ihex, "%s", buffer)) == 1) )
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
				break;
			}
			else if (memcmp(&buffer[7], "04", 2) == 0)
			{
				sscanf((char *)&(buffer[3]),"%4hx", &addr);
				sscanf((char *)&(buffer[9]),"%4lx", &ext_addr);
				printf("\rExtended page address: 0x%8.8lX\r", ext_addr*0x8000 );
				
				if (ext_addr >= 0x0002) sdcc_file = 1;
				
				if (ext_addr) ext_addr--;
				ext_addr *= 0x8000;
			}
		}
		fclose(ihex);
		if (retval == -1)
		{
			printf("Read error\n");
		}
		if (sdcc_file == 0)
		{
			printf("Not a SDCC banked file.\n");
			return 0;
		}
		printf("Writing %d pages.\n", pages);
		{	/*cut off extension*/
			char *ptr = strrchr(conf_opts.ihex_file, '.');
			if (ptr != NULL)
			{
				*ptr = 0;
			}
		}
		strcat(conf_opts.ihex_file, "_linear.hex");
 		printf("Output file: %s.\n", conf_opts.ihex_file);
		ihex = fopen(conf_opts.ihex_file, "wb");
		ext_addr=0;
		addr = 0;
		if (pages)
		{
			int j;
			error = 0;
			for (i=0; i<64; i++)
			{
				addr = (i & 0x1F) * 2048;
				if ( ((i / 32) * 0x10000) != ext_addr)
				{	/*write out ext addr*/
					printf("Ext: %4.4X\n", ((i / 32) * 0x10000));
					ext_addr = (i / 32) * 0x10000;
					fprintf(ihex, ":02000004%4.4X%2.2X\r\n", 
									         (int)(ext_addr>>16), (int)(0xFA-(ext_addr>>16)));
				}
								
				if (page_table[i] != 0)
				{
					printf("%4.4X", addr & 0xF800);
					for (j=0; j<2048; j++)
					{
						addr =(i & 0x1F) * 2048 + j;
						if ((j & 0x1F) == 0)
						{
							check = 0;
							check -= 0x20;
							check -= (uint8_t) (addr >> 8);
							check -= (uint8_t) (addr);
							fprintf(ihex, ":20%4.4X00", (int) addr);
						}
						fprintf(ihex, "%2.2X", page_buffer[ext_addr+addr]);
						check -= page_buffer[ext_addr+addr];
						if ((j & 0x1F) == 0x1F)
						{
								fprintf(ihex, "%2.2X\r\n", check);								
						}
					}
				}
				if ((i & 0x07) == 0x07) printf("\n");
				else printf(" ");
			}
			fprintf(ihex, ":00000001FF\r\n");
			printf("Write complete.\n");
		}
		fclose(ihex);
	}
	else if(conf_opts.target_type == UNDEFINED)
	{
		usage(argv[0]);
	}
	else
	{
		printf("\nSensinode hex file converter "CONVERTER_VERSION "\n");
	}
	return error;
}

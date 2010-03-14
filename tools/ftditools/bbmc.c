/* control reset and VREF2 lines */

#include <stdio.h>
#include <string.h>
#include <getopt.h>
#include <ftdi.h>

#define DEBUG 0

#define low(x)  (1 << x)
#define high(x) (1 << (x + 8))

#define REDBEE_ECONOTAG_RESET    high(2)
#define REDBEE_ECONOTAG_VREF2L   high(7)
#define REDBEE_ECONOTAG_VREF2H   high(6) 
#define REDBEE_ECONOTAG_INTERFACE INTERFACE_A

#define REDBEE_USB_RESET    high(2)
#define REDBEE_USB_VREF2L   low(5)
#define REDBEE_USB_VREF2H   low(6)
#define REDBEE_USB_INTERFACE INTERFACE_B

#define BOARD REDBEE_USB

#define STR(x)         #x
#define STR2(x)        STR(x)
#define CAT(x,y)       x##y
#define CAT2(x, y, z)  x##y##z

#define dir(x)            ( CAT(x,_RESET) | CAT(x,_VREF2L) | CAT(x,_VREF2H))
#define interface(x)      ( CAT(x,_INTERFACE) )
#define reset_release(x)  ( CAT(x,_RESET)     )
#define reset_set(x)      ( 0 )
#define vref2_normal(x)   ( CAT(x,_VREF2H)    )
#define vref2_erase(x)    ( CAT(x,_VREF2L)    )

/* fgets input buffer length: for prompts and such */
#define BUF_LEN 32

int print_and_prompt( struct ftdi_device_list *devlist, int num_devs);
int bb_mpsee(struct ftdi_context *ftdic, uint16_t dir, uint16_t val); 
void toggle_reset(struct ftdi_context *ftdic);

static uint32_t vendid = 0x0403; uint32_t prodid = 0x6010;

int main(int argc, char **argv) 
{
	struct ftdi_context ftdic;
	struct ftdi_device_list *devlist;
	int dev_index = -1; int num_devs;
	int ret;

	while (1) {
		int c;
		int option_index = 0;
		static struct option long_options[] = {
			{"index", required_argument, 0, 'i'},
			{"help",        no_argument, 0, '?'},
			{0, 0, 0, 0}
		};
		
		c = getopt_long (argc, argv, "i:",
				 long_options, &option_index);
		if (c == -1)
			break;
		
		switch (c) {
			/* process long opts */
		case 'i':
			dev_index = atoi(optarg);
			printf("index %d\n", dev_index);
		default:
			printf("Usage: don't know yet\n");
		}    
	}

	if ((num_devs = ftdi_usb_find_all(&ftdic, &devlist, vendid, prodid)) < 0)
	{
		fprintf(stderr, "ftdi_usb_find_all failed: %d (%s)\n", 
			num_devs, 
			ftdi_get_error_string(&ftdic));
		return EXIT_FAILURE;
	}
	
	if (ftdi_init(&ftdic) < 0)
	{
		fprintf(stderr, "ftdi_init failed\n");
		return EXIT_FAILURE;
	}

	if (ftdi_set_interface(&ftdic, interface(BOARD)) < 0) {
		fprintf(stderr, "couldn't set interface %d\n", interface(BOARD));
		return EXIT_FAILURE;
	}

	if(num_devs == 1) { dev_index = 0; }
	while( (dev_index < 0) || (dev_index >= num_devs)){
		dev_index = print_and_prompt(devlist, num_devs);
	}
	
	if( (ret = ftdi_usb_open_desc_index(
		     &ftdic,
		     vendid,
		     prodid,
		     NULL,
		     NULL,
		     dev_index)) < 0) {
		fprintf(stderr, "couldn't open dev_index %d\n", dev_index);
		return EXIT_FAILURE;
	}
	toggle_reset(&ftdic);
	
	ftdi_list_free(&devlist);
	ftdi_deinit(&ftdic);
	
	return EXIT_SUCCESS;
}

int print_and_prompt( struct ftdi_device_list *devlist, int num_devs) 
{
	int i, ret;
	struct ftdi_context ftdic;
	struct ftdi_device_list *curdev;
	char manufacturer[128], description[128], serial[128];
	char input[BUF_LEN]; char *s;
	int sel = -1;

	printf("Found %d devices with vendor id 0x%04x product id 0x%04x\n\n", 
	       num_devs, vendid, prodid);


	i = 0;
	for (curdev = devlist; curdev != NULL; i++)
	{
		printf("  [%d]   ", i);
		if (0 > (ret = ftdi_usb_get_strings(&ftdic, 
						    curdev->dev, 
						    manufacturer, 128, 
						    description, 128, 
						    serial, 128)))
		{
			fprintf(stderr, "ftdi_usb_get_strings failed: %d (%s)\n", 
				ret, ftdi_get_error_string(&ftdic));
			return EXIT_FAILURE;
		}
		printf("Manufacturer: %s, Description: %s, Serial %s\n", 
		       manufacturer, description, serial);
		curdev = curdev->next;
	}

	printf("\nUse which device? ");

	s = fgets(input, BUF_LEN, stdin);
	if (s != NULL) {
		size_t last = strlen (input) - 1;		
		if (input[last] == '\n') input[last] = '\0';
	}

	sscanf(s, "%i",&sel);

	return sel;
}

void toggle_reset(struct ftdi_context *ftdic) 
{
	printf("toggle reset\n");
	
        /* using MPSSE since it give access to high GPIO*/
	/* set as inputs for now */
	ftdi_set_bitmode(ftdic, 0 , BITMODE_MPSSE); 

	bb_mpsee(ftdic, dir(BOARD), (reset_release(BOARD) | vref2_normal(BOARD)));
	bb_mpsee(ftdic, dir(BOARD), (reset_set(BOARD)     | vref2_normal(BOARD)));
	bb_mpsee(ftdic, dir(BOARD), (reset_release(BOARD) | vref2_normal(BOARD)));

	return;

}


int bb_mpsee(struct ftdi_context *ftdic, uint16_t dir, uint16_t val) 
{
	uint8_t buf[3];
	int ret;

	/* command "set data bits low byte" */
	buf[0] = 0x80;
	buf[1] = (val & 0xff);
	buf[2] = dir & 0xff;
#if DEBUG
	fprintf(stderr,"write %x %x %x\n",buf[0],buf[1],buf[2]);
#endif

	if ((ret = (ftdi_write_data(ftdic, buf, 3))) < 0)
	{
		perror("ft2232_write error");
		fprintf(stderr, "ft2232_write command %x\n", buf[0]);
		return EXIT_FAILURE;
	}


	/* command "set data bits high byte" */
	buf[0] = 0x82;         
	buf[1] = (val >> 8);
	buf[2] = dir >> 8;
#if DEBUG
	fprintf(stderr,"write %x %x %x\n",buf[0],buf[1],buf[2]);
#endif

	if ((ret = (ftdi_write_data(ftdic, buf, 3))) < 0)
	{
		perror("ft2232_write error");
		fprintf(stderr, "ft2232_write command %x\n", buf[0]);
		return EXIT_FAILURE;
	}

	return 0;

}

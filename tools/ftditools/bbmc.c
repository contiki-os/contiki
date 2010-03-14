/* control reset and VREF2 lines */

#include <stdio.h>
#include <getopt.h>
#include <ftdi.h>

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

#define BOARD REDBEE_ECONOTAG

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

int print_and_prompt(struct ftdi_device_list *devlist);
void toggle_reset(struct ftdi_context *ftdic);

int main(int argc, char **argv) 
{
	int ret;
	struct ftdi_context ftdic;
	struct ftdi_device_list *devlist;
	uint32_t vendid = 0x0403; uint32_t prodid = 0x6010;
	int devindex = 0; 

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
			devindex = atoi(optarg);
			printf("index %d\n", devindex);
		default:
			printf("Usage: don't know yet\n");
		}    
	}

	if ((ret = ftdi_usb_find_all(&ftdic, &devlist, vendid, prodid)) < 0)
	{
		fprintf(stderr, "ftdi_usb_find_all failed: %d (%s)\n", ret, ftdi_get_error_string(&ftdic));
		return EXIT_FAILURE;
	}
	
	printf("Number of FTDI devices found: %d\n", ret);

	if (ftdi_init(&ftdic) < 0)
	{
		fprintf(stderr, "ftdi_init failed\n");
		return EXIT_FAILURE;
	}

	if (ftdi_set_interface(&ftdic, interface(BOARD)) < 0) {
		fprintf(stderr, "couldn't set interface %d\n", interface(BOARD));
		return EXIT_FAILURE;
	}
	
	if( (ret == 1) /*|| device number passed on cmdline */ ) {
		if( (ret = ftdi_usb_open_desc_index(
			     &ftdic,
			     vendid,
			     prodid,
			     NULL,
			     NULL,
			     devindex)) < 0) {
			fprintf(stderr, "couldn't open devindex %d\n", devindex);
			return EXIT_FAILURE;
		}
		toggle_reset(&ftdic);
	} else {
		print_and_prompt(devlist);
	}
	
	ftdi_list_free(&devlist);
	ftdi_deinit(&ftdic);
	
	return EXIT_SUCCESS;
}

int print_and_prompt( struct ftdi_device_list *devlist ) 
{
	int i, ret;
	struct ftdi_context ftdic;
	struct ftdi_device_list *curdev;
	char manufacturer[128], description[128];

	i = 0;
	for (curdev = devlist; curdev != NULL; i++)
	{
		printf("Checking device: %d\n", i);
		if (0 > (ret = ftdi_usb_get_strings(&ftdic, 
						    curdev->dev, 
						    manufacturer, 128, 
						    description, 128, 
						    NULL, 0)))
		{
			fprintf(stderr, "ftdi_usb_get_strings failed: %d (%s)\n", 
				ret, ftdi_get_error_string(&ftdic));
			return EXIT_FAILURE;
		}
		printf("Manufacturer: %s, Description: %s\n\n", manufacturer, description);
		curdev = curdev->next;
	}

	return EXIT_SUCCESS;
}

void toggle_reset(struct ftdi_context *ftdic) 
{
	uint8_t  buf[3], size;
	int ret;

	printf("toggle reset\n");
	
        /* using MPSSE since it give access to high GPIO*/
	/* set as inputs for now */
	ftdi_set_bitmode(ftdic, 0 , BITMODE_MPSSE); 

	size = 3;
	
	/* RESET RELEASE */

	/* initialize low byte */
	/* command "set data bits low byte" */
	buf[0] = 0x80;
	buf[1] = ((reset_release(BOARD) | vref2_normal(BOARD)) & 0xff);
	buf[2] = dir(BOARD) & 0xff;
	fprintf(stderr,"write %x %x %x\n",buf[0],buf[1],buf[2]);


	if ((ret = (ftdi_write_data(ftdic, buf, size))) < 0)
	{
		perror("ft2232_write error");
		fprintf(stderr, "ft2232_write command %x\n", buf[0]);
		return;
	}


	/* command "set data bits high byte" */
	buf[0] = 0x82;         
	buf[1] = ((reset_release(BOARD) | vref2_normal(BOARD)) >> 8);
	buf[2] = dir(BOARD) >> 8;
	fprintf(stderr,"write %x %x %x\n",buf[0],buf[1],buf[2]);

	if ((ret = (ftdi_write_data(ftdic, buf, size))) < 0)
	{
		perror("ft2232_write error");
		fprintf(stderr, "ft2232_write command %x\n", buf[0]);
		return;
	}

	/* RESET HOLD */

	/* initialize low byte */
	/* command "set data bits low byte" */
	buf[0] = 0x80;
	buf[1] = ((reset_set(BOARD) | vref2_normal(BOARD)) & 0xff);
	buf[2] = dir(BOARD) & 0xff;
	fprintf(stderr,"write %x %x %x\n",buf[0],buf[1],buf[2]);

	if ((ret = (ftdi_write_data(ftdic, buf, size))) < 0)
	{
		perror("ft2232_write error");
		fprintf(stderr, "ft2232_write command %x\n", buf[0]);
		return;
	}

 	/* command "set data bits high byte" */
	buf[0] = 0x82;         
	buf[1] = ((reset_set(BOARD) | vref2_normal(BOARD)) >> 8);
	buf[2] = dir(BOARD) >> 8;
	fprintf(stderr,"write %x %x %x\n",buf[0],buf[1],buf[2]);

	if ((ret = (ftdi_write_data(ftdic, buf, size))) < 0)
	{
		perror("ft2232_write error");
		fprintf(stderr, "ft2232_write command %x\n", buf[0]);
		return;
	}

	/* RESET RELEASE */

	/* initialize low byte */
	/* command "set data bits low byte" */
	buf[0] = 0x80;
	buf[1] = ((reset_release(BOARD) | vref2_normal(BOARD)) & 0xff);
	buf[2] = dir(BOARD) & 0xff;
	fprintf(stderr,"write %x %x %x\n",buf[0],buf[1],buf[2]);

	if ((ret = (ftdi_write_data(ftdic, buf, size))) < 0)
	{
		perror("ft2232_write error");
		fprintf(stderr, "ft2232_write command %x\n", buf[0]);
		return;
	}

	/* command "set data bits high byte" */
	buf[0] = 0x82;         
	buf[1] = ((reset_release(BOARD) | vref2_normal(BOARD)) >> 8);
	buf[2] = dir(BOARD) >> 8;
	fprintf(stderr,"write %x %x %x\n",buf[0],buf[1],buf[2]);

	if ((ret = (ftdi_write_data(ftdic, buf, size))) < 0)
	{
		perror("ft2232_write error");
		fprintf(stderr, "ft2232_write command %x\n", buf[0]);
		return;
	}

	return;

}

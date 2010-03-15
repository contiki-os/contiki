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

struct layout {
	char *name;
	enum ftdi_interface interface;
	uint16_t dir;
	uint16_t reset_release;
	uint16_t reset_set;
	uint16_t vref2_normal;
	uint16_t vref2_erase;
};

#define std_layout(x)                        \
	.interface = interface(x),           \
        .dir = dir(x),	                     \
	.reset_release = reset_release(x),   \
	.reset_set = reset_set(x),	     \
	.vref2_normal = vref2_normal(x),     \
	.vref2_erase = vref2_erase(x),     
	
static const struct layout layouts[] =
{
	{ .name = "redbee_econotag",
	  std_layout(REDBEE_ECONOTAG)
	},
	{ .name = "redbee_usb",
	  std_layout(REDBEE_USB)
	},
	{ .name = NULL, /* end of table */ },
};		
		

/* fgets input buffer length: for prompts and such */
#define BUF_LEN 32

int print_and_prompt( struct ftdi_device_list *devlist );
int bb_mpsee(struct ftdi_context *ftdic, uint16_t dir, uint16_t val); 
void toggle_reset(struct ftdi_context *ftdic, const struct layout * l);
void usage(void);

const struct layout * find_layout(char * str) 
{
	uint32_t i = 0;
	
	while(layouts[i].name != NULL) {
		if(strcmp(layouts[i].name, str) == 0) { return &layouts[i]; }
		i++;
	}

	return NULL;
}

static uint32_t vendid = 0x0403; uint32_t prodid = 0x6010;

int main(int argc, char **argv) 
{
	struct ftdi_context ftdic;
	struct ftdi_device_list *devlist;
	int dev_index = -1; int num_devs;
	char layout_str[BUF_LEN];
	const struct layout *layout;
	int ret;

	while (1) {
		int c;
		int option_index = 0;
		static struct option long_options[] = {
			{"layout", required_argument, 0, 'l'},
			{"index",  required_argument, 0, 'i'},
			{"help",         no_argument, 0, '?'},
			{0, 0, 0, 0}
		};
		
		c = getopt_long (argc, argv, "i:l:",
				 long_options, &option_index);
		if (c == -1)
			break;
		
		switch (c) {
			/* process long opts */
		case 'l':
			strncpy(layout_str, optarg, BUF_LEN);
			break;
		case 'i':
			dev_index = atoi(optarg);
			break;
		default:
			usage();
			break;
		}    
	}

	if( !(layout = find_layout(layout_str))) { 
		usage(); 
		printf("You must specify a layout\n");
		return EXIT_FAILURE;
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

	if (ftdi_set_interface(&ftdic, layout->interface) < 0) {
		fprintf(stderr, "couldn't set interface %d\n", layout->interface);
		return EXIT_FAILURE;
	}

	printf("Found %d devices with vendor id 0x%04x product id 0x%04x\n", 
	       num_devs, vendid, prodid);
	
	if(num_devs == 0) { return EXIT_SUCCESS; }

	if(num_devs == 1) { dev_index = 0; }
	while( (dev_index < 0) || (dev_index >= num_devs)){
		dev_index = print_and_prompt(devlist);
	}
	
	printf("Opening device %d using layout %s\n", dev_index, layout->name);

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
	toggle_reset(&ftdic, layout);
	
	ftdi_list_free(&devlist);
	ftdi_deinit(&ftdic);
	
	return EXIT_SUCCESS;
}

void usage(void) 
{
	printf("Usage: don't know yet\n");	
}

int print_and_prompt( struct ftdi_device_list *devlist ) 
{
	int i, ret;
	struct ftdi_context ftdic;
	struct ftdi_device_list *curdev;
	char manufacturer[128], description[128], serial[128];
	char input[BUF_LEN]; char *s;
	int sel = -1;

	printf("\n");

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

void toggle_reset(struct ftdi_context *ftdic, const struct layout * l) 
{
	printf("toggle reset\n");
	
        /* using MPSSE since it give access to high GPIO*/
	/* set as inputs for now */
	ftdi_set_bitmode(ftdic, 0 , BITMODE_MPSSE); 

	bb_mpsee(ftdic, l->dir, (l->reset_release | l->vref2_normal));
	bb_mpsee(ftdic, l->dir, (l->reset_set     | l->vref2_normal));
	bb_mpsee(ftdic, l->dir, (l->reset_release | l->vref2_normal));

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

/* find_all.c

   Example for ftdi_usb_find_all()

   This program is distributed under the GPL, version 2
*/

#include <stdio.h>
#include <stdlib.h>
#include <ftdi.h>
#include <string.h>

int main(void)
{
    int ret, i;
    struct ftdi_context ftdic;
    struct ftdi_device_list *devlist, *curdev;
    char manufacturer[128], description[128], serial[128];

    if (ftdi_init(&ftdic) < 0)
    {
        fprintf(stderr, "ftdi_init failed\n");
        return EXIT_FAILURE;
    }

    if ((ret = ftdi_usb_find_all(&ftdic, &devlist, 0x0403, 0x6010)) < 0)
    {
        fprintf(stderr, "ftdi_usb_find_all failed: %d (%s)\n", ret, ftdi_get_error_string(&ftdic));
        return EXIT_FAILURE;
    }

    printf("Number of FTDI devices found: %d\n", ret);

    i = 0;
    for (curdev = devlist; curdev != NULL; i++)
    {
        printf("Checking device: %d\n", i);
        if ((ret = ftdi_usb_get_strings(&ftdic, curdev->dev, manufacturer, 128, description, 128, serial, 128)) < 0)
        {
            fprintf(stderr, "ftdi_usb_get_strings failed: %d (%s)\n", ret, ftdi_get_error_string(&ftdic));
            return EXIT_FAILURE;
        }
        printf("Manufacturer: %s, Description: %s, Serial: %s\n\n", manufacturer, description, serial);
	/*if (strcmp("?", serial) == 0) // No serial set => new programmer
	{
		struct ftdi_context ftdic2;
		if (ftdi_init(&ftdic2) < 0)
		{
		        fprintf(stderr, "ftdi_init failed\n");
        		return EXIT_FAILURE;
    		}
 
		if (ftdi_usb_open_dev(&ftdic2, curdev->dev) < 0)
		{
		        fprintf(stderr, "ftdi open failed\n");
        		return EXIT_FAILURE;
		}
		strcpy(serial, "X2");
		if (ftdi_eeprom_initdefaults(&ftdic2, manufacturer, description, serial) < 0)
		{	
		        fprintf(stderr, "initdefaults failed\n");
        		return EXIT_FAILURE;
		}
		ftdi_usb_close(&ftdic2);
	}*/
        curdev = curdev->next;
    }

    ftdi_list_free(&devlist);
    ftdi_deinit(&ftdic);

    return EXIT_SUCCESS;
}


/****************************************************************************
 *
 * MODULE:             Jennic Module Programmer
 *
 * COMPONENT:          Main file
 *
 * VERSION:            $Name:  $
 *
 * REVISION:           $Revision: 1.2 $
 *
 * DATED:              $Date: 2009/03/02 13:33:44 $
 *
 * STATUS:             $State: Exp $
 *
 * AUTHOR:             Matt Redfearn
 *
 * DESCRIPTION:
 *
 *
 * LAST MODIFIED BY:   $Author: lmitch $
 *                     $Modtime: $
 *
 ****************************************************************************
 *
 * This software is owned by NXP B.V. and/or its supplier and is protected
 * under applicable copyright laws. All rights are reserved. We grant You,
 * and any third parties, a license to use this software solely and
 * exclusively on NXP products [NXP Microcontrollers such as JN5148, JN5142, JN5139]. 
 * You, and any third parties must reproduce the copyright and warranty notice
 * and any other legend of ownership on each copy or partial copy of the 
 * software.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.

 * Copyright NXP B.V. 2012. All rights reserved
 *
 ***************************************************************************/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <errno.h>
#include <stdarg.h>

#include <sys/select.h>
#include <sys/time.h>
#include <unistd.h>

#include "JN51xx_BootLoader.h"
#include "Firmware.h"
#include "uart.h"
#include "ChipID.h"
#include "dbg.h"

#define vDelay(a) usleep(a * 1000)

int iVerbosity = 1;

#ifndef VERSION
#error Version is not defined!
#else
const char *Version = "0.6 (r" VERSION ")";
#endif


const char *cpSerialDevice = NULL;
const char *pcFirmwareFile = NULL;

char *pcMAC_Address = NULL;
uint64_t u64MAC_Address;
uint64_t *pu64MAC_Address = NULL;


int iInitialSpeed=38400;
int iProgramSpeed=1000000;


void print_usage_exit(char *argv[])
{
    fprintf(stderr, "Usage: %s\n", argv[0]);
    fprintf(stderr, "  Arguments:\n");
    fprintf(stderr, "    -s --serial        <serial device> Serial device for 15.4 module, e.g. /dev/tts/1\n");
    fprintf(stderr, "  Options:\n");
    fprintf(stderr, "    -V --verbosity     <verbosity>     Verbosity level. Increses amount of debug information. Default 0.\n");
    fprintf(stderr, "    -I --initialbaud   <rate>          Set initial baud rate\n");
    fprintf(stderr, "    -P --programbaud   <rate>          Set programming baud rate\n");
    fprintf(stderr, "    -f --firmware      <firmware>      Load module flash with the given firmware file.\n");
    fprintf(stderr, "    -v --verify                        Verify image. If specified, verify the image programmedwas loaded correctly.\n");
    fprintf(stderr, "    -m --mac           <MAC Address>   Set MAC address of device. If this is not specified, the address is read from flash.\n");
    exit(EXIT_FAILURE);
}


int main(int argc, char *argv[])
{
    tsChipDetails   sChipDetails;
    int             iUartFd;
    struct termios  sUartOptions;
    
    tsFW_Info sFW_Info;
    int iVerify = 0;

    printf("JennicModuleProgrammer Version: %s\n", Version);
    
    {
        static struct option long_options[] =
        {
            {"help",                    no_argument,        NULL,       'h'},
            {"verbosity",               required_argument,  NULL,       'V'},
            {"initialbaud",             required_argument,  NULL,       'I'},
            {"programbaud",             required_argument,  NULL,       'P'},
            {"serial",                  required_argument,  NULL,       's'},
            {"firmware",                required_argument,  NULL,       'f'},
            {"verify",                  no_argument,        NULL,       'v'},
            {"mac",                     required_argument,  NULL,       'm'},
            { NULL, 0, NULL, 0}
        };
        signed char opt;
        int option_index;
        
        while ((opt = getopt_long(argc, argv, "hs:V:f:vI:P:m:", long_options, &option_index)) != -1) 
        {
            switch (opt) 
            {
                case 0:
                    
                case 'h':
                    print_usage_exit(argv);
                    break;
                case 'V':
                    iVerbosity = atoi(optarg);
                    break;
                case 'v':
                    iVerify = 1;
                    break;
                case 'I':
                {
                    char *pcEnd;
                    errno = 0;
                    iInitialSpeed = strtoul(optarg, &pcEnd, 0);
                    if (errno)
                    {
                        printf("Initial baud rate '%s' cannot be converted to 32 bit integer (%s)\n", optarg, strerror(errno));
                        print_usage_exit(argv);
                    }
                    if (*pcEnd != '\0')
                    {
                        printf("Initial baud rate '%s' contains invalid characters\n", optarg);
                        print_usage_exit(argv);
                    }
                    break;
                }
                case 'P':
                {
                    char *pcEnd;
                    errno = 0;
                    iProgramSpeed = strtoul(optarg, &pcEnd, 0);
                    if (errno)
                    {
                        printf("Program baud rate '%s' cannot be converted to 32 bit integer (%s)\n", optarg, strerror(errno));
                        print_usage_exit(argv);
                    }
                    if (*pcEnd != '\0')
                    {
                        printf("Program baud rate '%s' contains invalid characters\n", optarg);
                        print_usage_exit(argv);
                    }
                    break;
                }
                case 's':
                    cpSerialDevice = optarg;
                    break;
                case 'f':
                    pcFirmwareFile = optarg;
                    break;
                case 'm':
                    pcMAC_Address = optarg;
                    u64MAC_Address = strtoll(pcMAC_Address, (char **) NULL, 16);
                    pu64MAC_Address = &u64MAC_Address;
                    break;

                default: /* '?' */
                    print_usage_exit(argv);
            }
        }
    }

    if ((!cpSerialDevice))
    {
        /* Missing parameters */
        print_usage_exit(argv);
    }
    
    if (UART_eInitialise((char *)cpSerialDevice, iInitialSpeed, &iUartFd, &sUartOptions) != E_STATUS_OK)
    {
        fprintf(stderr, "Error opening serial port\n");
        return -1;
    }

    if (iInitialSpeed != iProgramSpeed)
    {
        if (iVerbosity > 1)
        {
            printf("Setting baudrate: %d\n", iProgramSpeed);
        }

        /* Talking at initial speed - change bootloader to programming speed */
        if (BL_eSetBaudrate(iUartFd, iProgramSpeed) != E_STATUS_OK)
        {
            printf("Error setting baudrate\n");
            return -1;
        }
        /* change local port to programming speed */
        if (UART_eSetBaudRate(iUartFd, &sUartOptions, iProgramSpeed) != E_STATUS_OK)
        {
            printf("Error setting baudrate\n");
            return -1;
        }
    }


    /* Read module details at initial baud rate */
    if (BL_eGetChipDetails(iUartFd, &sChipDetails) != E_STATUS_OK)
    {
        fprintf(stderr, "Error reading module information - check cabling and power\n");
        return -1;
    }

    
    if (iVerbosity > 0)
    {
        const char *pcPartName;
        
        switch (sChipDetails.u32ChipId)
        {
            case (CHIP_ID_JN5148_REV2A):    pcPartName = "JN5148";      break;
            case (CHIP_ID_JN5148_REV2B):    pcPartName = "JN5148";      break;
            case (CHIP_ID_JN5148_REV2C):    pcPartName = "JN5148";      break;
            case (CHIP_ID_JN5148_REV2D):    pcPartName = "JN5148J01";   break;
            case (CHIP_ID_JN5148_REV2E):    pcPartName = "JN5148Z01";   break;
            
            case (CHIP_ID_JN5142_REV1A):    pcPartName = "JN5142";      break;
            case (CHIP_ID_JN5142_REV1B):    pcPartName = "JN5142";      break;
            case (CHIP_ID_JN5142_REV1C):    pcPartName = "JN5142J01";   break;

            case (CHIP_ID_JN5168):          pcPartName = "JN516x";      break;
            
            default:                        pcPartName = "Unknown";     break;
        }
        
        printf("Detected Chip: %s\n", pcPartName);
        
        printf("MAC Address:   %02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X\n", 
                sChipDetails.au8MacAddress[0] & 0xFF, sChipDetails.au8MacAddress[1] & 0xFF, sChipDetails.au8MacAddress[2] & 0xFF, sChipDetails.au8MacAddress[3] & 0xFF, 
                sChipDetails.au8MacAddress[4] & 0xFF, sChipDetails.au8MacAddress[5] & 0xFF, sChipDetails.au8MacAddress[6] & 0xFF, sChipDetails.au8MacAddress[7] & 0xFF);
    }


    if (pcFirmwareFile)
    {
        /* Have file to program */
    
        if (iFW_Open(&sFW_Info, (char *)pcFirmwareFile))
        {
            /* Error with file. FW module has displayed error so just exit. */
            return -1;
        }

        if (BL_eReprogram(iUartFd, &sChipDetails, &sFW_Info) != E_STATUS_OK)
        {
            return -1;
        }
    }
    
    printf("Success\n");
    return 0;
}


void dbg_vPrintfImpl(const char *pcFile, uint32_t u32Line, const char *pcFormat, ...)
{
    va_list argp;
    va_start(argp, pcFormat);
#ifdef DBG_VERBOSE
    printf("%s:%d ", pcFile, u32Line);
#endif /* DBG_VERBOSE */
    vprintf(pcFormat, argp);
    return;
}


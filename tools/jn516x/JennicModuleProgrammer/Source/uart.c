
/****************************************************************************
 *
 * MODULE:             Jennic Module Programmer
 *
 * COMPONENT:          Serial port handling
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
/****************************************************************************/
/***        Include files                                                 ***/
/****************************************************************************/

#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <termios.h>

#include <sys/select.h>
#include <sys/signal.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/ioctl.h>

#include <errno.h>

#include <linux/types.h>

#include "uart.h"
#include "common.h"
#include "dbg.h"

/****************************************************************************/
/***        Macro Definitions                                             ***/
/****************************************************************************/

#ifdef DEBUG_UART
#define TRACE_UART	TRUE
#else
#define TRACE_UART	FALSE
#endif

/****************************************************************************/
/***        Type Definitions                                              ***/
/****************************************************************************/

/****************************************************************************/
/***        Local Function Prototypes                                     ***/
/****************************************************************************/

/****************************************************************************/
/***        Exported Variables                                            ***/
/****************************************************************************/

/****************************************************************************/
/***        Local Variables                                               ***/
/****************************************************************************/

/****************************************************************************/
/***        Exported Functions                                            ***/
/****************************************************************************/
/****************************************************************************
 *
 * NAME: UART_eInitialise
 *
 * DESCRIPTION:
 * Initialise a UART
 *
 * RETURNS:
 * teStatus
 *
 ****************************************************************************/
teStatus UART_eInitialise(char *pcDevice, int iBaudRate, int *piFileDescriptor, struct termios *psOptions)
{
 
    DBG_vPrintf(TRACE_UART, "Using UART device %s\n", pcDevice);
    
    //open the device(com port) to be non-blocking (read will return immediately)
    *piFileDescriptor = open(pcDevice, O_RDWR | O_NOCTTY);// | O_NONBLOCK | O_NDELAY);
    if (*piFileDescriptor < 0)
    {
        DBG_vPrintf(TRACE_UART, "Failed to open device!\n", pcDevice);
        return E_STATUS_FAILED_TO_OPEN_FILE;
    }

    if (tcgetattr(*piFileDescriptor, psOptions) == -1)
    {
        DBG_vPrintf(TRACE_UART, "Failed to get port settings!\n");
        return E_STATUS_ERROR;
    }

    psOptions->c_iflag &= ~(INPCK | ISTRIP | INLCR | IGNCR | ICRNL | IUCLC | IXON | IXANY | IXOFF);
    psOptions->c_iflag = IGNBRK | IGNPAR;
    psOptions->c_oflag &= ~(OPOST | OLCUC | ONLCR | OCRNL | ONOCR | ONLRET);
    psOptions->c_cflag &= ~(CSIZE | CSTOPB | PARENB | CRTSCTS);
    psOptions->c_cflag |= CS8 | CREAD | HUPCL | CLOCAL;
    psOptions->c_lflag &= ~(ISIG | ICANON | ECHO | IEXTEN);

    fcntl(*piFileDescriptor, F_SETFL, O_NONBLOCK);
       
    return UART_eSetBaudRate(*piFileDescriptor, psOptions, iBaudRate);

}
 
 
/****************************************************************************
 *
 * NAME: UART_eClose
 *
 * DESCRIPTION:
 * Close the specified UART
 *
 * RETURNS:
 * teStatus
 *
 ****************************************************************************/
teStatus UART_eClose(int iFileDescriptor)
{

    close(iFileDescriptor);
    
    return E_STATUS_OK;
}


/****************************************************************************
 *
 * NAME: UART_eSetBaudRate
 *
 * DESCRIPTION:
 * Sets the baud rate of the specified UART
 *
 * RETURNS:
 * teStatus
 *
 ****************************************************************************/
teStatus UART_eSetBaudRate(int iFileDescriptor, struct termios *psOptions, int iBaudRate)
{
    int iBaud;
    
    DBG_vPrintf(TRACE_UART, "Changing baud rate to %d\n", iBaudRate);

    switch (iBaudRate)
    {
    
    case 38400:     iBaud = B38400;
    	break;

    case 115200:    iBaud = B115200;
		break;

    case 230400:    iBaud = B230400;
		break;

    case 460800:    iBaud = B460800;
		break;

    case 500000:    iBaud = B500000;
		break;

    case 921600:    iBaud = B921600;
    	break;

    case 1000000:   iBaud = B1000000;
        break;
        
    default:
        DBG_vPrintf(TRACE_UART, "Unsupported baud rate: %d\n", iBaudRate);
        return E_STATUS_BAD_PARAMETER;
    }       
    
    if(tcflush(iFileDescriptor, TCIOFLUSH) == -1)
    {
        DBG_vPrintf(TRACE_UART, "Error flushing uart\n");
        return E_STATUS_ERROR;
    }

    if(cfsetispeed(psOptions, iBaud) == -1)
    {
        DBG_vPrintf(TRACE_UART, "Error setting input speed\n");
        return E_STATUS_ERROR;
    }

    if(cfsetospeed(psOptions, iBaud) == -1)
    {
        DBG_vPrintf(TRACE_UART, "Error setting output speed\n");
        return E_STATUS_ERROR;
    }

    if(tcsetattr(iFileDescriptor, TCSANOW, psOptions) == -1)
    {
        DBG_vPrintf(TRACE_UART, "Error changing port settings\n");
        return E_STATUS_ERROR;
    }
    
    return E_STATUS_OK;
}


/****************************************************************************
 *
 * NAME: UART_eFlush
 *
 * DESCRIPTION:
 * Flush the specified UART
 *
 * RETURNS:
 * teStatus
 *
 ****************************************************************************/
teStatus UART_eFlush(int iFileDescriptor)
{
	uint8_t u8Data;
	int iBytesRead;

    if(tcflush(iFileDescriptor, TCIOFLUSH) != 0)
    {
    	return E_STATUS_ERROR;
    }

    do
    {
        UART_eRead(iFileDescriptor, 100, 1, &u8Data, &iBytesRead);
    } while(iBytesRead > 0);


	return E_STATUS_OK;

}


/****************************************************************************
 *
 * NAME: UART_eRead
 *
 * DESCRIPTION:
 * Reads from the specified UART
 *
 * RETURNS:
 * teStatus
 *
 ****************************************************************************/
teStatus UART_eRead(int iFileDescriptor, int iTimeoutMicroseconds, int iBufferLen, uint8_t *pu8Buffer, int *piBytesRead)
{
    int         		iResult = -1;
    struct timeval      sTimeout;
    fd_set              sDescriptors;
    int                 iMaxFd;    
    char				acErrorString[MAX_ERROR_STRING_LEN] = {0};

    if(pu8Buffer == NULL)
    {
        return E_STATUS_NULL_PARAMETER;
    }
    
    *piBytesRead = 0;
    
    sTimeout.tv_usec = iTimeoutMicroseconds % 1000000;
    sTimeout.tv_sec = iTimeoutMicroseconds / 1000000;
    
    FD_ZERO(&sDescriptors);
    iMaxFd = iFileDescriptor;
    FD_SET(iFileDescriptor, &sDescriptors);
    
    iResult = select(iMaxFd + 1, &sDescriptors, NULL, NULL, &sTimeout);
    if (iResult < 0)
    {
		strerror_r(errno, acErrorString, sizeof(acErrorString));
        DBG_vPrintf(TRACE_UART, "Error reading from UART: %s\n", acErrorString);
        return E_STATUS_ERROR;
    }
    else if (iResult == 0)
    {
        DBG_vPrintf(TRACE_UART, "Timeout reading from UART\n");
    }
    else
    {
        if (FD_ISSET(iFileDescriptor, &sDescriptors))
        {
            iResult = read(iFileDescriptor, (uint8_t*)pu8Buffer, iBufferLen);
            if (iResult > 0)
            {
//                DBG_vPrintf(TRACE_UART, "Got 0x%02x %c (iResult=%d)\n", *pu8Data, *pu8Data, iResult);
//                DBG_vPrintf(TRACE_UART, "[URX%d]", iResult);

                *piBytesRead = iResult;
            }
            else
            {
//                DBG_vPrintf(TRACE_UART, "[URX0]");

                iResult = 0;
            }
        }
        else
        {
            DBG_vPrintf(TRACE_UART, "FD_ISn't set!\n");
        }
    }
    
    return E_STATUS_OK;
    
}
 

/****************************************************************************
 *
 * NAME: UART_eWrite
 *
 * DESCRIPTION:
 * Write to the specified UART
 *
 * RETURNS:
 * teStatus
 *
 ****************************************************************************/
teStatus UART_eWrite(int iFileDescriptor, uint8_t *pu8Data, int iLength)
{

    int iBytesWritten;
    int iTotalBytesWritten = 0;

    do
    {
        iBytesWritten = write(iFileDescriptor, &pu8Data[iTotalBytesWritten], iLength - iTotalBytesWritten);
        if(iBytesWritten < 0)
        {
        	if(errno != EAGAIN)
        	{
                DBG_vPrintf(TRACE_UART, "Could not write to UART!\n");
        		return E_STATUS_ERROR_WRITING;
        	}
        }
        else
        {
        	iTotalBytesWritten += iBytesWritten;
        }
    } while(iTotalBytesWritten < iLength);

    return E_STATUS_OK;
    
}


/****************************************************************************/
/***        Local Functions                                               ***/
/****************************************************************************/

/****************************************************************************/
/***        END OF FILE                                                   ***/
/****************************************************************************/

/****************************************************************************
 *
 * MODULE:             JN51xx_BootLoader
 *
 * COMPONENT:          $RCSfile: JN51xx_BootLoader.c,v $
 *
 * VERSION:            $Name:  $
 *
 * REVISION:           $Revision: 1.2 $
 *
 * DATED:              $Date: 2009/03/02 13:33:44 $
 *
 * STATUS:             $State: Exp $
 *
 * AUTHOR:             Lee Mitchell
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

#include <stdio.h>
#include <endian.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>

#include "uart.h"
#include "JN51xx_BootLoader.h"
#include "ChipID.h"
#include "dbg.h"

/****************************************************************************/
/***        Macro Definitions                                             ***/
/****************************************************************************/

#ifdef DEBUG_BOOTLOADER
#define TRACE_BOOTLOADER	TRUE
#else
#define TRACE_BOOTLOADER	FALSE
#endif


#define BL_MAX_CHUNK_SIZE	248

#define BL_TIMEOUT_1S       1000000
#define BL_TIMEOUT_10S      10000000


/* JN513x / JN514x definitions */
#define JN514X_ROM_ID_ADDR                      0x00000004
#define JN514X_MAC_ADDRESS_LOCATION             0x00000010


/* JN516x definitions */

/* Location of MAC address in Index sector */
#define JN516X_MAC_INDEX_SECTOR_PAGE            5
#define JN516X_MAC_INDEX_SECTOR_WORD            7

/* Location of MAC address in memory map */
#define JN516X_CUSTOMER_MAC_ADDRESS_LOCATION    0x01001570
#define JN516X_MAC_ADDRESS_LOCATION             0x01001580

/* Location of bootloader information in memory map */
#define JN516X_BOOTLOADER_VERSION_ADDRESS       0x00000062
#define JN516X_BOOTLOADER_ENTRY                 0x00000066

/* Location of device configuration in memory map */
#define JN516X_INDEX_SECTOR_DEVICE_CONFIG_ADDR  0x01001500


/****************************************************************************/
/***        Type Definitions                                              ***/
/****************************************************************************/

typedef enum
{
	E_BL_MSG_TYPE_FLASH_ERASE_REQUEST 					= 0x07,
	E_BL_MSG_TYPE_FLASH_ERASE_RESPONSE					= 0x08,
	E_BL_MSG_TYPE_FLASH_PROGRAM_REQUEST					= 0x09,
	E_BL_MSG_TYPE_FLASH_PROGRAM_RESPONSE				= 0x0a,
	E_BL_MSG_TYPE_FLASH_READ_REQUEST					= 0x0b,
	E_BL_MSG_TYPE_FLASH_READ_RESPONSE					= 0x0c,
	E_BL_MSG_TYPE_FLASH_SECTOR_ERASE_REQUEST			= 0x0d,
	E_BL_MSG_TYPE_FLASH_SECTOR_ERASE_RESPONSE			= 0x0e,
	E_BL_MSG_TYPE_FLASH_WRITE_STATUS_REGISTER_REQUEST	= 0x0f,
	E_BL_MSG_TYPE_FLASH_WRITE_STATUS_REGISTER_RESPONSE	= 0x10,
	E_BL_MSG_TYPE_RAM_WRITE_REQUEST						= 0x1d,
	E_BL_MSG_TYPE_RAM_WRITE_RESPONSE					= 0x1e,
	E_BL_MSG_TYPE_RAM_READ_REQUEST						= 0x1f,
	E_BL_MSG_TYPE_RAM_READ_RESPONSE						= 0x20,
	E_BL_MSG_TYPE_RAM_RUN_REQUEST						= 0x21,
	E_BL_MSG_TYPE_RAM_RUN_RESPONSE						= 0x22,
	E_BL_MSG_TYPE_FLASH_READ_ID_REQUEST					= 0x25,
	E_BL_MSG_TYPE_FLASH_READ_ID_RESPONSE				= 0x26,
	E_BL_MSG_TYPE_SET_BAUD_REQUEST						= 0x27,
	E_BL_MSG_TYPE_SET_BAUD_RESPONSE						= 0x28,
	E_BL_MSG_TYPE_FLASH_SELECT_TYPE_REQUEST				= 0x2c,
	E_BL_MSG_TYPE_FLASH_SELECT_TYPE_RESPONSE			= 0x2d,
    
    E_BL_MSG_TYPE_GET_CHIPID_REQUEST                    = 0x32,
    E_BL_MSG_TYPE_GET_CHIPID_RESPONSE                   = 0x33,
} __attribute ((packed)) teBL_MessageType;


typedef enum
{
	E_BL_RESPONSE_OK									= 0x00,
	E_BL_RESPONSE_NOT_SUPPORTED							= 0xff,
	E_BL_RESPONSE_WRITE_FAIL							= 0xfe,
	E_BL_RESPONSE_INVALID_RESPONSE						= 0xfd,
	E_BL_RESPONSE_CRC_ERROR								= 0xfc,
	E_BL_RESPONSE_ASSERT_FAIL							= 0xfb,
	E_BL_RESPONSE_USER_INTERRUPT						= 0xfa,
	E_BL_RESPONSE_READ_FAIL								= 0xf9,
	E_BL_RESPONSE_TST_ERROR								= 0xf8,
	E_BL_RESPONSE_AUTH_ERROR							= 0xf7,
	E_BL_RESPONSE_NO_RESPONSE							= 0xf6,
	E_BL_RESPONSE_ERROR									= 0xf0,
} __attribute__ ((packed)) teBL_Response;


typedef struct
{
	uint16_t				u16FlashId;
	uint8_t					u8FlashType;
	char					*pcFlashName;
} tsBL_FlashDevice;

/****************************************************************************/
/***        Local Function Prototypes                                     ***/
/****************************************************************************/

static int iBL_ReadChipId(int iUartFd, uint32_t *pu32ChipId);
static int iBL_ReadFlashId(int iUartFd, uint16_t *pu16FlashId);
static int iBL_SelectFlashDevice(int iUartFd, uint16_t u16FlashId);
static int iBL_WriteStatusRegister(int iUartFd, uint8_t u8StatusReg);

static int iBL_RunRAM(int iUartFd, uint32_t u32Address);
static int iBL_ReadRAM(int iUartFd, uint32_t u32Address, uint8_t u8Length, uint8_t *pu8Buffer);
static int iBL_WriteRAM(int iUartFd, uint32_t u32Address, uint8_t u8Length, uint8_t *pu8Buffer);

static int iBL_EraseFlash(int iUartFd);
static int iBL_ReadFlash(int iUartFd, uint32_t u32Address, uint8_t u8Length, uint8_t *pu8Buffer);
static int iBL_WriteFlash(int iUartFd, uint32_t u32Address, uint8_t u8Length, uint8_t *pu8Buffer);

static teBL_Response eBL_Request(int iUartFd, int iTimeoutMicroseconds, teBL_MessageType eTxType, uint8_t u8HeaderLen, uint8_t *pu8Header, uint8_t u8TxLength, uint8_t *pu8TxData, teBL_MessageType *peRxType, uint8_t *pu8RxLength, uint8_t *pu8RxData);

static int iBL_WriteMessage(int iUartFd, teBL_MessageType eType, uint8_t u8HeaderLength, uint8_t *pu8Header, uint8_t u8Length, uint8_t *pu8Data);
static teBL_Response eBL_ReadMessage(int iUartFd, int iTimeoutMicroseconds, teBL_MessageType *peType, uint8_t *pu8Length, uint8_t *pu8Data);

/****************************************************************************/
/***        Exported Variables                                            ***/
/****************************************************************************/

extern int iVerbosity;

/****************************************************************************/
/***        Local Variables                                               ***/
/****************************************************************************/

tsBL_FlashDevice asFlashDevices[] = {
		{
				.u16FlashId = 0x0505,
				.u8FlashType = 4,
				.pcFlashName = "ST M25P05-A"
		},

		{
				.u16FlashId = 0x1010,
				.u8FlashType = 0,
				.pcFlashName = "ST M25P10-A"
		},

		{
				.u16FlashId = 0x1111,
				.u8FlashType = 5,
				.pcFlashName = "ST M25P20-A"
		},

		{
				.u16FlashId = 0x1212,
				.u8FlashType = 3,
				.pcFlashName = "ST M25P40"
		},

		{
				.u16FlashId = 0xbf49,
				.u8FlashType = 1,
				.pcFlashName = "SST 25VF010A"
		},

		{
				.u16FlashId = 0x1f60,
				.u8FlashType = 2,
				.pcFlashName = "Atmel 25F512"
		},

		{
				.u16FlashId = 0xccee,
				.u8FlashType = 8,
				.pcFlashName = "JN516x"
		},

};


/****************************************************************************/
/***        Exported Functions                                            ***/
/****************************************************************************/
/****************************************************************************
 *
 * NAME: BL_eGetChipId
 *
 * DESCRIPTION:
 *	Gets the ChipID
 *
 * PARAMETERS: 	Name        	RW  Usage
 * 				iUartFd		    R	Uart file descriptor
 * 				psChipDetails   W   Pointer to chip details structure
 *
 * RETURNS:
 * teStatus			E_STATUS_OK if success
 *
 ****************************************************************************/
teStatus BL_eGetChipId(int iUartFd, tsChipDetails *psChipDetails)
{
    uint8_t au8Buffer[6];

    DBG_vPrintf(TRACE_BOOTLOADER, "Get Chip ID\n");
    
    if(psChipDetails == NULL)
    {
        return E_STATUS_NULL_PARAMETER;
    }

    /* Send chip id request */
    if(iBL_ReadChipId(iUartFd, &psChipDetails->u32ChipId) < 0)
    {
        DBG_vPrintf(TRACE_BOOTLOADER, "Error reading chip id\n");

        /* That failed so it might be an old device that doesn't support the command, try reading it directly */
        if (iBL_ReadRAM(iUartFd, 0x100000FC, 4, au8Buffer) < 0)
        {
            DBG_vPrintf(TRACE_BOOTLOADER, "Error Reading processor ID register\n");
            return E_STATUS_ERROR;
        }
        else
        {
            psChipDetails->u32ChipId  = au8Buffer[0] << 24;
            psChipDetails->u32ChipId |= au8Buffer[1] << 16;
            psChipDetails->u32ChipId |= au8Buffer[2] << 8;
            psChipDetails->u32ChipId |= au8Buffer[3] << 0;
        }
    }
    
    if (CHIP_ID_PART(psChipDetails->u32ChipId) == CHIP_ID_PART(CHIP_ID_JN5168))
    {
        DBG_vPrintf(TRACE_BOOTLOADER, "Reading 6x data\n");

        if (iBL_ReadRAM(iUartFd, JN516X_BOOTLOADER_VERSION_ADDRESS, 4, au8Buffer) < 0)
        {
            DBG_vPrintf(TRACE_BOOTLOADER, "Error Reading bootloader version\n");
            return E_STATUS_ERROR;
        }
        else
        {
            psChipDetails->u32BootloaderVersion  = au8Buffer[0] << 24;
            psChipDetails->u32BootloaderVersion |= au8Buffer[1] << 16;
            psChipDetails->u32BootloaderVersion |= au8Buffer[2] << 8;
            psChipDetails->u32BootloaderVersion |= au8Buffer[3] << 0;
            
            DBG_vPrintf(TRACE_BOOTLOADER, "JN516x Bootloader version 0x%08x\n", psChipDetails->u32BootloaderVersion);
        }

        if (iBL_ReadRAM(iUartFd, JN516X_INDEX_SECTOR_DEVICE_CONFIG_ADDR, 4, au8Buffer) < 0)
        {
            DBG_vPrintf(TRACE_BOOTLOADER, "Error Reading config from flash index sector\n");
            return E_STATUS_ERROR;
        }
        else
        {
            psChipDetails->u32RamSize      = (au8Buffer[3] & 0x07) >> 0;
            psChipDetails->u32FlashSize    = (au8Buffer[3] & 0x30) >> 4;

            psChipDetails->u32SupportedFirmware = (
                (psChipDetails->u32FlashSize << 16) |
                (psChipDetails->u32RamSize   << 24) |
                (0x08));
            
            DBG_vPrintf(TRACE_BOOTLOADER, "JN516x RAM size %dk\n", (psChipDetails->u32RamSize * 8) + 8);
            DBG_vPrintf(TRACE_BOOTLOADER, "JN516x Flash size %dk\n", (psChipDetails->u32FlashSize * 32) + 32);
            DBG_vPrintf(TRACE_BOOTLOADER, "JN516x Bootloader version 0x%08x\n", psChipDetails->u32BootloaderVersion);
            DBG_vPrintf(TRACE_BOOTLOADER, "JN516x Supported firmware 0x%08x\n", psChipDetails->u32SupportedFirmware);
        }
    }
    else
    {
        if (iBL_ReadRAM(iUartFd, JN514X_ROM_ID_ADDR, 4, au8Buffer) < 0)
        {
            DBG_vPrintf(TRACE_BOOTLOADER, "Error Reading ROM ID\n");
            return E_STATUS_ERROR;
        }
        else
        {
            psChipDetails->u32SupportedFirmware  = au8Buffer[0] << 24;
            psChipDetails->u32SupportedFirmware |= au8Buffer[1] << 16;
            psChipDetails->u32SupportedFirmware |= au8Buffer[2] << 8;
            psChipDetails->u32SupportedFirmware |= au8Buffer[3] << 0;
        }
    }
    
    return E_STATUS_OK;
}


/****************************************************************************
 *
 * NAME: BL_eGetMacAddress
 *
 * DESCRIPTION:
 * Reads the MAC address
 *
 * PARAMETERS: 	Name        	RW  Usage
 * 				iUartFd		    R	Uart file descriptor
 * 				psChipDetails   W   Pointer to chip details structure
 *
 * RETURNS:
 * teStatus         E_STATUS_OK if success
 *
 ****************************************************************************/
teStatus BL_eGetMacAddress(int iUartFd, tsChipDetails *psChipDetails)
{

    int iRetval;
    uint8_t au8InvalidMac[8] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff};

    if(psChipDetails == NULL)
    {
        return E_STATUS_NULL_PARAMETER;
    }

    switch(CHIP_ID_PART(psChipDetails->u32ChipId))
    {
        case CHIP_ID_PART(CHIP_ID_JN5148_REV2A):
            iRetval = iBL_ReadFlash(iUartFd, JN514X_MAC_ADDRESS_LOCATION, 8, psChipDetails->au8MacAddress);
            break;

        case CHIP_ID_PART(CHIP_ID_JN5168):
            /* First we read the customer specific MAC address, and if its not all F's, we use that */
            iRetval = iBL_ReadRAM(iUartFd, JN516X_CUSTOMER_MAC_ADDRESS_LOCATION, 8, psChipDetails->au8MacAddress);

            /* If its all F's, read factory assigned MAC */
            if(memcmp(psChipDetails->au8MacAddress, au8InvalidMac, 8) == 0)
            {
                DBG_vPrintf(TRACE_BOOTLOADER, "No customer MAC address - using factory\n");
                iRetval = iBL_ReadRAM(iUartFd, JN516X_MAC_ADDRESS_LOCATION, 8, psChipDetails->au8MacAddress);
            }
            break;

        default:
            return E_STATUS_ERROR;

    }

    return (iRetval < 0) ? E_STATUS_ERROR : E_STATUS_OK;
}


/****************************************************************************
 *
 * NAME: BL_eGetChipDetails
 *
 * DESCRIPTION:
 *  Get details of the device
 *
 * PARAMETERS:  Name            RW  Usage
 *              iUartFd         R   Uart file descriptor
 *              psChipDetails   W   Pointer to chip details structure
 *
 * RETURNS:
 * int          E_STATUS_OK if success
 *
 ****************************************************************************/
teStatus BL_eGetChipDetails(int iUartFd, tsChipDetails *psChipDetails)
{
    teStatus eStatus;
    
    eStatus = BL_eGetChipId(iUartFd, psChipDetails);
    if (eStatus != E_STATUS_OK)
    {
        return eStatus;
    }
    
    eStatus = BL_eGetMacAddress(iUartFd, psChipDetails);
    if (eStatus != E_STATUS_OK)
    {
        return eStatus;
    }
    return E_STATUS_OK;
}


/****************************************************************************
 *
 * NAME: BL_iReprogram
 *
 * DESCRIPTION:
 *	Reprograms the device
 *
 * PARAMETERS: 	Name        	RW  Usage
 * 				pu8Firmware		R	Pointer to firmware image to download
 * 				pu64Address     R   Pointer to MAC Address. If NULL, read from flash.
 *
 * RETURNS:
 * int			0 if success
 * 				-1 if an error occurred
 *
 ****************************************************************************/
teStatus BL_eReprogram(int iUartFd, tsChipDetails *psChipDetails, tsFW_Info *psFWImage)
{

    int n;
    uint16_t u16FlashId;
    uint8_t u8ChunkSize;
    uint8_t au8Buffer1[BL_MAX_CHUNK_SIZE + 1];
    uint8_t au8Buffer2[BL_MAX_CHUNK_SIZE + 1];

    if (memcmp(&psFWImage->u32ROMVersion, &psChipDetails->u32SupportedFirmware, 4) != 0)
    {
        printf("Incompatible firmware (Built for 0x%08x, Device is 0x%08x)\n", psFWImage->u32ROMVersion, psChipDetails->u32SupportedFirmware);
        return E_STATUS_INCOMPATIBLE;
    }

    /* First, depending on chip type, we may need to copy the MAC address into the firmware image
        * Also, some images have a 4 byte header in the binary that needs stripping off, so do that here
        * by adjusting start point and length to suit */
    switch(CHIP_ID_PART(psChipDetails->u32ChipId))
    {

        case CHIP_ID_PART(CHIP_ID_JN5148_REV2A):
            memcpy(&psFWImage->pu8ImageData[JN514X_MAC_ADDRESS_LOCATION], psChipDetails->au8MacAddress, 8);
            break;

        case CHIP_ID_PART(CHIP_ID_JN5168):
            break;

        default:
            printf("Unsupported chip type 0x%08x\n", psChipDetails->u32ChipId);
            return E_STATUS_ERROR;
            break;
    }
#if 0
    /* Dump the file contents in hex */
    DBG_vPrintf(TRACE_BOOTLOADER, "Dumping %d bytes\n", psFWImage->u32ImageLength);
    for(n = 0; n < psFWImage->u32ImageLength; n++)
    {

        if((n % 16) == 0)
        {
            DBG_vPrintf(TRACE_BOOTLOADER, "\n%08x: ", n);
        }

        DBG_vPrintf(TRACE_BOOTLOADER, " %02x", psFWImage->pu8ImageData[n]);

    }
    DBG_vPrintf(TRACE_BOOTLOADER, "\n");
#endif


    /* Get the flash Id */
    if(iBL_ReadFlashId(iUartFd, &u16FlashId) == -1)
    {
        DBG_vPrintf(TRACE_BOOTLOADER, "Error getting flash id...\n");
        return E_STATUS_ERROR;
    }
    DBG_vPrintf(TRACE_BOOTLOADER, "FlashId: %04x\n", u16FlashId);

    /* Set the flash type */
    if(iBL_SelectFlashDevice(iUartFd, u16FlashId) == -1)
    {
        DBG_vPrintf(TRACE_BOOTLOADER, "Error setting flash type...\n");
        return E_STATUS_ERROR;
    }

    /* If its not internal flash, we need to enable write access */
    if(u16FlashId != FLASH_INTERNAL)
    {
        if(iBL_WriteStatusRegister(iUartFd, 0x00) == -1)
        {
            DBG_vPrintf(TRACE_BOOTLOADER, "Error writing to flash status register...\n");
            return E_STATUS_ERROR;
        }
    }

    /* Erase the flash memory */
    if (iVerbosity > 0)
    {
        printf("Erasing:     0%%\n");
    }
    if(iBL_EraseFlash(iUartFd) == -1)
    {
        DBG_vPrintf(TRACE_BOOTLOADER, "Error erasing flash...\n");
        return E_STATUS_ERROR;
    }

    /* Ensure that flash is erased */
    DBG_vPrintf(TRACE_BOOTLOADER, "Checking flash is blank...\n");
    memset(au8Buffer2, 0xFF, 64);

    if (iBL_ReadFlash(iUartFd, 0, 64, au8Buffer1) == -1)
    {
        DBG_vPrintf(TRACE_BOOTLOADER, "Error reading Flash at address 0x%08x\n", 0);
        return E_STATUS_ERROR;
    }
    else
    {
        if (memcmp(au8Buffer1, au8Buffer2, 64))
        {
            printf("Failed to erase Flash: not blank!\n");
#if 0
            {
                /* Dump the contents in hex */
                DBG_vPrintf(TRACE_BOOTLOADER, "Dumping %d bytes\n", 64);
                for(n = 0; n < 64; n++)
                {

                    if((n % 16) == 0)
                    {
                        DBG_vPrintf(TRACE_BOOTLOADER, "\n%08x: ", n);
                    }

                    DBG_vPrintf(TRACE_BOOTLOADER, " %02x", au8Buffer1[n]);

                }
                DBG_vPrintf(TRACE_BOOTLOADER, "\n");

            }
#endif

            return E_STATUS_ERROR;
        }
        else
        {
            DBG_vPrintf(TRACE_BOOTLOADER, "Flash erase success\n");
            
            if (iVerbosity > 0)
            {
                printf("%c[AErasing:   100%%\n", 0x1B);
            }
        }
    }

    if (iVerbosity > 0)
    {
        printf("Writing Program to Flash\n\n");
    }

    for(n = 0; n < psFWImage->u32ImageLength;)
    {
        if((psFWImage->u32ImageLength - n) > 128)
        {
            u8ChunkSize = 128;
        }
        else
        {
            u8ChunkSize = psFWImage->u32ImageLength - n;
        }

        if(iBL_WriteFlash(iUartFd, n, u8ChunkSize, psFWImage->pu8ImageData + n) == -1)
        {
            printf("Failed to write at address 0x%08x\n", n);
            return E_STATUS_ERROR;
        }

        n += u8ChunkSize;

        if (iVerbosity > 0)
        {
            printf("%c[AWriting:   %3d%%\n", 0x1B, (n * 100) / psFWImage->u32ImageLength);
        }
    }

    if (iVerbosity > 0)
    {
        printf("Verifying Program in Flash\n\n");
    }

    for(n = 0; n < psFWImage->u32ImageLength;)
    {
        if((psFWImage->u32ImageLength - n) > 128)
        {
            u8ChunkSize = 128;
        }
        else
        {
            u8ChunkSize = psFWImage->u32ImageLength - n;
        }

        if (iBL_ReadFlash(iUartFd, n, u8ChunkSize, au8Buffer1) == -1)
        {
            printf("Error reading at address 0x%08x\n", n);
            return E_STATUS_ERROR;
        }
        else
        {
            if (memcmp(psFWImage->pu8ImageData + n, au8Buffer1, u8ChunkSize))
            {
                printf("Verify failed at address 0x%08x!\n", n);
                return E_STATUS_ERROR;
            }
        }

        n += u8ChunkSize;

        if (iVerbosity > 0)
        {
            printf("%c[AVerifying: %3d%%\n", 0x1B, (n * 100) / psFWImage->u32ImageLength);
        }
    }

    return E_STATUS_OK;
}


teStatus BL_eSetBaudrate(int iUartFd, uint32_t u32Baudrate)
{
    teBL_Response eResponse = 0;
    teBL_MessageType eRxType = 0;
    uint8_t u8RxDataLen = 0;
    uint8_t au8Buffer[6];
    uint32_t u32Divisor;
    
    // Divide 1MHz clock bu baudrate to get the divisor
    u32Divisor = 1000000 / u32Baudrate;
    
    au8Buffer[0] = (uint8_t)u32Divisor;
    au8Buffer[1] = 0;
    au8Buffer[2] = 0;
    au8Buffer[3] = 0;
    au8Buffer[4] = 0;

    eResponse = eBL_Request(iUartFd, BL_TIMEOUT_1S, E_BL_MSG_TYPE_SET_BAUD_REQUEST, 1, au8Buffer, 0, NULL, &eRxType, &u8RxDataLen, au8Buffer);
    if(eResponse != E_BL_RESPONSE_OK || eRxType != E_BL_MSG_TYPE_SET_BAUD_RESPONSE)
    {
        DBG_vPrintf(TRACE_BOOTLOADER, "%s: Response %02x\n", __FUNCTION__, eResponse);
        return E_STATUS_ERROR;
    }
    return E_STATUS_OK;
}



#if 0
/****************************************************************************
 *
 * NAME: iBL_DownloadFirmwareToRam
 *
 * DESCRIPTION:
 *	Downloads the given firmware to the module's RAM.
 *	see \ZED-TL\Sw\Source\boot.c for details
 *
 * PARAMETERS: 	Name        	RW  Usage
 * 				pu8Firmware		R	Pointer to firmware image to download
 * 				pu64Address     R   Pointer to MAC Address. If NULL, read from flash.
 *
 * RETURNS:
 * int			0 if success
 * 				-1 if an error occurred
 *
 ****************************************************************************/
int iBL_DownloadFirmwareToRam(tsFW_Info *psFW_Info, uint64_t *pu64MAC_Address)
{
	int n;
	uint8_t u8ChunkSize;

// 	if(psBinHeader->u8ConfigByte0 == 0xff && psBinHeader->u8ConfigByte1 == 0xff && psBinHeader->u16SpiScrambleIndex == 0xffff)
// 	{
// 		DBG_vPrintf(TRACE_BOOTLOADER, "\nFlash image is invalid, aborting.");
// 		return(-1);
// 	}

	DBG_vPrintf(TRACE_BOOTLOADER, "\nText Start 0x%08x - Len %d bytes", psFW_Info->u32TextSectionLoadAddress, psFW_Info->u32TextSectionLength);
	DBG_vPrintf(TRACE_BOOTLOADER, "\nBSS  Start 0x%08x - Len %d bytes", psFW_Info->u32BssSectionLoadAddress, psFW_Info->u32BssSectionLength);
	DBG_vPrintf(TRACE_BOOTLOADER, "\nReset entry point 0x%08x", ntohl(psFW_Info->u32ResetEntryPoint));
    DBG_vPrintf(TRACE_BOOTLOADER, "\nWake entry point 0x%08x",  ntohl(psFW_Info->u32WakeUpEntryPoint));
    
    printf("\nHeader is at %p\n", psFW_Info->pu8ImageData);
    printf("Text Start  at %p\n", psFW_Info->pu8TextData);
    
#if 0
    if (pu64MAC_Address)
    {
        uint64_t u64Temp;
        u64Temp = htobe64(*pu64MAC_Address);
        memset(au8Buffer, 0, 12);
        memcpy(au8Buffer, &u64Temp, sizeof(uint64_t));
    }
    else
    {
        DBG_vPrintf(TRACE_BOOTLOADER, "Reading module MAC address from flash.\n");
        /* Read out the module's MAC address & license key, we need to write them back to RAM */
        if(iBL_ReadFlash(MAC_ADDR_LOCATION, 12, au8Buffer) != 12)
        {
            DBG_vPrintf(TRACE_BOOTLOADER, "\nFailed to read module's MAC address");
            return(-1);
        }
    }
#endif
//     DBG_vPrintf(TRACE_BOOTLOADER, "\nLoading .text\n");
    /* Download text segment */
    for(n = 0; n < psFW_Info->u32TextSectionLength;)
    {
        if((psFW_Info->u32TextSectionLength - n) > BL_MAX_CHUNK_SIZE)
        {
            u8ChunkSize = BL_MAX_CHUNK_SIZE;
        }
        else
        {
            u8ChunkSize = psFW_Info->u32TextSectionLength - n;
        }
        
        if(iBL_WriteRAM(psFW_Info->u32TextSectionLoadAddress + n, u8ChunkSize, psFW_Info->pu8TextData + n) == -1)
        {
            DBG_vPrintf(TRACE_BOOTLOADER, "\nProblem writing chunk");
            return(-1);
        }

        printf("Wrote chunk length %d at address 0x%08x: 0x%08x\n", u8ChunkSize, psFW_Info->u32TextSectionLoadAddress + n, ntohl(*((uint32_t*)(psFW_Info->pu8TextData + n))));
        
        if (1)
        {
            uint8_t au8Buffer[BL_MAX_CHUNK_SIZE + 1];
            if (iBL_ReadRAM(psFW_Info->u32TextSectionLoadAddress + n, u8ChunkSize, au8Buffer) == -1)
            {
                printf("Error reading at address 0x%08x\n", psFW_Info->u32TextSectionLoadAddress + n);
                return -1;
            }
            else
            {
                if (memcmp(psFW_Info->pu8TextData + n, au8Buffer, u8ChunkSize))
                {
                    printf("Data read at address 0x%08x is incorrect\n", psFW_Info->u32TextSectionLoadAddress + n);
                    return -1;
                }
            }
        }
        
        n += u8ChunkSize;
        
        //DBG_vPrintf(TRACE_BOOTLOADER, "Loading: %3d%%\n%c[A", (n * 100) / psFW_Info->u32TextSectionLength, 0x1B);
        DBG_vPrintf(TRACE_BOOTLOADER, "Loading: %3d%%\n", (n * 100) / psFW_Info->u32TextSectionLength);
    }
	
	/* Write back MAC address & license key */
// 	if(iBL_WriteRAM(psFW_Info->u32TextSectionLoadAddress, 12, au8Buffer) == -1)
// 	{
// 		DBG_vPrintf(TRACE_BOOTLOADER, "\nProblem writing chunk");
// 		return(-1);
// 	}
#if 1
	DBG_vPrintf(TRACE_BOOTLOADER, "\nLoading .bss     \n");

    /* Download BSS segment */
    {
        uint8_t au8Buffer[127];
        memset(au8Buffer, 0, sizeof(au8Buffer));
        for(n = 0; n < psFW_Info->u32BssSectionLength;)
        {
            if((psFW_Info->u32BssSectionLength - n) > sizeof(au8Buffer))
            {
                u8ChunkSize = sizeof(au8Buffer);
            }
            else
            {
                u8ChunkSize = psFW_Info->u32BssSectionLength - n;
            }
            if(iBL_WriteRAM(psFW_Info->u32BssSectionLoadAddress + n, u8ChunkSize, au8Buffer) == -1)
            {
                DBG_vPrintf(TRACE_BOOTLOADER, "\nProblem writing BSS");
                return(-1);
            }

            n += u8ChunkSize;

            DBG_vPrintf(TRACE_BOOTLOADER, "Loading: %3d%%\n%c[A", (n * 100) / psFW_Info->u32BssSectionLength, 0x1B);
        }
    }
#endif

    DBG_vPrintf(TRACE_BOOTLOADER, "\nStarting module application");
    iBL_RunRAM(psFW_Info->u32ResetEntryPoint);

	return(0);
}
#endif

/****************************************************************************
 *
 * NAME: iBL_ReadChipId
 *
 * DESCRIPTION:
 *
 * PARAMETERS: Name        RW  Usage
 *
 * RETURNS:
 * int			0 if success
 * 				-1 if an error occurred
 *
 ****************************************************************************/
static int iBL_ReadChipId(int iUartFd, uint32_t *pu32ChipId)
{

    teBL_Response eResponse = 0;
    teBL_MessageType eRxType = 0;
    uint8_t u8RxDataLen = 0;
    uint8_t au8Buffer[6];

	if(pu32ChipId == NULL)
	{
		return(-1);
	}

    eResponse = eBL_Request(iUartFd, BL_TIMEOUT_1S, E_BL_MSG_TYPE_GET_CHIPID_REQUEST, 0, NULL, 0, NULL, &eRxType, &u8RxDataLen, au8Buffer);
    if(eResponse != E_BL_RESPONSE_OK || eRxType != E_BL_MSG_TYPE_GET_CHIPID_RESPONSE)
    {
    	DBG_vPrintf(TRACE_BOOTLOADER, "%s: Response %02x\n", __FUNCTION__, eResponse);
		return(-1);
    }

	*pu32ChipId  = au8Buffer[0] << 24;
	*pu32ChipId |= au8Buffer[1] << 16;
	*pu32ChipId |= au8Buffer[2] << 8;
	*pu32ChipId |= au8Buffer[3] << 0;

	return(0);
}


/****************************************************************************
 *
 * NAME: iBL_ReadFlashId
 *
 * DESCRIPTION:
 *
 * PARAMETERS: Name        RW  Usage
 *
 * RETURNS:
 * int			0 if success
 * 				-1 if an error occurred
 *
 ****************************************************************************/
static int iBL_ReadFlashId(int iUartFd, uint16_t *pu16FlashId)
{

    teBL_Response eResponse = 0;
    teBL_MessageType eRxType = 0;
    uint8_t u8RxDataLen = 0;
    uint8_t au8Buffer[6];

	if(pu16FlashId == NULL)
	{
		return(-1);
	}

	*pu16FlashId = 0x0000;

    eResponse = eBL_Request(iUartFd, BL_TIMEOUT_1S, E_BL_MSG_TYPE_FLASH_READ_ID_REQUEST, 0, NULL, 0, NULL, &eRxType, &u8RxDataLen, au8Buffer);
    if(eResponse != E_BL_RESPONSE_OK || eRxType != E_BL_MSG_TYPE_FLASH_READ_ID_RESPONSE)
    {
        /* Failed to read Flash ID */
    	DBG_vPrintf(TRACE_BOOTLOADER, "Could not read Flash type!\n");
        return -1;
    }

    *pu16FlashId  = au8Buffer[0] << 8;
    *pu16FlashId |= au8Buffer[1] << 0;

    return 0;
}


/****************************************************************************
 *
 * NAME: iBL_ReadFlashId
 *
 * DESCRIPTION:
 *
 * PARAMETERS: Name        RW  Usage
 *
 * RETURNS:
 * int			0 if success
 * 				-1 if an error occurred
 *
 ****************************************************************************/
static int iBL_SelectFlashDevice(int iUartFd, uint16_t u16FlashId)
{
	int n;
    teBL_Response eResponse = 0;
    teBL_MessageType eRxType = 0;
    uint8_t u8RxDataLen = 0;
    uint8_t au8Buffer[6];

    /* Search for flash type given flash device id code */
    for(n = 0; n < sizeof(asFlashDevices) / sizeof(tsBL_FlashDevice); n++)
    {
    	/* If we found a match, send command to select this flash device type */
    	if(asFlashDevices[n].u16FlashId == u16FlashId)
    	{

	    	DBG_vPrintf(TRACE_BOOTLOADER, "Flash type is %s\n", asFlashDevices[n].pcFlashName);

    		au8Buffer[0] = asFlashDevices[n].u8FlashType;
    	    au8Buffer[1] = 0;
    	    au8Buffer[2] = 0;
    	    au8Buffer[3] = 0;
    	    au8Buffer[4] = 0;

    	    eResponse = eBL_Request(iUartFd, BL_TIMEOUT_1S, E_BL_MSG_TYPE_FLASH_SELECT_TYPE_REQUEST, 5, au8Buffer, 0, NULL, &eRxType, &u8RxDataLen, au8Buffer);
    	    if(eResponse != E_BL_RESPONSE_OK || eRxType != E_BL_MSG_TYPE_FLASH_SELECT_TYPE_RESPONSE)
    	    {
    	        /* Failed to select flash type */
    	    	DBG_vPrintf(TRACE_BOOTLOADER, "Could not select Flash type!\n");
    	        return -1;
    	    }

    		return 0;

    	}
    }

    /* Flash type not found */
    return -1;

}


/****************************************************************************
 *
 * NAME: iBL_WriteStatusRegister
 *
 * DESCRIPTION:
 *
 * PARAMETERS: Name        RW  Usage
 *
 * RETURNS:
 * int			0 if success
 * 				-1 if an error occurred
 *
 ****************************************************************************/
static int iBL_WriteStatusRegister(int iUartFd, uint8_t u8StatusReg)
{
    teBL_Response eResponse = 0;
    teBL_MessageType eRxType = 0;
    uint8_t u8RxDataLen = 0;
    uint8_t au8Buffer[6];

    DBG_vPrintf(TRACE_BOOTLOADER, "Writing %02x to flash status register\n", u8StatusReg);

    eResponse = eBL_Request(iUartFd, BL_TIMEOUT_1S, E_BL_MSG_TYPE_FLASH_WRITE_STATUS_REGISTER_REQUEST, 1, &u8StatusReg, 0, NULL, &eRxType, &u8RxDataLen, au8Buffer);
    if(eResponse != E_BL_RESPONSE_OK || eRxType != E_BL_MSG_TYPE_FLASH_WRITE_STATUS_REGISTER_RESPONSE)
    {
        /* Failed to write to flash status register */
        DBG_vPrintf(TRACE_BOOTLOADER, "Could not write to status register!\n");
        return -1;
    }

    return 0;
}


/****************************************************************************
 *
 * NAME: iBL_RunRAM
 *
 * DESCRIPTION:
 *	Starts the module executing code from a given address
 *
 * PARAMETERS: Name        RW  Usage
 *
 * RETURNS:
 * int			0 if success
 * 				-1 if an error occurred
 *
 ****************************************************************************/
static int iBL_RunRAM(int iUartFd, uint32_t u32Address)
{
	uint8_t u8RxDataLen = 0;
	uint8_t au8CmdBuffer[4];
	teBL_Response eResponse = 0;
	teBL_MessageType eRxType = 0;

	au8CmdBuffer[0] = (uint8_t)(u32Address >> 0) & 0xff;
	au8CmdBuffer[1] = (uint8_t)(u32Address >> 8) & 0xff;
	au8CmdBuffer[2] = (uint8_t)(u32Address >> 16) & 0xff;
	au8CmdBuffer[3] = (uint8_t)(u32Address >> 24) & 0xff;

	eResponse = eBL_Request(iUartFd, BL_TIMEOUT_1S, E_BL_MSG_TYPE_RAM_RUN_REQUEST, 4, au8CmdBuffer, 0, NULL, &eRxType, &u8RxDataLen, NULL);
	if(eResponse != E_BL_RESPONSE_OK || eRxType != E_BL_MSG_TYPE_RAM_RUN_RESPONSE)
	{
		DBG_vPrintf(TRACE_BOOTLOADER, "%s: Response %02x\n", __FUNCTION__, eResponse);
		return(-1);
	}

	return(0);
}


/****************************************************************************
 *
 * NAME: iBL_ReadRAM
 *
 * DESCRIPTION:
 *
 * PARAMETERS: Name        RW  Usage
 *
 * RETURNS:
 * int			0 if success
 * 				-1 if an error occurred
 *
 ****************************************************************************/
static int iBL_ReadRAM(int iUartFd, uint32_t u32Address, uint8_t u8Length, uint8_t *pu8Buffer)
{
	uint8_t u8RxDataLen = 0;
	uint8_t au8CmdBuffer[6];
	teBL_Response eResponse = 0;
	teBL_MessageType eRxType = 0;

	if(u8Length > 0xfc || pu8Buffer == NULL)
	{
		return -1;
	}

	au8CmdBuffer[0] = (uint8_t)(u32Address >> 0) & 0xff;
	au8CmdBuffer[1] = (uint8_t)(u32Address >> 8) & 0xff;
	au8CmdBuffer[2] = (uint8_t)(u32Address >> 16) & 0xff;
	au8CmdBuffer[3] = (uint8_t)(u32Address >> 24) & 0xff;
	au8CmdBuffer[4] = u8Length;
	au8CmdBuffer[5] = 0;

	eResponse = eBL_Request(iUartFd, BL_TIMEOUT_1S, E_BL_MSG_TYPE_RAM_READ_REQUEST, 6, au8CmdBuffer, 0, NULL, &eRxType, &u8RxDataLen, pu8Buffer);
	if(eResponse != E_BL_RESPONSE_OK || eRxType != E_BL_MSG_TYPE_RAM_READ_RESPONSE)
	{
		DBG_vPrintf(TRACE_BOOTLOADER, "%s: Response %02x\n", __FUNCTION__, eResponse);
		return -1;
	}

	return(u8RxDataLen);
}

/****************************************************************************
 *
 * NAME: iBL_WriteRAM
 *
 * DESCRIPTION:
 *
 * PARAMETERS: Name        RW  Usage
 *
 * RETURNS:
 * int			0 if success
 * 				-1 if an error occurred
 *
 ****************************************************************************/
static int iBL_WriteRAM(int iUartFd, uint32_t u32Address, uint8_t u8Length, uint8_t *pu8Buffer)
{
	uint8_t u8RxDataLen = 0;
	uint8_t au8CmdBuffer[6];
	teBL_Response eResponse = 0;
	teBL_MessageType eRxType = 0;

	if(u8Length > 0xfc || pu8Buffer == NULL)
	{
		return(-1);
	}

	au8CmdBuffer[0] = (uint8_t)(u32Address >> 0) & 0xff;
	au8CmdBuffer[1] = (uint8_t)(u32Address >> 8) & 0xff;
	au8CmdBuffer[2] = (uint8_t)(u32Address >> 16) & 0xff;
	au8CmdBuffer[3] = (uint8_t)(u32Address >> 24) & 0xff;
	au8CmdBuffer[4] = u8Length;
	au8CmdBuffer[5] = 0;

	eResponse = eBL_Request(iUartFd, BL_TIMEOUT_1S, E_BL_MSG_TYPE_RAM_WRITE_REQUEST, 4, au8CmdBuffer, u8Length, pu8Buffer, &eRxType, &u8RxDataLen, pu8Buffer);
	if(eResponse != E_BL_RESPONSE_OK || eRxType != E_BL_MSG_TYPE_RAM_WRITE_RESPONSE)
	{
		DBG_vPrintf(TRACE_BOOTLOADER, "%s: Response %02x\n", __FUNCTION__, eResponse);
		return(-1);
	}

	return(u8Length);
}


/****************************************************************************
 *
 * NAME: iBL_EraseFlash
 *
 * DESCRIPTION:
 *
 * PARAMETERS: Name        RW  Usage
 *
 * RETURNS:
 * int          0 if success
 *              -1 if an error occurred
 *
 ****************************************************************************/
static int iBL_EraseFlash(int iUartFd)
{
    uint8_t u8RxDataLen = 0;
    teBL_Response eResponse = 0;
    teBL_MessageType eRxType = 0;
    uint8_t au8Buffer[64];

    eResponse = eBL_Request(iUartFd, BL_TIMEOUT_10S, E_BL_MSG_TYPE_FLASH_ERASE_REQUEST, 0, NULL, 0, NULL, &eRxType, &u8RxDataLen, au8Buffer);
    if(eResponse != E_BL_RESPONSE_OK || eRxType != E_BL_MSG_TYPE_FLASH_ERASE_RESPONSE)
    {
    	DBG_vPrintf(TRACE_BOOTLOADER, "%s: Response %02x\n", __FUNCTION__, eResponse);
        return(-1);
    }

    return(u8RxDataLen);
}


/****************************************************************************
 *
 * NAME: iBL_ReadFlash
 *
 * DESCRIPTION:
 *
 * PARAMETERS: Name        RW  Usage
 *
 * RETURNS:
 * int			0 if success
 * 				-1 if an error occurred
 *
 ****************************************************************************/
static int iBL_ReadFlash(int iUartFd, uint32_t u32Address, uint8_t u8Length, uint8_t *pu8Buffer)
{
	uint8_t u8RxDataLen = 0;
	uint8_t au8CmdBuffer[6];
	teBL_Response eResponse = 0;
	teBL_MessageType eRxType = 0;

	if(u8Length > 0xfc || pu8Buffer == NULL)
	{
		DBG_vPrintf(TRACE_BOOTLOADER, "Parameter error\n");
		return -1;
	}

	au8CmdBuffer[0] = (uint8_t)(u32Address >> 0) & 0xff;
	au8CmdBuffer[1] = (uint8_t)(u32Address >> 8) & 0xff;
	au8CmdBuffer[2] = (uint8_t)(u32Address >> 16) & 0xff;
	au8CmdBuffer[3] = (uint8_t)(u32Address >> 24) & 0xff;
	au8CmdBuffer[4] = u8Length;
	au8CmdBuffer[5] = 0;

	eResponse = eBL_Request(iUartFd, BL_TIMEOUT_1S, E_BL_MSG_TYPE_FLASH_READ_REQUEST, 6, au8CmdBuffer, 0, NULL, &eRxType, &u8RxDataLen, pu8Buffer);
	if(eResponse != E_BL_RESPONSE_OK || eRxType != E_BL_MSG_TYPE_FLASH_READ_RESPONSE)
	{
		DBG_vPrintf(TRACE_BOOTLOADER, "%s: Response %02x\n", __FUNCTION__, eResponse);
		return -1;
	}

	return u8RxDataLen;
}


/****************************************************************************
 *
 * NAME: iBL_WriteFlash
 *
 * DESCRIPTION:
 *
 * PARAMETERS: Name        RW  Usage
 *
 * RETURNS:
 * int          0 if success
 *              -1 if an error occurred
 *
 ****************************************************************************/
static int iBL_WriteFlash(int iUartFd, uint32_t u32Address, uint8_t u8Length, uint8_t *pu8Buffer)
{
    uint8_t u8RxDataLen = 0;
    uint8_t au8CmdBuffer[6];
    teBL_Response eResponse = 0;
    teBL_MessageType eRxType = 0;

    if(u8Length > 0xfc || pu8Buffer == NULL)
    {
        return -1;
    }

    au8CmdBuffer[0] = (uint8_t)(u32Address >> 0) & 0xff;
    au8CmdBuffer[1] = (uint8_t)(u32Address >> 8) & 0xff;
    au8CmdBuffer[2] = (uint8_t)(u32Address >> 16) & 0xff;
    au8CmdBuffer[3] = (uint8_t)(u32Address >> 24) & 0xff;
    au8CmdBuffer[4] = u8Length;
    au8CmdBuffer[5] = 0;

    eResponse = eBL_Request(iUartFd, BL_TIMEOUT_1S, E_BL_MSG_TYPE_FLASH_PROGRAM_REQUEST, 4, au8CmdBuffer, u8Length, pu8Buffer, &eRxType, &u8RxDataLen, pu8Buffer);
    if(eResponse != E_BL_RESPONSE_OK || eRxType != E_BL_MSG_TYPE_FLASH_PROGRAM_RESPONSE)
    {
    	DBG_vPrintf(TRACE_BOOTLOADER, "%s: Response %02x\n", __FUNCTION__, eResponse);
        return -1;
    }

    return u8Length;
}



/****************************************************************************/
/***        Local Functions                                               ***/
/****************************************************************************/
/****************************************************************************
 *
 * NAME: eBL_Request
 *
 * DESCRIPTION:
 *
 * PARAMETERS: Name        RW  Usage
 *
 * RETURNS:
 * void
 ****************************************************************************/
static teBL_Response eBL_Request(int iUartFd, int iTimeoutMicroseconds, teBL_MessageType eTxType, uint8_t u8HeaderLen, uint8_t *pu8Header, uint8_t u8TxLength, uint8_t *pu8TxData,
                          teBL_MessageType *peRxType, uint8_t *pu8RxLength, uint8_t *pu8RxData)
{
	/* Check data is not too long */
	if(u8TxLength > 0xfd)
	{
		DBG_vPrintf(TRACE_BOOTLOADER, "Data too long\n");
		return(E_BL_RESPONSE_ERROR);
	}

	/* Send message */
	if(iBL_WriteMessage(iUartFd, eTxType, u8HeaderLen, pu8Header, u8TxLength, pu8TxData) == -1)
	{
		return(E_BL_RESPONSE_ERROR);
	}

	/* Get the response to the request */
	return(eBL_ReadMessage(iUartFd, iTimeoutMicroseconds, peRxType, pu8RxLength, pu8RxData));
}


/****************************************************************************
 *
 * NAME: iBL_WriteMessage
 *
 * DESCRIPTION:
 *
 * PARAMETERS: Name        RW  Usage
 *
 * RETURNS:
 * int			0 if success
 * 				-1 if an error occured
 *
 ****************************************************************************/
static int iBL_WriteMessage(int iUartFd, teBL_MessageType eType, uint8_t u8HeaderLength, uint8_t *pu8Header, uint8_t u8Length, uint8_t *pu8Data)
{
	uint8_t u8CheckSum = 0;
	int n;

	uint8_t au8Msg[256];

	/* total message length cannot be > 255 bytes */
	if(u8HeaderLength + u8Length >= 0xfe)
	{
		DBG_vPrintf(TRACE_BOOTLOADER, "Length too big\n");
		return(-1);
	}

	/* Message length */
	au8Msg[0] = u8HeaderLength + u8Length + 2;

	/* Message type */
	au8Msg[1] = (uint8_t)eType;

	/* Message header */
	memcpy(&au8Msg[2], pu8Header, u8HeaderLength);

	/* Message payload */
	memcpy(&au8Msg[2 + u8HeaderLength], pu8Data, u8Length);

	for(n = 0; n < u8HeaderLength + u8Length + 2; n++)
	{
		u8CheckSum ^= au8Msg[n];
	}

	/* Message checksum */
	au8Msg[u8HeaderLength + u8Length + 2] = u8CheckSum;

	/* Write whole message to UART */
	UART_eWrite(iUartFd, au8Msg, u8HeaderLength + u8Length + 3);

	return(0);

}


/****************************************************************************
 *
 * NAME: eBL_ReadMessage
 *
 * DESCRIPTION:
 *
 * PARAMETERS: Name        RW  Usage
 *
 * RETURNS:
 * void
 ****************************************************************************/
static teBL_Response eBL_ReadMessage(int iUartFd, int iTimeoutMicroseconds, teBL_MessageType *peType, uint8_t *pu8Length, uint8_t *pu8Data)
{

	int n;
	teStatus eStatus;
	uint8_t au8Msg[256];
	uint8_t u8CalculatedCheckSum = 0;
	uint8_t u8Length = 0;
	teBL_Response eResponse = E_BL_RESPONSE_OK;
	int iAttempts = 0;
	int iBytesRead = 0;
	int iTotalBytesRead = 0;

	/* Get the length byte */
	if((UART_eRead(iUartFd, iTimeoutMicroseconds, 1, &u8Length, &iBytesRead) != E_STATUS_OK) || (iBytesRead != 1))
	{
		DBG_vPrintf(TRACE_BOOTLOADER, "Error getting length\n");
		return(E_BL_RESPONSE_NO_RESPONSE);
	}

//	DBG_vPrintf(TRACE_BOOTLOADER, "Got length %d\n", u8Length);

	/* Add length to checksum */
	u8CalculatedCheckSum ^= u8Length;

	do
	{
		/* Get the rest of the message */
		eStatus = UART_eRead(iUartFd, iTimeoutMicroseconds, u8Length - iTotalBytesRead, &au8Msg[iTotalBytesRead], &iBytesRead);
		if(eStatus != E_STATUS_OK)
		{
			DBG_vPrintf(TRACE_BOOTLOADER, "Error reading message from UART\n");
			return(E_BL_RESPONSE_NO_RESPONSE);
		}

		iTotalBytesRead += iBytesRead;
		iAttempts++;

	} while((iTotalBytesRead < u8Length) && (iBytesRead > 0 || iAttempts < 10));

	if(iTotalBytesRead != u8Length)
	{
		DBG_vPrintf(TRACE_BOOTLOADER, "Got %d bytes but expected %d after %d attempts: ", iTotalBytesRead, u8Length, iAttempts);
			for(n = 0; n < iTotalBytesRead; n++)
			{
				DBG_vPrintf(TRACE_BOOTLOADER, "%02x ", au8Msg[n]);
			}
			DBG_vPrintf(TRACE_BOOTLOADER, "\n");

		return(E_BL_RESPONSE_NO_RESPONSE);
	}

	/* Add rest of message to checksum */
//	DBG_vPrintf(TRACE_BOOTLOADER, "Got Data ");
	for(n = 0; n < u8Length; n++)
	{
//		DBG_vPrintf(TRACE_BOOTLOADER, "%02x ", au8Msg[n]);
		u8CalculatedCheckSum ^= au8Msg[n];
	}
//	DBG_vPrintf(TRACE_BOOTLOADER, "\n");

	if(u8CalculatedCheckSum != 0x00)
	{
//		DBG_vPrintf(TRACE_BOOTLOADER, "Checksum bad, got %02x expected %02x\n", u8CalculatedCheckSum, 0);
		return(E_BL_RESPONSE_CRC_ERROR);
	}

	*peType = au8Msg[0];
	*pu8Length = u8Length - 3;
	eResponse = au8Msg[1];
	memcpy(pu8Data, &au8Msg[2], *pu8Length);

	return(eResponse);

}

#if 0
/****************************************************************************
 *
 * NAME: pcGetName
 *
 * DESCRIPTION: Lookup name representing an ID.
 *
 * PARAMETERS: Name        RW  Usage
 *
 * RETURNS:
 * void
 ****************************************************************************/
static const char *pcGetName (tsIDNameLookup *psIDNameLookup, int iID)
{
    uint32_t i;
    for (i = 0; i < psIDNameLookup->u32Length; i++)
    {
        if (psIDNameLookup->asPairs[i].iID == iID)
        {
            return psIDNameLookup->asPairs[i].pcName;
        }
    }
    return "Unknown";
}
#endif

/****************************************************************************/
/***        END OF FILE                                                   ***/
/****************************************************************************/


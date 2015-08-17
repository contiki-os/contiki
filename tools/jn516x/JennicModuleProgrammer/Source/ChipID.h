/*****************************************************************************
 *
 * MODULE:              Chip ID's
 *
 * COMPONENT:           $RCSfile: ChipID.h,v $
 *
 * VERSION:             $Name:  $
 *
 * REVISION:            $Revision: 1.25 $
 *
 * DATED:               $Date: 2009/07/15 08:16:39 $
 *
 * STATUS:              $State: Exp $
 *
 * AUTHOR:              Lee Mitchell
 *
 * DESCRIPTION:
 *
 *
 * LAST MODIFIED BY:    $Author: lmitch $
 *                      $Modtime: $
 *
 ****************************************************************************
 *
 * This software is owned by Jennic and/or its supplier and is protected
 * under applicable copyright laws. All rights are reserved. We grant You,
 * and any third parties, a license to use this software solely and
 * exclusively on Jennic products. You, and any third parties must reproduce
 * the copyright and warranty notice and any other legend of ownership on
 * each copy or partial copy of the software.
 *
 * THIS SOFTWARE IS PROVIDED "AS IS". JENNIC MAKES NO WARRANTIES, WHETHER
 * EXPRESS, IMPLIED OR STATUTORY, INCLUDING, BUT NOT LIMITED TO, IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE,
 * ACCURACY OR LACK OF NEGLIGENCE. JENNIC SHALL NOT, IN ANY CIRCUMSTANCES,
 * BE LIABLE FOR ANY DAMAGES, INCLUDING, BUT NOT LIMITED TO, SPECIAL,
 * INCIDENTAL OR CONSEQUENTIAL DAMAGES FOR ANY REASON WHATSOEVER.
 *
 * Copyright Jennic Ltd 2005, 2006. All rights reserved
 *
 ***************************************************************************/

#ifndef  CHIP_ID_H_INCLUDED
#define  CHIP_ID_H_INCLUDED

#if defined __cplusplus
extern "C" {
#endif

/****************************************************************************/
/***        Include Files                                                 ***/
/****************************************************************************/

/****************************************************************************/
/***        Macro Definitions                                             ***/
/****************************************************************************/

/* Jennic/NXP Chip ID's */
#define CHIP_ID_MANUFACTURER_ID_MASK    0x00000fff
#define CHIP_ID_PART_MASK               0x003ff000
#define CHIP_ID_MASK_VERSION_MASK       0x0fc00000
#define CHIP_ID_REV_MASK                0xf0000000

#define CHIP_ID_JN5121_REV1A            0x00000000  /* ZED001               JN5121 Development      */
#define CHIP_ID_JN5121_REV2A            0x10000000  /* ZED002               JN5121 Development      */
#define CHIP_ID_JN5121_REV3A            0x20000000  /* ZED003               JN5121 Production       */

#define CHIP_ID_JN5131_REV1A            0x00001000  /* Alberich             Never Produced          */

#define CHIP_ID_JN5139_REV1A            0x00002000  /* BAL01                JN5139R                 */
#define CHIP_ID_JN5139_REV2A            0x10002000  /* BAL02A               JN5139R1                */
#define CHIP_ID_JN5139_REV2B            0x10002000  /* BAL02B               Test Chip Only          */
#define CHIP_ID_JN5139_REV2C            0x10802000  /* BAL02C   (Trent ROM) JN5139T01 & JN5139J01   */

#define CHIP_ID_JN5147_REV1A            0x00004686  /* */

#define CHIP_ID_JN5148_REV2A            0x10004686  /* JAG02A               JN5148                  */
#define CHIP_ID_JN5148_REV2B            0x10404686  /* JAG02B               JN5148                  */
#define CHIP_ID_JN5148_REV2C            0x10804686  /* JAG02C               JN5148                  */
#define CHIP_ID_JN5148_REV2D            0x10C04686  /* JAG02D   (Trent2 ROM)JN5148T01 & JN5148J01   */
#define CHIP_ID_JN5148_REV2E            0x11004686  /* JAG02E   (JAG03A?)   JN5148Z01               */

#define CHIP_ID_JN5142_REV1A            0x00005686  /* PUM01A               JN5142                  */
#define CHIP_ID_JN5142_REV1B            0x00425686  /* PUM01B               JN5142                  */
#define CHIP_ID_JN5142_REV1C            0x00845686  /* PUM01C               JN5142J01               */

#define CHIP_ID_COG03                   0x00006686  /* Cougar COG03                                 */
#define CHIP_ID_COG04                   0x00007686  /* Cougar COG04                                 */
#define CHIP_ID_JN5168                  0x00008686  /* Cougar COG05                                 */
#define CHIP_ID_JN5168_COG07            0x10008686  /* Cougar COG07                                 */


/* Macro to get just the Part ID from the chip ID */
#define CHIP_ID_PART(a)                 (a & CHIP_ID_PART_MASK)


/* SPI Flash device ID's  */
#define	FLASH_ST_M25P10			0x1010	/* ST microelectronics M25P10		*/
#define	FLASH_ST_M25P40			0x1212	/* ST microelectronics M25P40		*/
#define	FLASH_SST_SST25VF010A	0xbf49	/* SST SST25VF010A					*/
#define FLASH_INTERNAL			0xccee	/* Internal on-chip flash 			*/

/****************************************************************************/
/***        Type Definitions                                              ***/
/****************************************************************************/

/****************************************************************************/
/***        Exported Functions                                            ***/
/****************************************************************************/

/****************************************************************************/
/***        Exported Variables                                            ***/
/****************************************************************************/

#if defined __cplusplus
}
#endif

#endif  /* CHIP_ID_H_INCLUDED */

/****************************************************************************/
/***        END OF FILE                                                   ***/
/****************************************************************************/

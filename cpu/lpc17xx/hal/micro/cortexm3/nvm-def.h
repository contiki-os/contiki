/** @file hal/micro/cortexm3/nvm-def.h
 * @brief Data definitions for the Cortex-M3 Non-Volatile Memory data storage
 * system.
 * See @ref nvm for documentation.
 *
 * See hal/micro/cortexm3/nvm-def.h for source code.
 *
 * <!--(C) COPYRIGHT 2010 STMicroelectronics. All rights reserved.        -->
 */
 
/** @addtogroup nvm
 * @brief Data definitions for the Cortex-M3 Non-Volatile Memory data storage
 * system.
 *
 * This header defines all of the data sets in the NVM data storage.  Each
 * piece of data in NVM storage uses an offset to indicate where the data
 * lives and a size to indicate the size of that data.  Both the offset
 * and size are defined in bytes and must be a mupltiple of 16bits.  The
 * offset is from the start of an NVM page, defined by NVM_LEFT_PAGE and
 * NVM_RIGHT_PAGE.  The offset and size must be below the maximum size
 * of NVM storage as defined by NVM_DATA_SIZE_B.  All NVM data must start
 * above NVM_MGMT_SIZE_B, since this is where the management bytes live.
 *
 * @note This file is not directly used by the nvm.c or nvm.h files.  This
 *       file is intended to be a convenient place to define all data that
 *       lives in NVM so it can be seen together in one group.  nvm.h includes
 *       this file which means any code that includes nvm.h to call the read
 *       and write functions also has access to these defines.
 *@{
 */

#ifndef __NVM_DEF_H__
#define __NVM_DEF_H__

//The bottom 64 bytes of NVM storage is allocated to radio calibration
//values.  These 64 bytes *must* exist for the radio to function.
#define NVM_RADIO_CAL_OFFSET  (NVM_MGMT_SIZE_B+0x0000)
#define NVM_RADIO_CAL_SIZE_B  64

//IMPORTANT: Data storage starts at offset 0x0040.

//Three example pieces of data:
#define NVM_FOO_OFFSET  (NVM_MGMT_SIZE_B+0x0040)
#define NVM_FOO_SIZE_B  2
#define NVM_HAM_OFFSET  (NVM_MGMT_SIZE_B+0x0042)
#define NVM_HAM_SIZE_B  10
#define NVM_SPAM_OFFSET  (NVM_MGMT_SIZE_B+0x004C)
#define NVM_SPAM_SIZE_B  20

/** @} END addtogroup */

#endif // __NVM_DEF_H__


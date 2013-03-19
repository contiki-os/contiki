/** @file hal/micro/cortexm3/nvm.h
 * @brief Cortex-M3 Non-Volatile Memory data storage system.
 * See @ref nvm for documentation.
 *
 * The functions in this file return an ::StStatus value. 
 * See error-def.h for definitions of all ::StStatus return values.
 *
 * See hal/micro/cortexm3/nvm.h for source code.
 *
 * <!--(C) COPYRIGHT 2010 STMicroelectronics. All rights reserved.        -->
 */
 
/** @addtogroup nvm
 * @brief Cortex-M3 Non-Volatile Memory data storage system.
 *
 * This header defines the API for NVM data storage.  This header also
 * describes the algorithm behind the NVM data storage system with notes
 * on algorithm behavior.
 *
 * See hal/micro/cortexm3/nvm.h for source code.
 *
 * @note The algorithm description uses "page" to indicate an area of memory
 *       that is a multiple of physical flash pages.  There are two pages: LEFT
 *       and RIGHT.  The term "flash page" is used to refer to a page of
 *       physical flash.
 * 
 * NVM data storage works by alternating between two pages: LEFT and RIGHT.
 * The basic algorithm is driven by a call to halCommonSaveToNvm().  It will:
 * - erase the inactive page
 * - write the new data to the inactive page
 * - copy existing data from the active page to the inactive page
 * - mark the inactive page as the new active page
 * - mark the old active page as the new inactive page
 * To accomplish alternating between two pages and knowing which page has the
 * valid set of data, the algorithm uses 4 bytes of mgmt data that exists
 * at the top of both LEFT and RIGHT (the term "mgmt" is shorthand referring to
 * the management data).  The management data is comprised of a Valid marker,
 * an Active marker, a Dead marker, and a Spare byte.  Viewing the
 * management data as a single 32 bit quantity yields:
 * - Valid is mgmt[0]
 * - Active is mgmt[1]
 * - Dead is mgmt[2]
 * - Spare is mgmt[3]
 * The algorithm is based on a simple, circular state machine.  The following
 * discussion details all of the possible mgmt bytes and the states they
 * correspond to.  The "Reads from" line indicates which page a call to
 * halCommonReadFromNvm() will read from (an 'x' page will stuff the read
 * data with 0xFF).  The vertical "erase" and "write" words indicate the
 * flash altering actions taken between those states.  Invalid mgmt bytes
 * is equivalent to erased mgmt bytes (state 0) and will trigger an
 * erase of both LEFT and RIGHT.  State 3 and state 7 are the only exit
 * states.  When the algorithm is run, regardless of starting state, it
 * will advance to the next exit state.  This means if the "Read from"
 * is LEFT then the state machine will advance until state 7 and then exit.
 * If "Read from" is RIGHT, then the state machine will advance until
 * state 3 and then exit.
 * 
 * @code
 * Starting from erased or invalid mgmt, write to LEFT
 * State #       0     0         1      2      3  
 * Reads from:   x     x   e w   L      L      L  
 * Valid       xx|xx FF|FF r r 00|FF  00|FF  00|00
 * Active      xx|xx FF|FF a i 00|FF  00|FF  00|00
 * Dead        xx|xx FF|FF s t FF|FF  FF|00  FF|00
 * Spare       xx|xx FF|FF e e FF|FF  FF|FF  FF|FF
 * 
 * 
 * Starting from LEFT page, transition to RIGHT page:
 * State #      3       4       5      6      7  
 * Reads from:  L   e   L   w   R      R      R  
 * Valid      00|00 r 00|FF r 00|00  00|00  00|00
 * Active     00|00 a 00|FF i 00|FF  00|FF  00|00
 * Dead       FF|00 s FF|FF t FF|FF  00|FF  00|FF
 * Spare      FF|FF e FF|FF e FF|FF  FF|FF  FF|FF
 * 
 * 
 * Starting from RIGHT page, transition to LEFT page:
 * State #      7       8       9     10      3  
 * Reads from:  R   e   R   w   L      L      L  
 * Valid      00|00 r FF|00 r 00|00  00|00  00|00
 * Active     00|00 a FF|00 i FF|00  FF|00  00|00
 * Dead       00|FF s FF|FF t FF|FF  FF|00  FF|00
 * Spare      FF|FF e FF|FF e FF|FF  FF|FF  FF|FF
 * @endcode
 * 
 * Based on the 10 possible states, there are 5 valid 32bit mgmt words:
 * - 0xFFFFFFFF
 * - 0xFFFFFF00
 * - 0xFFFF0000
 * - 0xFF000000
 * - 0xFF00FFFF
 * The algorithm determines the current state by using these 5 mgmt words
 * with the 10 possible combinations of LEFT mgmt and RIGHT mgmt.
 * 
 * Detailed State Description:
 * - State 0:
 *   In this state the mgmt bytes do not conform to any of the other states
 *   and therefore the entire NVM system, both the LEFT and RIGHT, is
 *   invalid.  Invalid could be as simple as both LEFT and RIGHT are erased
 *   or as complex as serious memory corruption or a bug caused bad data to
 *   be written to the NVM.  By using a small set of very strict, precise,
 *   valid states (versus other management systems such as a simple counter),
 *   the algorithm/data gains some protection against not only corruption, but
 *   also executing the NVM algorithm on a chip that previously did not
 *   have the NVM system running on it.
 * - State 1, 4, 8
 *   In these states, mgmt is saying that one page is valid and active, while
 *   the other page is erased.  This tells the algorithm which page to read
 *   from and indicates that the other page has already been erased.
 * - State 2
 *   This state is only necessary for transitioning from state 0.  From state
 *   0, the goal is to arrive at state 3.  Ideally, the RIGHT mgmt would
 *   be written with 0xFF000000, but the flash library only permits 16 bit
 *   writes.  If a reset were to occur in the middle of this section of the
 *   algorithm, we want to ensure that the mgmt is left in a known state,
 *   state 2, so that the algorithm could continue from where it got
 *   interrupted.
 * - State 5, 9
 *   These states indicate that the other page has just become valid because
 *   the new data has just been written.  Once at these states, reading
 *   from the NVM will now pull data from the other page.
 * - State 6, 10
 *   These states indicate that the old page is now dead and not in use.
 *   While the algorithm already knows to read from the new page, the Dead
 *   mgmt byte is primarily used to indicate that the other page needs to
 *   be erased.  Conceptually, the Dead byte can also be considered a type
 *   of "garbage collection" flag indicating the old page needs to be
 *   destroyed and has not yet been erased.
 * - State 3, 7
 *   These states are the final exit points of the circular state machine.
 *   Once at these states, the current page is marked Valid and Active and
 *   the old page is marked as Dead.  The algorithm knows which page to
 *   read from and which page needs to be erased on the next write to the NVM.
 *   
 * 
 * Notes on algorithm behavior:
 * - Refer to nvm-def.h for a list of offset/length that define the data
 *   stored in NVM storage space.
 * - All writes to flash are 16bit granularity and therefore the internal
 *   flash writes cast the data to uint16_t.  Length is also required to be
 *   a multiple of 16bits.
 * - Flash page erase uses a granularity of a single flash page.  The size
 *   of a flash page depends on the chip and is defined in memmap.h with
 *   the define MFB_PAGE_SIZE_B.
 * - Erasing will only occur when halCommonSaveToNvm() is called.
 * - Erasing will always occur when halCommonSaveToNvm() is called unless the
 *   page intended to be erased is already entirely 0xFFFF.
 * - When reading and management is invalid, the read will return 0xFF for data.
 * - Calling halCommonSaveToNvm() while in any state is always valid and the
 *   new data will be written to flash.
 * - halCommonSaveToNvm() will always advance the state machine to 3 or 7.
 * - When writing and management is invalid, both LEFT and RIGHT will be erased
 *   and the new data will be written to LEFT.
 * - Writing causes the new data being passed into halCommonSaveToNvm() to be
 *   written to flash.  The data already existing in the currently valid page
 *   will be copied over to the new page.
 * - Reading or writing to an offset equal to or greater than NVM_DATA_SIZE_B is
 *   illegal and will cause an assert.
 * - Offset and length must always be multiples of 16bits.  If not, both a read
 *   and a write will trigger an assert.
 * - Offset and length must be supplied in bytes.
 * - All data in NVM storage must exist above the mgmt bytes, denoted by
 *   NVM_MGMT_SIZE_B.
 * - The bottom 64 bytes of NVM storage are allocated to radio calibration
 *   values.  These 64 bytes *must* exist for the radio to function.
 * - There is no error checking beyond checking for 16bit alignment.  This
 *   means it is possible to use data offset and size combinations that
 *   exceed NVM storage space or overlap with other data.  Be careful!
 *@{
 */


#ifndef __NVM_H__
#define __NVM_H__

//Pull in the MFB_ definitions.
#include "hal/micro/cortexm3/memmap.h"
//Pull in nvm-def.h so any code including nvm.h has access to the
//offsets and sizes defining the NVM data.
#include "hal/micro/cortexm3/nvm-def.h"
//Necessary to define StStatus and codes.
#include "error.h"


/**
 * @brief Copy the NVM data from flash into the provided RAM location.
 * It is illegal for the offset to be greater than NVM_DATA_SIZE_B.
 * 
 * @param data    A (RAM) pointer to where the data should be copied.
 *  
 * @param offset  The location from which the data should be copied.  Must be
 *                16bit aligned.
 * 
 * @param length  The length of the data in bytes.  Must be 16bit aligned.
 * 
 * @return An StStatus value indicating the success of the function.
 *  - ST_SUCCESS if the read completed cleanly.
 *  - ST_ERR_FATAL if the NVM storage management indicated an invalid
 *    state.  The function will return entirely 0xFF in the data parameter.
 */
StStatus halCommonReadFromNvm(void *data, uint32_t offset, uint16_t length);

/**
 * @brief Return the address of the token in NVM
 * 
 * @param offset  The location offset from which the address should be returned
 * 
 * 
 * @return The address requested
 */
uint16_t *halCommonGetAddressFromNvm(uint32_t offset);

/**
 * @brief Write the NVM data from the provided location RAM into flash.
 * It is illegal for the offset to be greater than NVM_DATA_SIZE_B.
 * 
 * @param data    A (RAM) pointer from where the data should be taken.
 *  
 * @param offset  The location to which the data should be written.  Must be
 *                16bit aligned.
 * 
 * @param length  The length of the data in bytes.  Must be 16bit aligned.
 * 
 * @return An StStatus value indicating the success of the function.
 *  - ST_SUCCESS if the write completed cleanly.
 *  - Any other status value is an error code generated by the low level
 *    flash erase and write API.  Refer to flash.h for details.
 */
StStatus halCommonWriteToNvm(const void *data, uint32_t offset, uint16_t length);

/**
 * @brief Define the number of physical flash pages that comprise a NVM page.
 * Since NVM_DATA_SIZE_B must be a multiple of MFB_PAGE_SIZE_B, increasing the
 * size of NVM storage should be done by modifying this define.
 *
 * @note The total flash area consumed by NVM storage is double this value.
 * This is due to the fact that there are two NVM pages, LEFT and RIGHT,
 * which the algorithm alternates between.
 */
#define NVM_FLASH_PAGE_COUNT  (1)

/**
 * @brief Define the total size of a NVM page, in bytes.  This must be a
 * multiple of the memory map define MFB_PAGE_SIZE_B.  Note that 4 bytes of
 * the total size of an NVM page are dedicated to page management.
 *
 * @note <b>DO NOT EDIT THIS DEFINE.  Instead, edit NVM_FLASH_PAGE_COUNT.</b>
 */
#define NVM_DATA_SIZE_B  (MFB_PAGE_SIZE_B*NVM_FLASH_PAGE_COUNT)
#if ((NVM_DATA_SIZE_B%MFB_PAGE_SIZE_B) != 0)
  #error Illegal NVM data storage size.  NVM_DATA_SIZE_B must be a multiple of MFB_PAGE_SIZE_B.
#endif

/**
 * @brief Define the absolute address of the LEFT page.  LEFT page storage
 * is defined by nvmStorageLeft[NVM_DATA_SIZE_B] and placed by the linker
 * using the segment "NVM".
 */
#define NVM_LEFT_PAGE   ((uint32_t)nvmStorageLeft)

/**
 * @brief Define the absolute address of the RIGHT page.  RIGHT page storage
 * is defined by nvmStorageRight[NVM_DATA_SIZE_B] and placed by the linker
 * using the segment "NVM".
 */
#define NVM_RIGHT_PAGE  ((uint32_t)nvmStorageRight)

/**
 * @brief Define the number of bytes that comprise the NVM management bytes.
 * All data must begin at an offset above the management bytes.
 *
 * @note This value <b>must not change</b>.
 */
#define NVM_MGMT_SIZE_B  (4)

/** @} END addtogroup */

#endif // __NVM_H__


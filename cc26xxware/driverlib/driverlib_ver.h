/******************************************************************************
*  Filename:       driverlib_ver.h
*  Revised:        $Date: 2014-04-24 16:43:24 +0200 (Thu, 24 Apr 2014) $
*  Revision:       $Revision: 40660 $
*
*  Description:    Provides functions and macros for checking DriverLib version
*
*  Copyright (c) 2015, Texas Instruments Incorporated
*  All rights reserved.
*
*  Redistribution and use in source and binary forms, with or without
*  modification, are permitted provided that the following conditions are met:
*
*  1) Redistributions of source code must retain the above copyright notice,
*     this list of conditions and the following disclaimer.
*
*  2) Redistributions in binary form must reproduce the above copyright notice,
*     this list of conditions and the following disclaimer in the documentation
*     and/or other materials provided with the distribution.
*
*  3) Neither the name of the ORGANIZATION nor the names of its contributors may
*     be used to endorse or promote products derived from this software without
*     specific prior written permission.
*
*  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
*  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
*  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
*  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
*  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
*  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
*  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
*  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
*  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
*  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
*  POSSIBILITY OF SUCH DAMAGE.
*
******************************************************************************/

//*****************************************************************************
//
//! \addtogroup driverlib_ver_api
//! @{
//
//*****************************************************************************

#ifndef __DRIVERLIB_VER_H__
#define __DRIVERLIB_VER_H__


#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>




/// DriverLib major revision (incompatible with other major versions)
#define DRIVERLIB_MAJOR_VER 2
/// DriverLib minor revision (may affect software use of DriverLib)
#define DRIVERLIB_MINOR_VER 00
/// DriverLib patch revision (patches and bug fixes)
#define DRIVERLIB_PATCH_VER 06
/// DriverLib build identification number (source code base version)
#define DRIVERLIB_BUILD_ID  42416




//*****************************************************************************
//
//! This macro is called internally from within DriverLib to declare the
//! DriverLib version locking object:
//! \param major is the DriverLib major revision.
//! \param minor is the DriverLib minor revision.
//! \param patch is the DriverLib patch revision.
//! \param build is the DriverLib build identification number.
//!
//! This macro shall not be called in the application unless the intention is
//! to bypass the version locking (at own risk).
//
//*****************************************************************************
#define DRIVERLIB_DECLARE_VERSION(major, minor, patch, build) \
    const volatile uint8_t driverlib_version_##major##_##minor##_##patch##_##build

/// External declaration of the DriverLib version locking object
extern DRIVERLIB_DECLARE_VERSION(2, 00, 06, 42416);




//*****************************************************************************
//
//! This macro shall be called once from within a function of a precompiled
//! software deliverable to lock the deliverable to a specific DriverLib
//! version. It is essential that the call is made from code that is not
//! optimized away.
//!
//! This macro locks to a specific DriverLib version:
//! \param major is the DriverLib major revision.
//! \param minor is the DriverLib minor revision.
//! \param patch is the DriverLib patch revision.
//! \param build is the DriverLib build identification number.
//!
//! If attempting to use the precompiled deliverable with a different version
//! of DriverLib, a linker error will be produced, stating that
//! "driverlib_version_xx_yy_zz_bbbbb is undefined" or similar.
//!
//! To override the check, for example when upgrading DriverLib but not the
//! precompiled deliverables, or when mixing precompiled deliverables,
//! application developers may (at own risk) declare the missing DriverLib
//! version using the \ref DRIVERLIB_DECLARE_VERSION() macro.
//
//*****************************************************************************
#define DRIVERLIB_ASSERT_VERSION(major, minor, patch, build) \
    (driverlib_version_##major##_##minor##_##patch##_##build)




//*****************************************************************************
//
//! This macro shall be called once from within a function of a precompiled
//! software deliverable to lock the deliverable to a specific DriverLib
//! version. It is essential that the call is made from code that is not
//! optimized away.
//!
//! This macro locks to the current DriverLib version used at compile-time.
//!
//! If attempting to use the precompiled deliverable with a different version
//! of DriverLib, a linker error will be produced, stating that
//! "driverlib_version_xx_yy_zz_bbbbb is undefined" or similar.
//!
//! To override the check, for example when upgrading DriverLib but not the
//! precompiled deliverables, or when mixing precompiled deliverables,
//! application developers may (at own risk) declare the missing DriverLib
//! version using the \ref DRIVERLIB_DECLARE_VERSION() macro.
//
//*****************************************************************************
#define DRIVERLIB_ASSERT_CURR_VERSION() \
    DRIVERLIB_ASSERT_VERSION(2, 00, 06, 42416)




#ifdef __cplusplus
}
#endif

#endif // __DRIVERLIB_VER_H__


//*****************************************************************************
//
//! Close the Doxygen group.
//! @}
//
//*****************************************************************************

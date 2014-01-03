================ Energy Micro Peripheral Library ============================

This directory, "emlib", contains the Energy Micro Peripheral Support 
library for the EFM32 series of microcontrollers and EFR4 series for RF
transceivers and System-On-Chip devices.

The "emlib" SW is designed to support all Energy Micro EFM and EFR devices,
from Gecko revision B and forward.

Some design guidelines for this library:

* Follow the guidelines established by ARM's and Energy Micro's adaptation 
  of the CMSIS (see below) standard.

* Be usable as a starting point for developing richer, more target specific 
  functionality (i.e. copy and modify further).

* Ability to be used as a standalone software component, to be used by other 
  drivers, that should cover "the most common cases".

* Readability of the code and usability preferred before optimization for speed
  and size or covering a particular "narrow" purpose.

* As little "cross-dependency" between modules as possible, to enable users to
  pick and choose what they want.

================ About CMSIS ================================================

These APIs are based on EFM32_CMSIS "Device" header file structure.

As a result of this, the library requires basic C99-support. You might have 
to enable C99 support in your compiler. Comments are in doxygen compatible 
format. 

The EFM32_CMSIS library contains all peripheral module registers and bit field
descriptors.

To download EFM32_CMSIS, go to
    http://www.energymicro.com/downloads

For more information about CMSIS see 
    http://www.onarm.com
    http://www.arm.com/products/CPUs/CMSIS.html

The requirements for using CMSIS also apply to this package.

================ File structure ==============================================

inc/ - header files
src/ - source files

================ Licenses ====================================================

See the top of each file for SW license. Basically you are free to use the
Energy Micro code for any project using Energy Micro devices. Parts of the 
CMSIS library is copyrighted by ARM Inc. See "License.doc" for ARM's CMSIS 
license.

================ Software updates ============================================

Energy Micro continually works to provide updated and improved emlib, example
code and other software of use for Energy Micro customers. Please check the 
download section of Energy Micro's web site at 

        http://www.energymicro.com/downloads

for the latest releases, news and updates. If you download and install the
Simplicity Studio application, you will be notified about updates when
available.


(C) Copyright Energy Micro AS, 2012

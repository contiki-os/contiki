lis3dh-fifo
===========

This is a simple example of using the FIFO buffer in the LIS3DH accelerometer.

This example will compute some statistics on the read values which will
require floating point support in the compiler, either through software
emulation (-msoft-float -mfloat-abi=soft for arm-none-eabi-gcc) or through a
CPU with a floating point unit (-mfloat-abi=hard or softfp on ARM, requires
a Cortex-M4F MCU)

ARM Cortex M0,M1,M3, and M4 without FPU

If you get hard faults in the compute_stats_3d function, make sure your GCC
was built with proper thumb2 fpu emulation support. Some compilers (e.g.
Gentoo crossdev by default) will build the ARM version even when compiling
for a thumb target.

CC2538 Base Platform
====================

This platform is designed to be a base platform for any other CC2538
based hardware platform. This should allow other platforms to reuse the code
in this folder instead of having to copy it to create a new platform.

As an example, the cc2538dk platform is based off of this one.


Creating a New CC2538-based Platform
------------------------------------

To create a platform based on the CC2538 you must define a couple files
that are specific to your platform.

First, the Makefile. Create a file named `Makefile.<platform>`

    Makefile.<platform>:

    ### Include the files from the base platform (cc2538)
    CONTIKI_TARGET_DIRS =  $(CONTIKI)/platform/cc2538

    ### Also include any folders specific to this platform
    CONTIKI_TARGET_DIRS += . dev

    ### Include the .c files that need to be compiled for this
    CONTIKI_TARGET_SOURCEFILES += sensors.c smartrf-sensors.c
    CONTIKI_TARGET_SOURCEFILES += button-sensor.c adc-sensor.c

    ### Configure make to clean the relevant files
    CLEAN += *.<platform>

    ### Include the base CC2538 platform makefile
    include $(CONTIKI)/platform/cc2538/Makefile.cc2538


Second, you must create a `platform.c` that holds platform specific
initialization code. A good starting point is the `platform.c` in the
cc2538dk platform.

    platform.c must have these three functions:

    /**
     * \brief First init after the core CC2538 peripherals are initialized
     */
    void
    platform_init_post_initial()
    {
    }

    /**
     * \brief Second init after the networking stack has been configured
     */
    void
    platform_init_post_networking()
    {
    }

    /**
     * \brief Init function at the very end before the main loop
     */
    void
    platform_init_end()
    {
    }


Third, you need a `platform-conf.h` to set any platform specific defines.
Here is a short example:

    platform-conf.h:

    #ifndef PLATFORM_CONF_H_
    #define PLATFORM_CONF_H_

    #define STARTUP_CONF_LEDS     1

    #endif /* PLATFORM_CONF_H_ */


Using the New Platform as a Target
----------------------------------

At this point the target should be usable like any other target/platform

    make TARGET=<platform>

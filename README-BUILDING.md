The Contiki build system
========================

The Contiki build system is designed to make it easy to compile Contiki
applications for different hardware platforms or into a simulation platform by
simply supplying different parameters to the make command, without having to
edit makefiles or modify the application code.

The file example project in examples/hello-world/ shows how the Contiki build
system works. The hello-world.c application can be built into a complete
Contiki system by running make in the examples/hello-world/ directory. Running
make without parameters will build a Contiki system using the native target.
The native target is a special Contiki platform that builds an entire Contiki
system as a program that runs on the development system. After compiling the
application for the native target it is possible to run the Contiki system with
the application by running the file hello-world.native. To compile the
application and a Contiki system for the ESB platform the command make
TARGET=esb is used. This produces a hello-world.esb file that can be loaded
into an ESB board.

To compile the hello-world application into a stand-alone executable that can
be loaded into a running Contiki system, the command make hello-world.ce is
used. To build an executable file for the ESB platform, make TARGET=esb
hello-world.ce is run.

To avoid having to type TARGET= every time make is run, it is possible to run
make TARGET=esb savetarget to save the selected target as the default target
platform for subsequent invocations of make. A file called Makefile.target
containing the currently saved target is saved in the project's directory.

Beside TARGET= there's DEFINES= which allows to set arbitrary variables for the
C preprocessor in form of a comma-separated list. Again it is possible to avoid
having to re-type i.e. DEFINES=MYTRACE,MYVALUE=4711 by running make TARGET=esb
DEFINES=MYTRACE,MYVALUE=4711 savedefines. A file called Makefile.esb.defines is
saved in the project's directory containing the currently saved defines for the
ESB platform.

Makefiles used in the Contiki build system The Contiki build system is composed
of a number of Makefiles. These are:

* Makefile: the project's makefile, located in the project directory.

* Makefile.include: the system-wide Contiki makefile, located in the root of
  the Contiki source tree.

* Makefile.$(TARGET) (where $(TARGET) is the name of the platform that is
  currently being built): rules for the specific platform, located in the
  platform's subdirectory in the platform/ directory.

* Makefile.$(CPU) (where $(CPU) is the name of the CPU or microcontroller
  architecture used on the platform for which Contiki is built): rules for the
  CPU architecture, located in the CPU architecture's subdirectory in the cpu/
  directory.

* Makefile.$(APP) (where $(APP) is the name of an application in the apps/
  directory): rules for applications in the apps/ directories. Each application
  has its own makefile.

The Makefile in the project's directory is intentionally simple. It specifies
where the Contiki source code resides in the system and includes the
system-wide Makefile, Makefile.include. The project's makefile can also define
in the APPS variable a list of applications from the apps/ directory that
should be included in the Contiki system. The Makefile used in the hello-world
example project looks like this:

    CONTIKI_PROJECT = hello-world
    all: $(CONTIKI_PROJECT)

    CONTIKI = ../..
    include $(CONTIKI)/Makefile.include

First, the location of the Contiki source code tree is given by defining the
CONTIKI variable. Next, the name of the application is defined. Finally, the
system-wide Makefile.include is included.

The Makefile.include contains definitions of the C files of the core Contiki
system. Makefile.include always reside in the root of the Contiki source tree.
When make is run, Makefile.include includes the Makefile.$(TARGET) as well as
all makefiles for the applications in the APPS list (which is specified by the
project's Makefile).

Makefile.$(TARGET), which is located in the platform/$(TARGET)/ directory,
contains the list of C files that the platform adds to the Contiki system. This
list is defined by the CONTIKI_TARGET_SOURCEFILES variable. The
Makefile.$(TARGET) also includes the Makefile.$(CPU) from the cpu/$(CPU)/
directory.

The Makefile.$(CPU) typically contains definitions for the C compiler used for
the particular CPU. If multiple C compilers are used, the Makefile.$(CPU) can
either contain a conditional expression that allows different C compilers to be
defined, or it can be completely overridden by the platform specific makefile
Makefile.$(TARGET).

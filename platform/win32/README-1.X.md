Win32
=====

The platform/win32/ directory contains a showcase of Contiki 1.x technologies
ported to the Contiki 2.x environment. As such it serves primarily two
purposes:

- Allow for easy interactive experience of the Contiki applications with user
  interface, especially the Contiki web browser.
- Allow for easy regression tests of the code in core/ctk/ and most of the code
  in apps/.

The employed Contiki 1.x technologies include:

- The 'Contiki Tool Kit' (CTK) UI framework in general
- CTK running in a character based environment (here the Win32 Console API)
- CTK mouse support
- Dynamic loading and unloading of Contiki programs (.PRG) and Contiki program
  descriptions (.DSC) based on dynamically loadable libraries (here Win32 DLLs)

A typical Contiki 1.x implementation consists of a core binary (containing
among others the uIP and CTK libraries) and the program (and program
description) binaries which are both loaded by the core and reference the core
libraries.  Therefore the Contiki 2.x project-based build system doesn't suit
exactly. So in order to be able to leverage the Contiki 2.x build system to its
maximum extend two makefiles are necessary:

- Makefile plays the role of a Contiki 2.x project Makefile
- Makefile.win32 is an ordinary (yet complex) Contiki 2.x Makefile.$(TARGET)

As platform/win32/Makefile.win32 includes cpu/native/Makefile.native the
WinPcap library is used for network I/O. Please consult
cpu/native/net/README-WPCAP.md for further details.

Both a Cygwin Bash Shell and an ordinary Windows Command Prompt are fine for
runtime. But in order to experience the CTK mouse support it is necessary to
deactivate the 'QuickEdit Mode' of the window used. This option is found in its
Windows system menu under 'Properties'.

As the console is used for displaying the actual user interface the log output
is routed to the debug output. DebugView is a very lean program for displaying
the debug output in case no debugger is active and does so. It is available at
[http://technet.microsoft.com/en-us/sysinternals/bb896647.aspx](http://technet.microsoft.com/en-us/sysinternals/bb896647.aspx).

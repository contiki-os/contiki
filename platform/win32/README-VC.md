Win32 - VC
==========

The 'win32' target allows projects to be built using two different toolchains:

GCC / Cygwin
------------

Start your build from a Cygwin Shell and set TARGET=win32 to use this toolchain.
The result will be similiar to the 'minimal-net' target. The advantage of 'win32'
over 'minimal-net' is the support for GUI applications using the Contiki Toolkit
(CTK). For this reason several projects in the /examples directory are built in
the target 'win32' by default.

VC++ / GnuWin
-------------

Start your build from a VC++ Command Prompt and set TARGET=win32 to use this
toolchain. You need however a few addional tools in your PATH:

- cp.exe / rm.exe [http://gnuwin32.sourceforge.net/packages/coreutils.htm](http://gnuwin32.sourceforge.net/packages/coreutils.htm)
- make.exe        [http://gnuwin32.sourceforge.net/packages/make.htm](http://gnuwin32.sourceforge.net/packages/make.htm)
- sed.exe         [http://gnuwin32.sourceforge.net/packages/sed.htm](http://gnuwin32.sourceforge.net/packages/sed.htm)
- makedepend.exe  [http://llg.cubic.org/docs/vc7.html](http://llg.cubic.org/docs/vc7.html)

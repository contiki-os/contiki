#!/usr/bin/python

# Copyright (c) 2015, SICS Swedish ICT
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
# 1. Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.
# 3. Neither the name of the Institute nor the names of its contributors
#    may be used to endorse or promote products derived from this software
#    without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
# ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
# FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
# OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
# HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
# LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
# OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
# SUCH DAMAGE.
#
# This file is part of the Contiki operating system.
#

import sys, os, platform
import multiprocessing

# detect the operating system
sysname = platform.system()
if "Linux" in sysname:
    IS_WINDOWS = False
    FLASH_PROGRAMMER_DEFAULT_PATH = "/usr/jn-toolchain/tools/flashprogrammer/JennicModuleProgrammer"
    import motelist_lib.linux_motelist_impl as motelist_impl # @UnusedImport

elif ("Win" in sysname) or ("NT" in sysname):
    IS_WINDOWS = True
    FLASH_PROGRAMMER_DEFAULT_PATH = 'C:\\NXP\\bstudio_nxp\\sdk\\JN-SW-4163\\Tools\\flashprogrammer\\FlashCLI.exe'
    import motelist_lib.windows_motelist_impl as motelist_impl # @Reimport @UnusedImport

else:
    print ("OS ('{}') is not supported".format(os.name))


def main():
    # use the default location
    flash_programmer = FLASH_PROGRAMMER_DEFAULT_PATH
    if len(sys.argv) > 2:
       flash_programmer=sys.argv[1]

    serial_dumper = ""
    if len(sys.argv) > 3:
       serial_dumper=sys.argv[3]

    motes = motelist_impl.list_motes(flash_programmer)
    if motes:
        motes.sort()
    print 'Found %d JN516X motes at:' %(len(motes))
    motes_str = ''
    for m in motes:
        motes_str += "%s " %(str(m))
    print motes_str

    firmware_file='#'
    if len(sys.argv) > 2:
        firmware_file = sys.argv[2]
    elif len(sys.argv) > 1:
        firmware_file = sys.argv[1]

    if firmware_file[0] == '\\':
        firmware_file = firmware_file[1:]

    if firmware_file not in ['#', '!', '?', '%']:
        print '\nBatch programming all connected motes...\n'
        motelist_impl.program_motes(flash_programmer, motes, firmware_file)
    elif firmware_file == '?' or firmware_file == '!':
        should_display_mac_list = (firmware_file == '!')
        motelist_impl.print_info(flash_programmer, motes, should_display_mac_list)
    elif firmware_file == '%':
        print '\nLogging from all connected motes...\n'
        motelist_impl.serialdump_ports(flash_programmer, serial_dumper, motes)
    else:
        print '\nNo firmware file specified.\n'

if __name__ == '__main__':
    multiprocessing.freeze_support()         
    main()

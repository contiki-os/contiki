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
# Author(s): 
#           Simon Duquennoy <simonduq@sics.se>
#           Atis Elsts <atis.elsts@sics.se>

import os, re, subprocess, multiprocessing
    
def list_motes(flash_programmer):
    #There is no COM0 in windows. We use this to trigger an error message that lists all valid COM ports
    cmd = [flash_programmer, '-c', 'COM0']
    proc = subprocess.Popen(cmd, shell=False, stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.PIPE,)
    stdout_value, stderr_value = proc.communicate('through stdin to stdout')
    com_str = (stderr_value)
    #print '\tpass through:', repr(stdout_value)
    #print '\tstderr      :', com_str

    ## Extract COM ports from output:
    ## Example com_str: "Available ports: ['COM15', 'COM10']" 
    res = re.compile('\[((?:\'COM\d+\'.?.?)+)\]').search(com_str)

    ports = []
    if res:
        port_str=str(res.group(1))
        ports=port_str.replace('\'', '').replace(',', '').split()
    return ports

def extract_information(port, stdout_value):
    mac_str=''
    info=''
    is_program_success=''

    #print 'output: ', stdout_value

#    res = re.compile('Connecting to device on (COM\d+)').search(stdout_value)
#    if res:
#        port_str = str(res.group(1))

    ### extracting the following information   
    '''
    Devicelabel:           JN516x, BL 0x00080006
    FlashLabel:            Internal Flash (256K)
    Memory:                0x00008000 bytes RAM, 0x00040000 bytes Flash
    ChipPartNo:            8
    ChipRevNo:             1
    ROM Version:           0x00080006
    MAC Address:           00:15:8D:00:00:35:DD:FB
    ZB License:            0x00.00.00.00.00.00.00.00.00.00.00.00.00.00.00.00
    User Data:             00:00:00:00:00:00:00:00
    FlashMID:              0xCC
    FlashDID:              0xEE
    MacLocation:           0x00000010
    Sector Length:         0x08000
    Bootloader Version:    0x00080006
    '''

    res = re.compile('(Devicelabel.*\sFlashLabel.*\sMemory.*\sChipPartNo.*\sChipRevNo.*\sROM Version.*\sMAC Address.*\sZB License.*\sUser Data.*\sFlashMID.*\sFlashDID.*\sMacLocation.*\sSector Length.*\sBootloader Version\:\s+0x\w{8})').search(stdout_value) 
    if res:
        info = str(res.group(1))
      
    res = re.compile('MAC Address\:\s+((?:\w{2}\:?){8})').search(stdout_value)
    if res:
        mac_str = str(res.group(1))

    res = re.compile('(Program\ssuccessfully\swritten\sto\sflash)').search(stdout_value)
    if res:
        is_program_success = str(res.group(1))

    return [mac_str, info, is_program_success] 
        
def program_motes(flash_programmer, motes, firmware_file):
  for m in motes:
      cmd = [flash_programmer, '-c', m, '-B', '1000000', '-s', '-w', '-f', firmware_file]
      proc = subprocess.Popen(cmd, shell=False, stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.PIPE,)
      stdout_value, stderr_value = proc.communicate('through stdin to stdout')   
      [mac_str, info, is_program_success] = extract_information(m, stdout_value)
      print m, mac_str, is_program_success

      errors = (stderr_value)
      if errors != '':
          print 'Errors:', errors  

def print_info(flash_programmer, motes, do_mac_only):
    if do_mac_only:
        print "Listing Mac addresses:"
    else:
        print "Listing mote info:"
    for m in motes:
        cmd=[flash_programmer, '-c', m, '-B', '1000000']
        proc = subprocess.Popen(cmd, shell=False, stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.PIPE,)
        stdout_value, stderr_value = proc.communicate('through stdin to stdout')
        [mac_str, info, is_program_success] = extract_information(m, stdout_value)

        errors = (stderr_value)
    
        if do_mac_only:
            print m, mac_str
        else:
            print m, '\n', info, '\n'
          
        if errors != '':
            print 'Errors:', errors

def serialdump(args):
    port_name = args[0]
    serial_dumper = args[1]
    cmd = [serial_dumper, '-b1000000', "/dev/" + port_name.lower()]
    if os.name == "posix" or os.name == "cygwin":
        cmd = " ".join(cmd)
    rv = subprocess.call(cmd, shell=True)

def serialdump_ports(flash_programmer, serial_dumper, ports):
    p = multiprocessing.Pool()
    p.map(serialdump, zip(ports, [serial_dumper] * len(ports)))
    p.close()

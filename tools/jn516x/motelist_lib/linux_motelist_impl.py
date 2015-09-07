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
#           Janis Judvaitis
#           Atis Elsts <atis.elsts@sics.se>

import os, glob, re
import multiprocessing, subprocess

FTDI_VENDOR_ID = "0403"
FTDI_PRODUCT_ID = "6001"

doPrintVendorID = False

def read_line(filename):
    """helper function to read a single line from a file"""
    line = "Unknown"
    try:
        with open(filename) as f:
            line = f.readline().strip()
    finally:
        return line

# try to extract descriptions from sysfs. this was done by experimenting,
# no guarantee that it works for all devices or in the future...

def usb_sysfs_hw_string(sysfs_path):
    """given a path to a usb device in sysfs, return a string describing it"""
    snr = read_line(sysfs_path + '/serial')
    if snr:
        snr_txt = '%s' % (snr,)
    else:
        snr_txt = ''
    if doPrintVendorID:
        return 'USB VID:PID=%s:%s SNR=%s' % (
                read_line(sysfs_path + '/idVendor'),
                read_line(sysfs_path + '/idProduct'),
                snr_txt
                )
    else:
        return snr_txt

def usb_string(sysfs_path):
    # Get dir name in /sys/bus/usb/drivers/usb for current usb dev
    dev = os.path.basename(os.path.realpath(sysfs_path))
    dev_dir = os.path.join("/sys/bus/usb/drivers/usb", dev)

    try:
        # Go to usb dev directory
        product = read_line(os.path.join(dev_dir, "product"))
        manufacturer = read_line(os.path.join(dev_dir, "manufacturer"))
        result = product + " by " + manufacturer
    except:
        result = "Unknown device"

    return result

def describe(device):
    """
    Get a human readable description.
    For USB-Serial devices try to run lsusb to get a human readable description.
    For USB-CDC devices read the description from sysfs.
    """
    base = os.path.basename(device)
    # USB-Serial devices
    sys_dev_path = '/sys/class/tty/%s/device/driver/%s' % (base, base)
    if os.path.exists(sys_dev_path):
        sys_usb = os.path.dirname(os.path.dirname(os.path.realpath(sys_dev_path)))
        return usb_string(sys_usb)

    # Arduino wants special handling
    sys_dev_path = '/sys/class/tty/%s/device/driver/' % (base)
    for x in os.listdir(sys_dev_path):
        # Driver directory's name contains device ID in /sys/bus/usb/drivers/usb
        temp = x.split(":")
        if len(temp) == 2:
            # No Arduino adds, need to save space!
            return usb_string(temp[0]).replace("(www.arduino.cc)", "").strip()

    # USB-CDC devices
    sys_dev_path = '/sys/class/tty/%s/device/interface' % (base,)
    if os.path.exists(sys_dev_path):
        return read_line(sys_dev_path)

    return base

def hwinfo(device):
    """Try to get a HW identification using sysfs"""
    base = os.path.basename(device)
    if os.path.exists('/sys/class/tty/%s/device' % (base,)):
        # PCI based devices
        sys_id_path = '/sys/class/tty/%s/device/id' % (base,)
        if os.path.exists(sys_id_path):
            return read_line(sys_id_path)
        # USB-Serial devices
        sys_dev_path = '/sys/class/tty/%s/device/driver/%s' % (base, base)
        if os.path.exists(sys_dev_path):
            sys_usb = os.path.dirname(os.path.dirname(os.path.realpath(sys_dev_path)))
            return usb_sysfs_hw_string(sys_usb)
        # USB-CDC devices
        if base.startswith('ttyACM'):
            sys_dev_path = '/sys/class/tty/%s/device' % (base,)
            if os.path.exists(sys_dev_path):
                return usb_sysfs_hw_string(sys_dev_path + '/..')
    return 'n/a'  # XXX directly remove these from the list?

#######################################

def is_nxp_mote(device):
    base = os.path.basename(device)
    # USB-Serial device?
    sys_dev_path = '/sys/class/tty/%s/device/driver/%s' % (base, base)
    if not os.path.exists(sys_dev_path):
        return False

    path_usb = os.path.dirname(os.path.dirname(os.path.realpath(sys_dev_path)))

    dev = os.path.basename(os.path.realpath(path_usb))
    dev_dir = os.path.join("/sys/bus/usb/drivers/usb", dev)

    try:
        idProduct = read_line(os.path.join(dev_dir, "idProduct"))
        idVendor = read_line(os.path.join(dev_dir, "idVendor"))
        if idVendor != FTDI_VENDOR_ID or idProduct != FTDI_PRODUCT_ID:
            return False
        product = read_line(os.path.join(dev_dir, "product"))
        manufacturer = read_line(os.path.join(dev_dir, "manufacturer"))
        if manufacturer != "NXP":
            return False
    except:
        return False
    return True


def list_motes(flash_programmer):
    devices = glob.glob('/dev/ttyUSB*')# + glob.glob('/dev/ttyACM*')
    return [d for d in devices if is_nxp_mote(d)]


def extract_information(port, stdout_value):
    mac_str='Unknown' # not supported on Linux
    info='' # not properly supported on Linux
    is_program_success=''

    info = describe(port) + ", SerialID: " + hwinfo(port)

    res = re.compile('(Success)').search(stdout_value)
    if res:
        is_program_success = str(res.group(1))
    else:
        res = re.compile('(Error .*)\n').search(stdout_value)
        if res:
            is_program_success = str(res.group(1))

    return [mac_str, info, is_program_success] 


def program_motes(flash_programmer, motes, firmware_file):
  for m in motes:
      cmd = [flash_programmer, '-v', '-s', m, '-I', '38400', '-P', '1000000', '-f', firmware_file]
      cmd = " ".join(cmd)
      proc = subprocess.Popen(cmd, shell=True, stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.PIPE,)
      stdout_value, stderr_value = proc.communicate('through stdin to stdout')   
      [mac_str, info, is_program_success] = extract_information(m, stdout_value)
      print m, is_program_success

      errors = (stderr_value)
      if errors != '':
          print 'Errors:', errors  


def print_info(flash_programmer, motes, do_mac_only):
    if do_mac_only:
        print "Listing Mac addresses (not supported on Linux):"
    else:
        print "Listing mote info:"

    for m in motes:
        [mac_str, info, is_program_success] = extract_information(m, '')
        if do_mac_only:
            print m, mac_str
        else:
            print m, '\n', info, '\n'

def serialdump(args):
    port_name = args[0]
    serial_dumper = args[1]
    rv = subprocess.call(serial_dumper + ' -b1000000 ' + port_name, shell=True)

def serialdump_ports(flash_programmer, serial_dumper, ports):
    p = multiprocessing.Pool()
    p.map(serialdump, zip(ports, [serial_dumper] * len(ports)))
    p.close()

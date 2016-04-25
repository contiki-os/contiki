# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#  * Redistributions of source code must retain the above copyright notice,
#    this list of  conditions and the following disclaimer.
#  * Redistributions in binary form must reproduce the above copyright
#   notice, this list of conditions and the following disclaimer in the
#   documentation and/or other materials provided with the distribution.
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS OR
# CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
# EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
# PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
# OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
# WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
# OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
# ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

import os
import glob
import re

printVID = False

def read_line(filename):
    """help function to read a single line from a file. returns none"""
    line = "Unknown"
    try:
        with open(filename) as f:
            line = f.readline().strip()
    finally:
        return line

def re_group(regexp, text):
    """search for regexp in text, return 1st group on match"""
    m = re.search(regexp, text)
    if m: return m.group(1)


# try to extract descriptions from sysfs. this was done by experimenting,
# no guarantee that it works for all devices or in the future...

def usb_sysfs_hw_string(sysfs_path):
    """given a path to a usb device in sysfs, return a string describing it"""
    snr = read_line(sysfs_path + '/serial')
    if snr:
        snr_txt = '%s' % (snr,)
    else:
        snr_txt = ''
    if printVID:
        return 'USB VID:PID=%s:%s SNR=%s' % (
                read_line(sysfs_path + '/idVendor'),
                read_line(sysfs_path + '/idProduct'),
                snr_txt
                )
    else:
        return snr_txt

# Old and wrong
# def usb_lsusb_string(sysfs_path):
#    bus, dev = os.path.basename(os.path.realpath(sysfs_path)).split('-')
#    print bus, " :::: ", dev
#    try:
#        desc = popen(['lsusb', '-v', '-s', '%s:%s' % (bus, dev)])
#        # descriptions from device
#        iManufacturer = re_group('iManufacturer\s+\w+ (.+)', desc)
#        iProduct = re_group('iProduct\s+\w+ (.+)', desc)
#        iSerial = re_group('iSerial\s+\w+ (.+)', desc) or ''
#        # descriptions from kernel
#        idVendor = re_group('idVendor\s+0x\w+ (.+)', desc)
#        idProduct = re_group('idProduct\s+0x\w+ (.+)', desc)
#        print "####", iManufacturer, iProduct, iSerial, idVendor, idProduct
#        # create descriptions. prefer text from device, fall back to the others
#        return '%s %s %s' % (iManufacturer or idVendor, iProduct or idProduct, iSerial)
#    except IOError:
#        return "IOError"

def usb_string(sysfs_path):
    # Get dir name in /sys/bus/usb/drivers/usb for current usb dev
    dev = os.path.basename(os.path.realpath(sysfs_path))
    devDir = os.path.join("/sys/bus/usb/drivers/usb", dev)

    try:
        # Go to usb dev directory
        product = read_line(os.path.join(devDir, "product"))
        manufacturer = read_line(os.path.join(devDir, "manufacturer"))
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

def comports():
    devices = glob.glob('/dev/ttyUSB*') + glob.glob('/dev/ttyACM*')
    return [(d, describe(d), hwinfo(d)) for d in devices]

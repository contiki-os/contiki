#!/usr/bin/env python
"""Test File generator.
This tool generates a hex file, of given size, ending on address
0xffff.

USAGE: hen-ihex.py size_in_kilobyte

The resulting Intel-hex file is output to stdout, use redirection
to save the data to a file.
"""

#return a string with data in intel hex format
def makeihex(address, data):
    out = []
    start = 0
    while start<len(data):
        end = start + 16
        if end > len(data): end = len(data)
        out.append(_ihexline(address, [ord(x) for x in data[start:end]]))
        start += 16
        address += 16
    out.append(_ihexline(address, [], end=1))   #append no data but an end line
    return ''.join(out)

def _ihexline(address, buffer, end=0):
    out = []
    if end:
        type = 1
    else:
        type = 0
    out.append( ':%02X%04X%02X' % (len(buffer),address&0xffff,type) )
    sum = len(buffer) + ((address>>8)&255) + (address&255)
    for b in buffer:
        if b == None: b = 0         #substitute nonexistent values with zero
        out.append('%02X' % (b&255) )
        sum += b&255
    out.append('%02X\n' %( (-sum)&255))
    return ''.join(out)

if __name__ == '__main__':
    import struct, sys
    if len(sys.argv) != 2:
        print __doc__
        sys.exit(1)
        
    size = int(sys.argv[1]) #in kilo
    startadr = 0x10000 - 1024*size
    data = ''.join([struct.pack(">H", x) for x in range(startadr, startadr+ 1024*size, 2)])
    print makeihex(startadr, data)

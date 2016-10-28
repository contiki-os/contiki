#!/usr/bin/env python


# Copyright (c) 2016, relayr http://relayr.io/
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
#
# 3. Neither the name of the copyright holder nor the names of its
#    contributors may be used to endorse or promote products derived
#    from this software without specific prior written permission.
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
# FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE
# COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
# INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
# (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
# SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
# HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
# STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
# ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
# OF THE POSSIBILITY OF SUCH DAMAGE.
#
# file
#         coap_firmware_update
# author
#         Nenad Ilic <nenad@relayr.io>

from multiprocessing import Queue
from coapthon import defines
from coapthon.client.coap import CoAP
from coapthon.messages.request import Request
import random
import struct
import crcmod
import sys

from intelhex import IntelHex

BLOCK1_SIZE = 64
IMG_HEADER_SIZE = 12
FLASH_CCA_SECOTR_START = 0x27f800

class FirmwareSend(object):
    def __init__(self, ip_addr, payload):
        self.queue = Queue()
        self.payload = (defines.Content_types['application/octet-stream'], payload)
        self.path = 'firmware'
        self.server = (ip_addr, 5683)
        self.protocol = CoAP(self.server, random.randint(1, 65535), self._wait_response)

    def _wait_response(self, message):
        if message.code != defines.Codes.CONTINUE.number:
            self.queue.put(message)

    def stop(self):
        self.protocol.stopped.set()
        self.queue.put(None)

    def send(self):
        request = Request()
        request.destination = self.server
        request.code = defines.Codes.PUT.number
        request.uri_path = self.path
        request.payload = self.payload
        request.block1 = (0,1, BLOCK1_SIZE)

        self.protocol.send_message(request)
        response = self.queue.get(block=True)
        return response

def main():

    ip_address = sys.argv[1]
    hex_filename = sys.argv[2]

    ih = IntelHex(hex_filename)
    last_address = ih.minaddr()
    # we need to exclude the FLASH_CCA_SECOTR
    for i in ih.addresses():
        if i >= FLASH_CCA_SECOTR_START:
            break
        last_address = i

    start_address = ih.minaddr()
    end_address = last_address
    size = end_address - start_address

    firmware_hex = ih.tobinstr(start=ih.minaddr(), end=last_address)

    crc16_func = crcmod.mkCrcFun(0x11021, rev=True, initCrc=0x0000, xorOut=0x0000)
    crc16 = crc16_func(firmware_hex)

    payload = struct.pack("<i", size)
    payload += struct.pack("<i", start_address)
    payload += struct.pack("<i", crc16)

    payload += "\0" * (BLOCK1_SIZE - IMG_HEADER_SIZE)

    payload += firmware_hex

    firmware = FirmwareSend(ip_addr=ip_address, payload=payload)

    response = firmware.send()

    firmware.stop()

    print("Dveice successfully updated")

if __name__ == '__main__':
    main()

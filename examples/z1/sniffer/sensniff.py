#!/usr/bin/env python

# Copyright (c) 2012, George Oikonomou (oikonomou@users.sf.net)
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#   * Redistributions of source code must retain the above copyright notice,
#     this list of conditions and the following disclaimer.
#   * Redistributions in binary form must reproduce the above copyright
#     notice, this list of conditions and the following disclaimer in the
#     documentation and/or other materials provided with the distribution.
#   * Neither the name of the owner nor the names of its contributors may be
#     used to endorse or promote products derived from this software without
#     specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
# LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
# CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
# SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
# INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
# CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
# ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
# POSSIBILITY OF SUCH DAMAGE.

# Read IEEE802.15.4 frames from a serial line (captured by a sniffer) and pipe
# them to wireshark. At the same time, the frames can be logged to a file for
# subsequent offline processing
# In interactive mode, the user can also input commands from stdin

# ToDo:
# * Python 3 support
# * Configuration file support (ConfigParser ?)
import serial
import argparse
import os
import sys
import select
import time
import stat
import errno
import StringIO
import logging
import logging.handlers
import struct
#####################################
### Constants
#####################################
__version__ = '0.1 alpha'
#####################################
### Default configuration values
#####################################
defaults = {
    'port': '/dev/ttyUSB0',
    'port_speed': 115200,
    'out_file': 'sensniff.hexdump',
    'out_fifo': '/tmp/sensniff',
    'out_pcap': 'sensniff.pcap',
    'debug_level': 'WARN',
    'log_level': 'INFO',
    'log_file': 'sensniff.log',
}
#####################################
### PCAP and Command constants
#####################################
LINKTYPE_IEEE802_15_4_NOFCS = 230
LINKTYPE_IEEE802_15_4 = 195
MAGIC_NUMBER = 0xA1B2C3D4
VERSION_MAJOR = 2
VERSION_MINOR = 4
THISZONE = 0
SIGFIGS = 0
SNAPLEN = 0xFFFF
NETWORK = LINKTYPE_IEEE802_15_4

PCAP_GLOBAL_HDR_FMT = '<LHHlLLL'
PCAP_FRAME_HDR_FMT = '<LLLL'

CMD_FRAME = 0x00
CMD_CHANNEL = 0x01
CMD_GET_CHANNEL = 0x81
CMD_SET_CHANNEL = 0x82
SNIFFER_PROTO_VERSION = 1
#####################################
### Globals
#####################################
logger = logging.getLogger(__name__)
stats = {}
#####################################
class Frame(object):
    def __init__(self, raw, timestamp):
        self.__raw = raw
        self.__t = timestamp
        self.len = len(self.__raw)

        self.__pcap_hdr = self.__generate_frame_hdr()

        self.pcap = self.__pcap_hdr + self.__raw
        self.hex = ''.join('%02x ' % ord(c) for c in self.__raw).rstrip()

    def __generate_frame_hdr(self):
        sec = int(self.__t)
        usec = (self.__t - sec) * 100000
        return struct.pack(PCAP_FRAME_HDR_FMT,
                           sec, usec, self.len, self.len)

    def get_pcap(self):
        return self.pcap

    def get_hex(self):
        return self.hex
#####################################
class DummyInputHandler(object):
    def __init__(self, line):
        self.__sensniff_magic = struct.pack('BBBB', 0xC1, 0x1F, 0xFE, 0x72)
        self.line = line
        stats['Captured'] = 0
        logger.info('DummyHandler initialised')

    def read_frame(self):
        infile = open(self.line, 'rb')
        for line in infile:
            stats['Captured'] += 1
            logger.info('Line input: %s' % (line.rstrip(),))
            return line
#####################################
class SerialInputHandler(object):
    def __init__(self,
                 port = defaults['port'],
                 baudrate = defaults['port_speed']):
        self.__sensniff_magic_legacy = struct.pack('BBBB', 0x53, 0x6E, 0x69, 0x66)
        self.__sensniff_magic = struct.pack('BBBB', 0xC1, 0x1F, 0xFE, 0x72)
        stats['Captured'] = 0
        stats['Non-Frame'] = 0
        try:
            self.port = serial.Serial(port = port,
                                      baudrate = baudrate,
                                      bytesize = serial.EIGHTBITS,
                                      parity = serial.PARITY_NONE,
                                      stopbits = serial.STOPBITS_ONE,
                                      xonxoff = False,
                                      rtscts = False,
                                      timeout = 0.1)
            self.port.flushInput()
            self.port.flushOutput()
        except (serial.SerialException, ValueError, IOError) as e:
            logger.error('Error opening port: %s' % (port,))
            logger.error('The error was: %s' % (e.args,))
            sys.exit(1)
        logger.info('Serial port %s opened' % (self.port.name))

    def read_frame(self):
        try:
            #self.get_channel()
            # Read the magic + 1 more byte
            b = self.port.read(5)
            size = len(b)
        except (IOError, OSError) as e:
            logger.error('Error reading port: %s' % (self.port.port,))
            logger.error('The error was: %s' % (e.args,))
            sys.exit(1)

        if size == 0:
            logger.debug('No frame input')
            return b
        if size < 5:
            logger.warn('Read %d bytes but not part of a frame'
                         % (size,))
            self.port.flushInput()
            return ''

        if b[0:4] not in (self.__sensniff_magic, self.__sensniff_magic_legacy):
            # Peripheral UART output - print it
            per_out = self.port.readline().rstrip()
            logger.info("Peripheral: %s%s" % (b, per_out))
            stats['Non-Frame'] += 1
            return ''

        # If we reach here:
        # Next byte == 1: Proto version 1, header follows
        # Next Byte != 1 && < 128. Old proto version. Frame follows, length == the byte
        b = bytearray(b)
        if b[4] != SNIFFER_PROTO_VERSION:
            # Legacy contiki sniffer support. Will slowly fade away
            size = b[4]
            try:
                b = self.port.read(size)
            except (IOError, OSError) as e:
                logger.error('Error reading port: %s' % (self.port.port,))
                logger.error('The error was: %s' % (e.args,))
                sys.exit(1)

            if len(b) != size:
                # We got the magic right but subsequent bytes did not match
                # what we expected to receive
                logger.warn('Read correct magic not followed by a frame')
                logger.warn('Expected %d bytes, got %d' % (size, len(b)))
                self.port.flushInput()
                return ''

            logger.info('Read a frame of size %d' % (len(b),))
            stats['Captured'] += 1
            return b

        # If we reach here, we have a packet of proto ver SNIFFER_PROTO_VERSION
        # Read CMD and LEN
        try:
            b = self.port.read(2)

        except (IOError, OSError) as e:
            logger.error('Error reading port: %s' % (self.port.port,))
            logger.error('The error was: %s' % (e.args[0],))
            sys.exit(1)

        if size < 2:
            logger.warn('Read correct magic not followed by a frame header')
            logger.warn('Expected 2 bytes, got %d' % (len(b), ))
            self.port.flushInput()
            return ''

        b = bytearray(b)
        cmd = b[0]
        length = b[1]

        # Read the frame or command response
        b = self.port.read(length)
        if len(b) != length:
            # We got the magic right but subsequent bytes did not match
            # what we expected to receive
            logger.warn('Read correct header not followed by a frame')
            logger.warn('Expected %d bytes, got %d' % (length, len(b)))
            self.port.flushInput()
            return ''

        # If we reach here, b holds a frame or a command response of length len
        if cmd == CMD_FRAME:
            logger.info('Read a frame of size %d' % (length,))
            stats['Captured'] += 1
            return b

        # If we reach here, we have a command response
        b = bytearray(b)
        logger.info('Received a command response: [%02x %02x]' % (cmd, b[0]))
        if cmd == CMD_CHANNEL:
            # We'll only ever see this if the user asked for it, so we are
            # running interactive. Print away
            print 'Sniffing in channel: %d' % (b[0],)
        else:
            logger.warn("Received a command response with unknown code")
        return ''

    def __write_command(self, cmd):
        self.port.write(self.__sensniff_magic)
        self.port.write(bytearray([SNIFFER_PROTO_VERSION]))
        self.port.write(cmd)
        self.port.flush()
        logger.debug('Sent bytes: '
                    + ''.join('%02x ' % ord(c) for c in self.__sensniff_magic)
                    + ('%02x ' % (SNIFFER_PROTO_VERSION))
                    + ''.join('%02x ' % c for c in cmd))

    def set_channel(self, channel):
        self.__write_command(bytearray([CMD_SET_CHANNEL, 1, channel]))

    def get_channel(self):
        self.__write_command(bytearray([CMD_GET_CHANNEL]))
#####################################
class FifoHandler(object):
    def __init__(self, out_fifo):
        self.out_fifo = out_fifo
        self.of = None
        self.needs_pcap_hdr = True
        stats['Piped'] = 0
        stats['Not Piped'] = 0
        self.__pcap_global_hdr = struct.pack(
            PCAP_GLOBAL_HDR_FMT, MAGIC_NUMBER, VERSION_MAJOR, VERSION_MINOR,
            THISZONE, SIGFIGS, SNAPLEN, NETWORK)

        self.__create_fifo()

    def __create_fifo(self):
        try:
            os.mkfifo(self.out_fifo)
            logger.info('Opened FIFO %s' % (self.out_fifo,))
        except OSError as e:
            if e.errno == errno.EEXIST:
                if stat.S_ISFIFO(os.stat(self.out_fifo).st_mode) is False:
                    logger.error('File %s exists and is not a FIFO'
                                 % (self.out_fifo,))
                    sys.exit(1)
                else:
                    logger.warn('FIFO %s exists. Using it' % (self.out_fifo,))
            else:
                raise

    def __open_fifo(self):
        try:
            fd = os.open(self.out_fifo, os.O_NONBLOCK | os.O_WRONLY)
            self.of = os.fdopen(fd, 'w')
        except OSError as e:
            if e.errno == errno.ENXIO:
                logger.warn('Remote end not reading')
                stats['Not Piped'] += 1
                self.of = None
                self.needs_pcap_hdr = True
            elif e.errno == errno.ENOENT:
                logger.error('%s vanished under our feet' % (self.out_fifo,))
                logger.error('Trying to re-create it')
                self.__create_fifo_file()
                self.of = None
                self.needs_pcap_hdr = True
            else:
                raise

    def handle(self, data):
        if self.of is None:
            self.__open_fifo()

        if self.of is not None:
            try:
                if self.needs_pcap_hdr is True:
                    self.of.write(self.__pcap_global_hdr)
                    self.needs_pcap_hdr = False
                self.of.write(data.pcap)
                self.of.flush()
                logger.debug('Wrote a frame of size %d bytes' % (data.len))
                stats['Piped'] += 1
            except IOError as e:
                if e.errno == errno.EPIPE:
                    logger.info('Remote end stopped reading')
                    stats['Not Piped'] += 1
                    self.of = None
                    self.needs_pcap_hdr = True
                else:
                    raise
#####################################
class PcapDumpHandler(object):
    def __init__(self, out_pcap):
        self.out_pcap = out_pcap
        stats['Dumped to PCAP'] = 0
        self.__pcap_global_hdr = struct.pack(
            PCAP_GLOBAL_HDR_FMT, MAGIC_NUMBER, VERSION_MAJOR, VERSION_MINOR,
            THISZONE, SIGFIGS, SNAPLEN, NETWORK)

        try:
            self.of = open(self.out_pcap, 'w')
            self.of.write(self.__pcap_global_hdr)
            logger.info("Dumping PCAP to %s" % (self.out_pcap,))
        except IOError as e:
            self.of = None
            logger.warn("Error opening %s to save pcap. Skipping"
                         % (out_pcap))
            logger.warn("The error was: %d - %s"
                         % (e.args))

    def handle(self, frame):
        if self.of is None:
            return
        self.of.write(frame.get_pcap())
        self.of.flush()
        logger.info('PcapDumpHandler: Dumped a frame of size %d bytes'
                     % (frame.len))
        stats['Dumped to PCAP'] += 1
#####################################
class HexdumpHandler(object):
    def __init__(self, of):
        stats['Dumped as Hex'] = 0
        try:
            self.of = open(of, 'wb')
            logger.info("Dumping hex to %s" % (of,))
        except IOError as e:
            logger.warn("Error opening %s for hex dumps. Skipping"
                         % (of))
            logger.warn("The error was: %d - %s" % (e.args))
            self.of = None

    def handle(self, frame):
        if self.of is None:
            return

        try:
            self.of.write('0000 ')
            self.of.write(frame.get_hex())
            self.of.write('\n')
            self.of.flush()
            stats['Dumped as Hex'] += 1
            logger.info('HexdumpHandler: Dumped a frame of size %d bytes'
                         % (frame.len))
        except IOError as e:
            logger.warn("Error writing hex to %s for hex dumps. Skipping"
                     % (self.of))
            logger.warn("The error was: %d - %s" % (e.args))
#####################################
def arg_parser():
    speed_choices = (9600, 19200, 38400, 57600, 115200, 230400, 460800)
    debug_choices = ('DEBUG', 'INFO', 'WARN', 'ERROR')

    parser = argparse.ArgumentParser(add_help = False,
                                     description = 'Read IEEE802.15.4 frames \
    from a sensniff enabled device, convert them to pcap and pipe them \
    into wireshark over a FIFO pipe for online analysis. Frames \
    can also be saved in a file in hexdump and/or pcap format for offline \
    analysis.')

    in_group = parser.add_argument_group('Input Options')
    in_group.add_argument('-b', '--baud', type = int, action = 'store',
                          choices = speed_choices,
                          default = defaults['port_speed'],
                          help = 'Set the line\'s BAUD rate to BAUD \
                                  (Default: %s)' % (defaults['port_speed'],))
    in_group.add_argument('-d', '--device', action = 'store',
                          default = defaults['port'],
                          help = 'Read from device DEVICE \
                                  (Default: %s)' % (defaults['port'],))

    out_group = parser.add_argument_group('Output Options')
    out_group.add_argument('-f', '--file', action = 'store', nargs = '?',
                           const = defaults['out_file'], default = False,
                           help = 'Save the capture (hexdump) file FILE. \
                                   If -f is specified but FILE is omitted, \
                                   %s will be used. If the argument is \
                                   omitted altogether, the capture will not \
                                   be saved.' % (defaults['out_file'],))
    out_group.add_argument('-p', '--pcap', action = 'store', nargs = '?',
                           const = defaults['out_pcap'], default = False,
                           help = 'Save the capture (pcap format) in PCAP. \
                                   If -p is specified but PCAP is omitted, \
                                   %s will be used. If the argument is \
                                   omitted altogether, the capture will not \
                                   be saved.' % (defaults['out_pcap'],))
    out_group.add_argument('-F', '--fifo', action = 'store',
                           default = defaults['out_fifo'],
                           help = 'Pipe the capture through OUTPUT_FIFO \
                                   (Default: %s)' % (defaults['out_fifo'],))
    out_group.add_argument('-o', '--offline', action = 'store_true',
                           default = False,
                           help = 'Disable piping (Mainly used for debugging) \
                                   (Default: Piping enabled)')

    log_group = parser.add_argument_group('Verbosity and Logging')
    log_group.add_argument('-n', '--non-interactive', action = 'store_true',
                           default = False,
                           help = 'Run in non-interactive mode, without \
                                   accepting user input. (Default Disabled)')
    log_group.add_argument('-D', '--debug-level', action = 'store',
                           choices = debug_choices,
                           default = defaults['debug_level'],
                           help = 'Print messages of severity DEBUG_LEVEL \
                                   or higher (Default %s)'
                                   % (defaults['debug_level'],))
    log_group.add_argument('-L', '--log-file', action = 'store', nargs = '?',
                           const = defaults['log_file'], default = False,
                           help = 'Log output in LOG_FILE. If -L is specified \
                                   but LOG_FILE is omitted, %s will be used. \
                                   If the argument is omitted altogether, \
                                   logging will not take place at all.'
                                   % (defaults['log_file'],))
    log_group.add_argument('-l', '--log-level', action = 'store',
                           choices = debug_choices,
                           default = defaults['log_level'],
                           help = 'Log messages of severity LOG_LEVEL or \
                                   higher. Only makes sense if -L is also \
                                   specified (Default %s)'
                                   % (defaults['log_level'],))

    gen_group = parser.add_argument_group('General Options')
    gen_group.add_argument('-v', '--version', action = 'version',
                           version = 'sensniff v%s' % (__version__))
    gen_group.add_argument('-h', '--help', action = 'help',
                           help = 'Shows this message and exits')

    return parser.parse_args()
#####################################
def dump_stats():
    s = StringIO.StringIO()

    s.write('Frame Stats:\n')
    for k, v in stats.items():
        s.write('%20s: %d\n' % (k, v))

    print(s.getvalue())
#####################################
def log_init():
    logger.setLevel(logging.DEBUG)
    ch = logging.StreamHandler()
    ch.setLevel(getattr(logging, args.debug_level))
    cf = logging.Formatter('%(message)s')
    ch.setFormatter(cf)
    logger.addHandler(ch)

    if args.log_file is not False:
        fh = logging.handlers.RotatingFileHandler(filename = args.log_file,
                                                  maxBytes = 5000000)
        fh.setLevel(getattr(logging, args.log_level))
        ff = logging.Formatter(
            '%(asctime)s - %(levelname)8s - %(message)s')
        fh.setFormatter(ff)
        logger.addHandler(fh)
#####################################
if __name__ == '__main__':
    args = arg_parser()
    log_init()

    logger.info('Started logging')

    serial_handler = SerialInputHandler(port = args.device, baudrate = args.baud)
#    serial_handler = DummyInputHandler('/tmp/in')

    handlers = []
    if args.offline is not True:
        f = FifoHandler(out_fifo = args.fifo)
        handlers.append(f)
    if args.file is not False:
        handlers.append(HexdumpHandler(of = args.file))
    if args.pcap is not False:
        handlers.append(PcapDumpHandler(args.pcap))

    if args.non_interactive is False:
        h = StringIO.StringIO()
        h.write('Commands:\n')
        h.write('c: Print current RF Channel\n')
        h.write('n: Trigger new pcap header before the next frame\n')
        h.write('h,?: Print this message\n')
        h.write('[11,26]: Change RF channel\n')
        h.write('q: Quit')
        h = h.getvalue()

        e = 'Unknown Command. Type h or ? for help'

        print h

    while 1:
        if args.non_interactive is False:
            try:
                if select.select([sys.stdin, ], [], [], 0.0)[0]:
                    cmd = sys.stdin.readline().rstrip()
                    logger.debug('User input: "%s"' % (cmd,))
                    if cmd in ('h', '?'):
                        print h
                    elif cmd == 'c':
                        serial_handler.get_channel()
                    elif cmd == 'n':
                        f.needs_pcap_hdr = True
                    elif cmd == 'q':
                        logger.info('User requested shutdown')
                        dump_stats()
                        sys.exit(0)
                    elif int(cmd) in range(11, 27):
                        serial_handler.set_channel(int(cmd))
                    else:
                        raise ValueError
                else:
                    logger.debug('No user input')
            except select.error:
                logger.warn('Error while trying to read stdin')
            except ValueError:
                print e
            except UnboundLocalError:
                # Raised by command 'n' when -o was specified at command line
                pass

        try:
            raw = serial_handler.read_frame()
            if len(raw) > 0:
                t = time.time()
                frame = Frame(raw, t)
                for h in handlers:
                    h.handle(frame)
        except (KeyboardInterrupt, SystemExit):
            logger.info('Shutting down')
            dump_stats()
            sys.exit(0)


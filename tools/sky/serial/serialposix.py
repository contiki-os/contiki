#!/usr/bin/env python
#module for serial IO for POSIX compatible systems, like Linux
#see __init__.py
#
#(C) 2001-2002 Chris Liechti <cliechti@gmx.net>
# this is distributed under a free software license, see license.txt
#
#parts based on code from Grant B. Edwards  <grante@visi.com>:
#  ftp://ftp.visi.com/users/grante/python/PosixSerial.py
# references: http://www.easysw.com/~mike/serial/serial.html

import sys, os, fcntl, termios, struct, string, select
import serialutil

VERSION = string.split("$Revision: 1.1 $")[1]     #extract CVS version

PARITY_NONE, PARITY_EVEN, PARITY_ODD = range(3)
STOPBITS_ONE, STOPBITS_TWO = (1, 2)
FIVEBITS, SIXBITS, SEVENBITS, EIGHTBITS = (5,6,7,8)

#Do check the Python version as some constants have moved.
if (sys.hexversion < 0x020100f0):
    import TERMIOS
else:
    TERMIOS = termios

if (sys.hexversion < 0x020200f0):
    import FCNTL
else:
    FCNTL = fcntl

#try to detect the os so that a device can be selected...
plat = string.lower(sys.platform)

if   plat[:5] == 'linux':    #Linux (confirmed)
    def device(port):
        return '/dev/ttyS%d' % port

elif plat == 'cygwin':       #cywin/win32 (confirmed)
    def device(port):
        return '/dev/com%d' % (port + 1)

elif plat     == 'openbsd3': #BSD (confirmed)
    def device(port):
        return '/dev/ttyp%d' % port

elif plat[:3] == 'bsd' or  \
     plat[:6] == 'netbsd' or \
     plat[:7] == 'freebsd' or \
     plat[:7] == 'openbsd' or \
     plat[:6] == 'darwin':   #BSD (confirmed for freebsd4: cuaa%d)
    def device(port):
        return '/dev/cuaa%d' % port

elif plat[:4] == 'irix':     #IRIX (not tested)
    def device(port):
        return '/dev/ttyf%d' % port

elif plat[:2] == 'hp':       #HP-UX (not tested)
    def device(port):
        return '/dev/tty%dp0' % (port+1)

elif plat[:5] == 'sunos':    #Solaris/SunOS (confirmed)
    def device(port):
        return '/dev/tty%c' % (ord('a')+port)

elif plat[:3] == 'dgux':     #Digital UNIX (not tested)
    def device(port):
        return '/dev/tty0%d' % (port+1)

else:
    #platform detection has failed...
    info = "sys.platform = %r\nos.name = %r\nserialposix.py version = %s" % (sys.platform, os.name, VERSION)
    print """send this information to the author of this module:

%s

also add the device name of the serial port and where the
counting starts for the first serial port.
e.g. 'first serial port: /dev/ttyS0'
and with a bit luck you can get this module running...
"""
    raise Exception, "this module does not run on this platform, sorry."

#whats up with "aix", "beos", "sco", ....
#they should work, just need to know the device names.


# construct dictionaries for baud rate lookups
baudEnumToInt = {}
baudIntToEnum = {}
for rate in (0,50,75,110,134,150,200,300,600,1200,1800,2400,4800,9600,
             19200,38400,57600,115200,230400,460800,500000,576000,921600,
             1000000,1152000,1500000,2000000,2500000,3000000,3500000,4000000
    ):
    try:
        i = eval('TERMIOS.B'+str(rate))
        baudEnumToInt[i]=rate
        baudIntToEnum[rate] = i
    except:
        pass


#load some constants for later use.
#try to use values from TERMIOS, use defaults from linux otherwise
TIOCMGET  = hasattr(TERMIOS, 'TIOCMGET') and TERMIOS.TIOCMGET or 0x5415
TIOCMBIS  = hasattr(TERMIOS, 'TIOCMBIS') and TERMIOS.TIOCMBIS or 0x5416
TIOCMBIC  = hasattr(TERMIOS, 'TIOCMBIC') and TERMIOS.TIOCMBIC or 0x5417
TIOCMSET  = hasattr(TERMIOS, 'TIOCMSET') and TERMIOS.TIOCMSET or 0x5418

#TIOCM_LE = hasattr(TERMIOS, 'TIOCM_LE') and TERMIOS.TIOCM_LE or 0x001
TIOCM_DTR = hasattr(TERMIOS, 'TIOCM_DTR') and TERMIOS.TIOCM_DTR or 0x002
TIOCM_RTS = hasattr(TERMIOS, 'TIOCM_RTS') and TERMIOS.TIOCM_RTS or 0x004
#TIOCM_ST = hasattr(TERMIOS, 'TIOCM_ST') and TERMIOS.TIOCM_ST or 0x008
#TIOCM_SR = hasattr(TERMIOS, 'TIOCM_SR') and TERMIOS.TIOCM_SR or 0x010

TIOCM_CTS = hasattr(TERMIOS, 'TIOCM_CTS') and TERMIOS.TIOCM_CTS or 0x020
TIOCM_CAR = hasattr(TERMIOS, 'TIOCM_CAR') and TERMIOS.TIOCM_CAR or 0x040
TIOCM_RNG = hasattr(TERMIOS, 'TIOCM_RNG') and TERMIOS.TIOCM_RNG or 0x080
TIOCM_DSR = hasattr(TERMIOS, 'TIOCM_DSR') and TERMIOS.TIOCM_DSR or 0x100
TIOCM_CD  = hasattr(TERMIOS, 'TIOCM_CD') and TERMIOS.TIOCM_CD or TIOCM_CAR
TIOCM_RI  = hasattr(TERMIOS, 'TIOCM_RI') and TERMIOS.TIOCM_RI or TIOCM_RNG
#TIOCM_OUT1 = hasattr(TERMIOS, 'TIOCM_OUT1') and TERMIOS.TIOCM_OUT1 or 0x2000
#TIOCM_OUT2 = hasattr(TERMIOS, 'TIOCM_OUT2') and TERMIOS.TIOCM_OUT2 or 0x4000
TIOCINQ   = hasattr(TERMIOS, 'FIONREAD') and TERMIOS.FIONREAD or 0x541B

TIOCM_zero_str = struct.pack('I', 0)
TIOCM_RTS_str = struct.pack('I', TIOCM_RTS)
TIOCM_DTR_str = struct.pack('I', TIOCM_DTR)

portNotOpenError = ValueError('port not open')

class Serial(serialutil.FileLike):
    def __init__(self,
                 port,                  #number of device, numbering starts at
                                        #zero. if everything fails, the user
                                        #can specify a device string, note
                                        #that this isn't portable anymore
                 baudrate=9600,         #baudrate
                 bytesize=EIGHTBITS,    #number of databits
                 parity=PARITY_NONE,    #enable parity checking
                 stopbits=STOPBITS_ONE, #number of stopbits
                 timeout=None,          #set a timeout value, None for waiting forever
                 xonxoff=0,             #enable software flow control
                 rtscts=0,              #enable RTS/CTS flow control
                 ):
        """init comm port"""
        self.fd = None
        self.timeout = timeout
        vmin = vtime = 0                #timeout is done via select
        #open
        if type(port) == type(''):      #strings are taken directly
            self.portstr = port
        else:
            self.portstr = device(port) #numbers are transformed to a os dependant string
        try:
            self.fd = os.open(self.portstr, os.O_RDWR|os.O_NOCTTY|os.O_NONBLOCK)
        except Exception, msg:
            self.fd = None
            raise serialutil.SerialException, "could not open port: %s" % msg
        fcntl.fcntl(self.fd, FCNTL.F_SETFL, 0)  #set blocking
        try:
            self.__tcgetattr()          #read current settings
        except termios.error, msg:      #if a port is nonexistent but has a /dev file, it'll fail here
            raise serialutil.SerialException, "could not open port: %s" % msg
        #set up raw mode / no echo / binary
        self.cflag = self.cflag |  (TERMIOS.CLOCAL|TERMIOS.CREAD)
        self.lflag = self.lflag & ~(TERMIOS.ICANON|TERMIOS.ECHO|TERMIOS.ECHOE|TERMIOS.ECHOK|TERMIOS.ECHONL|
                                    TERMIOS.ECHOCTL|TERMIOS.ECHOKE|TERMIOS.ISIG|TERMIOS.IEXTEN) #|TERMIOS.ECHOPRT
        self.oflag = self.oflag & ~(TERMIOS.OPOST)
        if hasattr(TERMIOS, 'IUCLC'):
            self.iflag = self.iflag & ~(TERMIOS.INLCR|TERMIOS.IGNCR|TERMIOS.ICRNL|TERMIOS.IUCLC|TERMIOS.IGNBRK)
        else:
            self.iflag = self.iflag & ~(TERMIOS.INLCR|TERMIOS.IGNCR|TERMIOS.ICRNL|TERMIOS.IGNBRK)
        #setup baudrate
        try:
            self.ispeed = self.ospeed = baudIntToEnum[baudrate]
        except:
            raise ValueError,'invalid baud rate: %s' % baudrate
        #setup char len
        self.cflag = self.cflag & ~TERMIOS.CSIZE
        if bytesize == 8:
            self.cflag = self.cflag | TERMIOS.CS8
        elif bytesize == 7:
            self.cflag = self.cflag | TERMIOS.CS7
        elif bytesize == 6:
            self.cflag = self.cflag | TERMIOS.CS6
        elif bytesize == 5:
            self.cflag = self.cflag | TERMIOS.CS5
        else:
            raise ValueError,'invalid char len: '+str(clen)
        #setup stopbits
        if stopbits == STOPBITS_ONE:
            self.cflag = self.cflag & ~(TERMIOS.CSTOPB)
        elif stopbits == STOPBITS_TWO:
            self.cflag = self.cflag |  (TERMIOS.CSTOPB)
        else:
            raise ValueError,'invalid stopit specification:'+str(stopbits)
        #setup parity
        self.iflag = self.iflag & ~(TERMIOS.INPCK|TERMIOS.ISTRIP)
        if parity == PARITY_NONE:
            self.cflag = self.cflag & ~(TERMIOS.PARENB|TERMIOS.PARODD)
        elif parity == PARITY_EVEN:
            self.cflag = self.cflag & ~(TERMIOS.PARODD)
            self.cflag = self.cflag |  (TERMIOS.PARENB)
        elif parity == PARITY_ODD:
            self.cflag = self.cflag |  (TERMIOS.PARENB|TERMIOS.PARODD)
        else:
            raise ValueError,'invalid parity: '+str(par)
        #setup flow control
        #xonxoff
        if hasattr(TERMIOS, 'IXANY'):
            if xonxoff:
                self.iflag = self.iflag |  (TERMIOS.IXON|TERMIOS.IXOFF|TERMIOS.IXANY)
            else:
                self.iflag = self.iflag & ~(TERMIOS.IXON|TERMIOS.IXOFF|TERMIOS.IXANY)
        else:
            if xonxoff:
                self.iflag = self.iflag |  (TERMIOS.IXON|TERMIOS.IXOFF)
            else:
                self.iflag = self.iflag & ~(TERMIOS.IXON|TERMIOS.IXOFF)
        #rtscts
        if hasattr(TERMIOS, 'CRTSCTS'):
            if rtscts:
                self.cflag = self.cflag |  (TERMIOS.CRTSCTS)
            else:
                self.cflag = self.cflag & ~(TERMIOS.CRTSCTS)
        elif hasattr(TERMIOS, 'CNEW_RTSCTS'):   #try it with alternate constant name
            if rtscts:
                self.cflag = self.cflag |  (TERMIOS.CNEW_RTSCTS)
            else:
                self.cflag = self.cflag & ~(TERMIOS.CNEW_RTSCTS)
        #XXX should there be a warning if setting up rtscts (and xonxoff etc) fails??
        
        #buffer
        #vmin "minimal number of characters to be read. = for non blocking"
        if vmin<0 or vmin>255:
            raise ValueError,'invalid vmin: '+str(vmin)
        self.cc[TERMIOS.VMIN] = vmin
        #vtime
        if vtime<0 or vtime>255:
            raise ValueError,'invalid vtime: '+str(vtime)
        self.cc[TERMIOS.VTIME] = vtime
        #activate settings
        self.__tcsetattr()

    def __tcsetattr(self):
        """internal function to set port attributes"""
        termios.tcsetattr(self.fd, TERMIOS.TCSANOW, [self.iflag,self.oflag,self.cflag,self.lflag,self.ispeed,self.ospeed,self.cc])

    def __tcgetattr(self):
        """internal function to get port attributes"""
        self.iflag,self.oflag,self.cflag,self.lflag,self.ispeed,self.ospeed,self.cc = termios.tcgetattr(self.fd)

    def close(self):
        """close port"""
        if self.fd:
            os.close(self.fd)
            self.fd = None

    def setBaudrate(self, baudrate):
        """change baudrate after port is open"""
        if not self.fd: raise portNotOpenError
        self.__tcgetattr()  #read current settings
        #setup baudrate
        try:
            self.ispeed = self.ospeed = baudIntToEnum[baudrate]
        except:
            raise ValueError,'invalid baud rate: %s' % baudrate
        self.__tcsetattr()

    def inWaiting(self):
        """how many character are in the input queue"""
        #~ s = fcntl.ioctl(self.fd, TERMIOS.FIONREAD, TIOCM_zero_str)
        s = fcntl.ioctl(self.fd, TIOCINQ, TIOCM_zero_str)
        return struct.unpack('I',s)[0]

    def write(self, data):
        """write a string to the port"""
        if not self.fd: raise portNotOpenError
        t = len(data)
        d = data
        while t>0:
            n = os.write(self.fd, d)
            d = d[n:]
            t = t - n

    def read(self, size=1):
        """read a number of bytes from the port.
        the default is one (unlike files)"""
        if not self.fd: raise portNotOpenError
        read = ''
        inp = None
        if size > 0:
            while len(read) < size:
                #print "\tread(): size",size, "have", len(read)    #debug
                ready,_,_ = select.select([self.fd],[],[], self.timeout)
                if not ready:
                    break   #timeout
                buf = os.read(self.fd, size-len(read))
                read = read + buf
                if self.timeout >= 0 and not buf:
                    break  #early abort on timeout
        return read

    def flushInput(self):
        """clear input queue"""
        if not self.fd:
            raise portNotOpenError
        termios.tcflush(self.fd, TERMIOS.TCIFLUSH)

    def flushOutput(self):
        """flush output"""
        if not self.fd:
            raise portNotOpenError
        termios.tcflush(self.fd, TERMIOS.TCOFLUSH)

    def sendBreak(self):
        """send break signal"""
        if not self.fd:
            raise portNotOpenError
        termios.tcsendbreak(self.fd, 0)

    def drainOutput(self):
        """internal - not portable!"""
        if not self.fd: raise portNotOpenError
        termios.tcdrain(self.fd)

    def nonblocking(self):
        """internal - not portable!"""
        if not self.fd:
            raise portNotOpenError
        fcntl.fcntl(self.fd, FCNTL.F_SETFL, FCNTL.O_NONBLOCK)

    def getDSR(self):
        """read terminal status line"""
        if not self.fd: raise portNotOpenError
        s = fcntl.ioctl(self.fd, TIOCMGET, TIOCM_zero_str)
        return struct.unpack('I',s)[0] & TIOCM_DSR

    def getCD(self):
        """read terminal status line"""
        if not self.fd: raise portNotOpenError
        s = fcntl.ioctl(self.fd, TIOCMGET, TIOCM_zero_str)
        return struct.unpack('I',s)[0] & TIOCM_CD

    def getRI(self):
        """read terminal status line"""
        if not self.fd: raise portNotOpenError
        s = fcntl.ioctl(self.fd, TIOCMGET, TIOCM_zero_str)
        return struct.unpack('I',s)[0] & TIOCM_RI

    def getCTS(self):
        """read terminal status line"""
        if not self.fd: raise portNotOpenError
        s = fcntl.ioctl(self.fd, TIOCMGET, TIOCM_zero_str)
        return struct.unpack('I',s)[0] & TIOCM_CTS

    def setDTR(self,on=1):
        """set terminal status line"""
        if not self.fd: raise portNotOpenError
        if on:
            fcntl.ioctl(self.fd, TIOCMBIS, TIOCM_DTR_str)
        else:
            fcntl.ioctl(self.fd, TIOCMBIC, TIOCM_DTR_str)

    def setRTS(self,on=1):
        """set terminal status line"""
        if not self.fd: raise portNotOpenError
        if on:
            fcntl.ioctl(self.fd, TIOCMBIS, TIOCM_RTS_str)
        else:
            fcntl.ioctl(self.fd, TIOCMBIC, TIOCM_RTS_str)

if __name__ == '__main__':
    s = Serial(0,
                 baudrate=19200,        #baudrate
                 bytesize=EIGHTBITS,    #number of databits
                 parity=PARITY_EVEN,    #enable parity checking
                 stopbits=STOPBITS_ONE, #number of stopbits
                 timeout=3,             #set a timeout value, None for waiting forever
                 xonxoff=0,             #enable software flow control
                 rtscts=0,              #enable RTS/CTS flow control
               )
    s.setRTS(1)
    s.setDTR(1)
    s.flushInput()
    s.flushOutput()
    s.write('hello')
    print repr(s.read(5))
    print s.inWaiting()
    del s

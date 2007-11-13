#!jython
#module for serial IO for Jython and JavaComm
#see __init__.py
#
#(C) 2002 Chris Liechti <cliechti@gmx.net>
# this is distributed under a free software license, see license.txt

import sys, os, string, javax.comm
import serialutil

VERSION = string.split("$Revision: 1.1 $")[1]     #extract CVS version

PARITY_NONE, PARITY_EVEN, PARITY_ODD, PARITY_MARK, PARITY_SPACE = (0,1,2,3,4)
STOPBITS_ONE, STOPBITS_TWO, STOPBITS_ONE_HALVE = (1, 2, 3)
FIVEBITS, SIXBITS, SEVENBITS, EIGHTBITS = (5,6,7,8)


portNotOpenError = ValueError('port not open')

def device(portnumber):
    enum = javax.comm.CommPortIdentifier.getPortIdentifiers()
    ports = []
    while enum.hasMoreElements():
        el = enum.nextElement()
        if el.getPortType() == javax.comm.CommPortIdentifier.PORT_SERIAL:
            ports.append(el)
    return ports[portnumber]

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

        if type(port) == type(''):      #strings are taken directly
            portId = javax.comm.CommPortIdentifier.getPortIdentifier(port)
        else:
            portId = device(port)     #numbers are transformed to a comportid obj
        self.portstr = portId.getName()
        try:
            self.sPort = portId.open("python serial module", 10)
        except Exception, msg:
            self.sPort = None
            raise serialutil.SerialException, "could not open port: %s" % msg
        self.instream = self.sPort.getInputStream()
        self.outstream = self.sPort.getOutputStream()
        self.sPort.enableReceiveTimeout(30)
        if bytesize == FIVEBITS:
            self.databits = javax.comm.SerialPort.DATABITS_5
        elif bytesize == SIXBITS:
            self.databits = javax.comm.SerialPort.DATABITS_6
        elif bytesize == SEVENBITS:
            self.databits = javax.comm.SerialPort.DATABITS_7
        elif bytesize == EIGHTBITS:
            self.databits = javax.comm.SerialPort.DATABITS_8
        else:
            raise ValueError, "unsupported bytesize"
        
        if stopbits == STOPBITS_ONE:
            self.jstopbits = javax.comm.SerialPort.STOPBITS_1
        elif stopbits == STOPBITS_ONE_HALVE:
            self.jstopbits = javax.comm.SerialPort.STOPBITS_1_5
        elif stopbits == STOPBITS_TWO:
            self.jstopbits = javax.comm.SerialPort.STOPBITS_2
        else:
            raise ValueError, "unsupported number of stopbits"

        if parity == PARITY_NONE:
            self.jparity = javax.comm.SerialPort.PARITY_NONE
        elif parity == PARITY_EVEN:
            self.jparity = javax.comm.SerialPort.PARITY_EVEN
        elif parity == PARITY_ODD:
            self.jparity = javax.comm.SerialPort.PARITY_ODD
        elif parity == PARITY_MARK:
            self.jparity = javax.comm.SerialPort.PARITY_MARK
        elif parity == PARITY_SPACE:
            self.jparity = javax.comm.SerialPort.PARITY_SPACE
        else:
            raise ValueError, "unsupported parity type"

        jflowin = jflowout = 0
        if rtscts:
            jflowin = jflowin | javax.comm.SerialPort.FLOWCONTROL_RTSCTS_IN 
            jflowout = jflowout | javax.comm.SerialPort.FLOWCONTROL_RTSCTS_OUT
        if xonxoff:
            jflowin = jflowin | javax.comm.SerialPort.FLOWCONTROL_XONXOFF_IN
            jflowout = jflowout | javax.comm.SerialPort.FLOWCONTROL_XONXOFF_OUT
        
        self.sPort.setSerialPortParams(baudrate, self.databits, self.jstopbits, self.jparity)
        self.sPort.setFlowControlMode(jflowin | jflowout)
        
        self.timeout = timeout
        if timeout >= 0:
            self.sPort.enableReceiveTimeout(timeout*1000)
        else:
            self.sPort.disableReceiveTimeout()

    def close(self):
        if self.sPort:
            self.instream.close()
            self.outstream.close()
            self.sPort.close()
            self.sPort = None

    def setBaudrate(self, baudrate):
        """change baudrate after port is open"""
        if not self.sPort: raise portNotOpenError
        self.sPort.setSerialPortParams(baudrate, self.databits, self.jstopbits, self.jparity)


    def inWaiting(self):
        if not self.sPort: raise portNotOpenError
        return self.instream.available()

    def write(self, data):
        if not self.sPort: raise portNotOpenError
        self.outstream.write(data)

    def read(self, size=1):
        if not self.sPort: raise portNotOpenError
        read = ''
        if size > 0:
            while len(read) < size:
                x = self.instream.read()
                if x == -1:
                    if self.timeout >= 0:
                        break
                else:
                    read = read + chr(x)
        return read

    def flushInput(self):
        if not self.sPort: raise portNotOpenError
        self.instream.skip(self.instream.available())

    def flushOutput(self):
        if not self.sPort: raise portNotOpenError
        self.outstream.flush()

    def sendBreak(self):
        if not self.sPort: raise portNotOpenError
        self.sPort.sendBreak()

    def getDSR(self):
        if not self.sPort: raise portNotOpenError
        self.sPort.isDSR()

    def getCD(self):
        if not self.sPort: raise portNotOpenError
        self.sPort.isCD()

    def getRI(self):
        if not self.sPort: raise portNotOpenError
        self.sPort.isRI()

    def getCTS(self):
        if not self.sPort: raise portNotOpenError
        self.sPort.isCTS()

    def setDTR(self,on=1):
        if not self.sPort: raise portNotOpenError
        self.sPort.setDTR(on)

    def setRTS(self,on=1):
        if not self.sPort: raise portNotOpenError
        self.sPort.setRTS(on)

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




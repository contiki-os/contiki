#!/usr/bin/env python
#Parallel JTAG programmer for the MSP430 embedded proccessor.
#
#(C) 2002 Chris Liechti <cliechti@gmx.net>
#this is distributed under a free software license, see license.txt
#
#Requires Python 2+ and the binary extension _parjtag.

import sys
import _parjtag

VERSION = "1.3"

DEBUG = 0                           #disable debug messages by default


#frame specific consts
ERASE_MASS  = 2
ERASE_MAIN  = 1
ERASE_SGMT  = 0

#states
FREERUNNING = 0
STOPPED     = 1

#Configurations of the MSP430 driver
VERIFICATION_MODE = 0  #Verify data downloaded to FLASH memories.
RAMSIZE_OPTION    = 1  #Change RAM used to download and program flash blocks
DEBUG_OPTION      = 2  #Set debug level. Enables debug outputs.

#enumeration of output formats for uploads
HEX             = 0
INTELHEX        = 1
BINARY          = 2

#exceptions
class JTAGException(Exception): pass

#for the use with memread
def hexdump( (adr, memstr) ):
    """Print a hex dump of data collected with memread
    arg1: tuple with adress, memory
    return None"""
    count = 0
    ascii = ''
    for value in map(ord, memstr):
        if not count: print "%04x: " % adr,
        print "%02x" % value,
        ascii += (32 <= value < 128) and chr(value) or '.'
        count += 1
        adr += 1
        if count == 16:
            count = 0
            print "  ", ascii
            ascii = ''
    if count < 16: print "   "*(16-count), " ", ascii

def makeihex( (address, data) ):
    """work though the data and output lines in inzel hex format.
    and end tag is appended"""
    start = 0
    while start<len(data):
        end = start + 16
        if end > len(data): end = len(data)
        _ihexline(address, [ord(x) for x in data[start:end]])
        start += 16
        address += 16
    _ihexline(address, [], type=1)   #append no data but an end line

def _ihexline(address, buffer, type=0):
    """encode one line, output with checksum"""
    sys.stdout.write( ':%02X%04X%02X' % (len(buffer), address & 0xffff, type) )
    sum = len(buffer) + ((address >> 8) & 255) + (address & 255)
    for b in buffer:
        if b == None: b = 0         #substitute nonexistent values with zero
        sys.stdout.write('%02X' % (b & 255))
        sum += b&255
    sys.stdout.write('%02X\n' %( (-sum) & 255))


class Segment:
    """store a string with memory contents along with its startaddress"""
    def __init__(self, startaddress = 0, data=None):
        if data is None:
            self.data = ''
        else:
            self.data = data
        self.startaddress = startaddress

    def __getitem__(self, index):
        return self.data[index]

    def __len__(self):
        return len(self.data)

    def __repr__(self):
        return "Segment(startaddress = 0x%04x, data=%r)" % (self.startaddress, self.data)

class Memory:
    """represent memory contents. with functions to load files"""
    def __init__(self, filename=None):
        self.segments = []
        if filename:
            self.filename = filename
            self.loadFile(filename)

    def append(self, seg):
        self.segments.append(seg)

    def __getitem__(self, index):
        return self.segments[index]

    def __len__(self):
        return len(self.segments)

    def loadIHex(self, file):
        """load data from a (opened) file in Intel-HEX format"""
        segmentdata = []
        currentAddr = 0
        startAddr   = 0
        lines = file.readlines()
        for l in lines:
            if not l.strip(): continue  #skip empty lines
            if l[0] != ':': raise Exception("File Format Error\n")
            l = l.strip()               #fix CR-LF issues...
            length  = int(l[1:3],16)
            address = int(l[3:7],16)
            type    = int(l[7:9],16)
            check   = int(l[-2:],16)
            if type == 0x00:
                if currentAddr != address:
                    if segmentdata:
                        self.segments.append( Segment(startAddr, ''.join(segmentdata)) )
                    startAddr = currentAddr = address
                    segmentdata = []
                for i in range(length):
                    segmentdata.append( chr(int(l[9+2*i:11+2*i],16)) )
                currentAddr = length + currentAddr
            elif type == 0x01:
                pass
            else:
                sys.stderr.write("Ignored unknown field (type 0x%02x) in ihex file.\n" % type)
        if segmentdata:
            self.segments.append( Segment(startAddr, ''.join(segmentdata)) )

    def loadTIText(self, file):
        """load data from a (opened) file in TI-Text format"""
        next        = 1
        currentAddr = 0
        startAddr   = 0
        segmentdata = []
        #Convert data for MSP430, TXT-File is parsed line by line
        while next >= 1:
            #Read one line
            l = file.readline()
            if not l: break #EOF
            l = l.strip()
            if l[0] == 'q': break
            elif l[0] == '@':        #if @ => new address => send frame and set new addr.
                #create a new segment
                if segmentdata:
                    self.segments.append( Segment(startAddr, ''.join(segmentdata)) )
                startAddr = currentAddr = int(l[1:],16)
                segmentdata = []
            else:
                for i in l.split():
                    segmentdata.append(chr(int(i,16)))
        if segmentdata:
            self.segments.append( Segment(startAddr, ''.join(segmentdata)) )

    def loadELF(self, file):
        """load data from a (opened) file in ELF object format.
        File must be seekable"""
        import elf
        obj = elf.ELFObject()
        obj.fromFile(file)
        if obj.e_type != elf.ELFObject.ET_EXEC:
            raise Exception("No executable")
        for section in obj.getSections():
            if DEBUG:
                sys.stderr.write("ELF section %s at 0x%04x %d bytes\n" % (section.name, section.lma, len(section.data)))
            if len(section.data):
                self.segments.append( Segment(section.lma, section.data) )
        
    def loadFile(self, filename):
        """fill memory with the contents of a file. file type is determined from extension"""
        #TODO: do a contents based detection
        if filename[-4:].lower() == '.txt':
            self.loadTIText(open(filename, "rb"))
        elif filename[-4:].lower() in ('.a43', '.hex'):
            self.loadIHex(open(filename, "rb"))
        else:
            self.loadELF(open(filename, "rb"))

    def getMemrange(self, fromadr, toadr):
        """get a range of bytes from the memory. unavailable values are filled with 0xff."""
        res = ''
        toadr = toadr + 1   #python indxes are excluding end, so include it
        while fromadr < toadr:
            for seg in self.segments:
                segend = seg.startaddress + len(seg.data)
                if seg.startaddress <= fromadr and fromadr < segend:
                    if toadr > segend:   #not all data in segment
                        catchlength = segend-fromadr
                    else:
                        catchlength = toadr-fromadr
                    res = res + seg.data[fromadr-seg.startaddress : fromadr-seg.startaddress+catchlength]
                    fromadr = fromadr + catchlength    #adjust start
                    if len(res) >= toadr-fromadr:
                        break   #return res
            else:   #undefined memory is filled with 0xff
                    res = res + chr(255)
                    fromadr = fromadr + 1 #adjust start
        return res

class JTAG:
    """wrap the _parjtag extension"""

    def __init__(self):
        self.showprogess = 0
        
    def connect(self, lpt=None):
        """connect to specified or default port"""
        if lpt is None:
            _parjtag.connect()
        else:
            _parjtag.connect(lpt)

    def close(self):
        """release JTAG"""
        _parjtag.release()

    def uploadData(self, startaddress, size):
        """upload a datablock"""
        if DEBUG > 1: sys.stderr.write("* uploadData()\n")
        return _parjtag.memread(startaddress, size)

    def actionMassErase(self):
        """Erase the flash memory completely (with mass erase command)"""
        sys.stderr.write("Mass Erase...\n")
        _parjtag.memerase(ERASE_MASS)

    def actionMainErase(self):
        """Erase the MAIN flash memory, leave the INFO mem"""
        sys.stderr.write("Erase Main Flash...\n")
        _parjtag.memerase(ERASE_MAIN, 0xfffe)

    def makeActionSegmentErase(self, address):
        """Selective segment erase"""
        class SegmentEraser:
            def __init__(self, segaddr):
                self.address = segaddr
            def __call__(self):
                sys.stderr.write("Erase Segment @ 0x%04x...\n" % self.address)
                _parjtag.memerase(ERASE_SGMT, self.address)
        return SegmentEraser(address)

    def actionEraseCheck(self):
        """check the erasure of required flash cells."""
        sys.stderr.write("Erase Check by file ...\n")
        if self.data is not None:
            for seg in self.data:
                data = _parjtag.memread(seg.startaddress, len(seg.data))
                if data != '\xff'*len(seg.data): raise JTAGException("Erase check failed")
        else:
            raise JTAGException("cannot do erase check against data with not knowing the actual data")

    def progess_update(self, count, total):
        sys.stderr.write("\r%d%%" % (100*count/total))
        
    def actionProgram(self):
        """program data into flash memory."""
        if self.data is not None:
            sys.stderr.write("Program ...\n")
            if self.showprogess:
                _parjtag.set_flash_callback(self.progess_update)
            bytes = 0
            for seg in self.data:
                _parjtag.memwrite(seg.startaddress, seg.data)
                bytes += len(seg.data)
            if self.showprogess:
                sys.stderr.write("\r")
            sys.stderr.write("%i bytes programmed.\n" % bytes)
        else:
            raise JTAGException("programming without data not possible")

    def actionVerify(self):
        """Verify programmed data"""
        if self.data is not None:
            sys.stderr.write("Verify ...\n")
            for seg in self.data:
                data = _parjtag.memread(seg.startaddress, len(seg.data))
                if data != seg.data: raise JTAGException("Verify failed")
        else:
            raise JTAGException("verify without data not possible")

    def actionReset(self):
        """perform a reset"""
        sys.stderr.write("Reset device ...\n")
        _parjtag.reset(0, 0)

    def actionRun(self, address):
        """start program at specified address"""
        raise NotImplementedError
        #sys.stderr.write("Load PC with 0x%04x ...\n" % address)

    def funclet(self):
        """download and start funclet"""
        sys.stderr.write("Download and execute of funclet...\n")
        if len(self.data) > 1:
            raise JTAGException("don't know how to handle multiple segments in funclets")
        _parjtag.funclet(self.data[0].data)
        sys.stderr.write("Funclet OK.\n")

def usage():
    """print some help message"""
    sys.stderr.write("""
USAGE: %s [options] [file]
Version: %s

If "-" is specified as file the data is read from the stdinput.
A file ending with ".txt" is considered to be in TIText format all
other filenames are considered IntelHex.

General options:
  -h, --help            Show this help screen.
  -l, --lpt=name        Specify an other parallel port.
                        (defaults to LPT1 (/dev/parport0 on unix)
  -D, --debug           Increase level of debug messages. This won't be
                        very useful for the average user...
  -I, --intelhex        Force fileformat to IntelHex
  -T, --titext          Force fileformat to be TIText
  -f, --funclet         The given file is a funclet (a small program to
                        be run in RAM)
  -R, --ramsize         Specify the amont of RAM to be used to program
                        flash (default 256).

Program Flow Specifiers:

  -e, --masserase       Mass Erase (clear all flash memory)
  -m, --mainerase       Erase main flash memory only
  --eraseinfo           Erase info flash memory only (0x1000-0x10ff)
  --erase=address       Selectively erase segment at the specified address
  -E, --erasecheck      Erase Check by file
  -p, --program         Program file
  -v, --verify          Verify by file

The order of the above options matters! The table is ordered by normal
execution order. For the options "Epv" a file must be specified.
Program flow specifiers default to "p" if a file is given.
Don't forget to specify "e" or "eE" when programming flash!
"p" already verifies the programmed data, "v" adds an additional
verification though uploading the written data for a 1:1 compare.
No default action is taken if "p" and/or "v" is given, say specifying
only "v" does a check by file of a programmed device.

Data retreiving:
  -u, --upload=addr     Upload a datablock (see also: -s).
  -s, --size=num        Size of the data block do upload. (Default is 2)
  -x, --hex             Show a hexadecimal display of the uploaded data.
                        (Default)
  -b, --bin             Get binary uploaded data. This can be used
                        to redirect the output into a file.
  -i, --ihex            Uploaded data is output in Intel HEX format.
                        This can be used to clone a device.

Do before exit:
  -g, --go=address      Start programm execution at specified address.
                        This implies option "w" (wait)
  -r, --reset           Reset connected MSP430. Starts application.
                        This is a normal device reset and will start
                        the programm that is specified in the reset
                        interrupt vector. (see also -g)
  -w, --wait            Wait for <ENTER> before closing parallel port.
""" % (sys.argv[0], VERSION))

def main():
    global DEBUG
    import getopt
    filetype    = None
    filename    = None
    reset       = 0
    wait        = 0
    goaddr      = None
    jtag        = JTAG()
    toinit      = []
    todo        = []
    startaddr   = None
    size        = 2
    outputformat= HEX
    lpt         = None
    funclet     = None
    ramsize     = None

    sys.stderr.write("MSP430 parallel JTAG programmer Version: %s\n" % VERSION)
    try:
        opts, args = getopt.getopt(sys.argv[1:],
            "hl:weEmpvrg:Du:d:s:xbiITfR:S",
            ["help", "lpt=", "wait"
             "masserase", "erasecheck", "mainerase", "program",
             "erase=", "eraseinfo",
             "verify", "reset", "go=", "debug",
             "upload=", "download=", "size=", "hex", "bin", "ihex",
             "intelhex", "titext", "funclet", "ramsize=", "progress"]
        )
    except getopt.GetoptError:
        # print help information and exit:
        usage()
        sys.exit(2)

    for o, a in opts:
        if o in ("-h", "--help"):
            usage()
            sys.exit()
        elif o in ("-l", "--lpt"):
            lpt = a
        elif o in ("-w", "--wait"):
            wait = 1
        elif o in ("-e", "--masserase"):
            toinit.append(jtag.actionMassErase)         #Erase Flash
        elif o in ("-E", "--erasecheck"):
            toinit.append(jtag.actionEraseCheck)        #Erase Check (by file)
        elif o in ("-m", "--mainerase"):
            toinit.append(jtag.actionMainErase)         #Erase main Flash
        elif o == "--erase":
            try:
                seg = int(a, 0)
                toinit.append(jtag.makeActionSegmentErase(seg))
            except ValueError:
                sys.stderr.write("segment address must be a valid number in dec, hex or octal\n")
                sys.exit(2)
        elif o == "--eraseinfo":
            toinit.append(jtag.makeActionSegmentErase(0x1000))
            toinit.append(jtag.makeActionSegmentErase(0x1080))
        elif o in ("-p", "--program"):
            todo.append(jtag.actionProgram)             #Program file
        elif o in ("-v", "--verify"):
            todo.append(jtag.actionVerify)              #Verify file
        elif o in ("-r", "--reset"):
            reset = 1
        elif o in ("-g", "--go"):
            try:
                goaddr = int(a, 0)                      #try to convert decimal
            except ValueError:
                sys.stderr.write("upload address must be a valid number in dec, hex or octal\n")
                sys.exit(2)
        elif o in ("-D", "--debug"):
            DEBUG = DEBUG + 1
        elif o in ("-u", "--upload"):
            try:
                startaddr = int(a, 0)                   #try to convert number of any base
            except ValueError:
                sys.stderr.write("upload address must be a valid number in dec, hex or octal\n")
                sys.exit(2)
        elif o in ("-s", "--size"):
            try:
                size = int(a, 0)
            except ValueError:
                sys.stderr.write("upload address must be a valid number in dec, hex or octal\n")
                sys.exit(2)
        #outut formats
        elif o in ("-x", "--hex"):
            outputformat = HEX
        elif o in ("-b", "--bin"):
            outputformat = BINARY
        elif o in ("-i", "--ihex"):
            outputformat = INTELHEX
        #input formats
        elif o in ("-I", "--intelhex"):
            filetype = 0
        elif o in ("-T", "--titext"):
            filetype = 1
        #others
        elif o in ("-f", "--funclet"):
            funclet = 1
        elif o in ("-R", "--ramsize"):
            try:
                ramsize = int(a, 0)
            except ValueError:
                sys.stderr.write("ramsize must be a valid number in dec, hex or octal\n")
                sys.exit(2)
        elif o in ("-S", "--progress"):
            jtag.showprogess = 1

    if len(args) == 0:
        sys.stderr.write("Use -h for help\n")
    elif len(args) == 1:                                #a filename is given
        if not funclet:
            if not todo:                                #if there are no actions yet
                todo.extend([                           #add some useful actions...
                    jtag.actionProgram,
                ])
        filename = args[0]
    else:                                               #number of args is wrong
        usage()
        sys.exit(2)

    if DEBUG:   #debug infos
        sys.stderr.write("debug level set to %d\n" % DEBUG)
        _parjtag.configure(DEBUG_OPTION, DEBUG)
        sys.stderr.write("python version: %s\n" % sys.version)


    #sanity check of options
    if goaddr and reset:
        sys.stderr.write("Warning: option --reset ignored as --go is specified!\n")
        reset = 0

    if startaddr and reset:
        sys.stderr.write("Warning: option --reset ignored as --upload is specified!\n")
        reset = 0

    #prepare data to download
    jtag.data = Memory()                                #prepare downloaded data
    if filetype is not None:                            #if the filetype is given...
        if filename is None:
            raise ValueError("no filename but filetype specified")
        if filename == '-':                             #get data from stdin
            file = sys.stdin
        else:
            file = open(filename,"rb")                  #or from a file
        if filetype == 0:                               #select load function
            jtag.data.loadIHex(file)                    #intel hex
        elif filetype == 1:
            jtag.data.loadTIText(file)                  #TI's format
        else:
            raise ValueError("illegal filetype specified")
    else:                                               #no filetype given...
        if filename == '-':                             #for stdin:
            jtag.data.loadIHex(sys.stdin)               #assume intel hex
        elif filename:
            jtag.data.loadFile(filename)                #autodetect otherwise

    if DEBUG > 5: sys.stderr.write("File: %r\n" % filename)

    try:
        jtag.connect(lpt)                               #try to open port
    except IOError:
        raise                                           #do not handle here
    else:                                               #continue if open was successful
        if ramsize is not None:
            _parjtag.configure(RAMSIZE_OPTION, ramsize)
        #initialization list
        if toinit:  #erase and erase check
            if DEBUG: sys.stderr.write("Preparing device ...\n")
            for f in toinit: f()

        #work list
        if todo:
            if DEBUG > 0:       #debug
                #show a nice list of sheduled actions
                sys.stderr.write("TODO list:\n")
                for f in todo:
                    try:
                        sys.stderr.write("   %s\n" % f.func_name)
                    except AttributeError:
                        sys.stderr.write("   %r\n" % f)
            for f in todo: f()                          #work through todo list

        if reset:                                       #reset device first if desired
            jtag.actionReset()

        if funclet is not None:                         #download and start funclet
            jtag.funclet()

        if goaddr is not None:                          #start user programm at specified address
            jtag.actionRun(goaddr)                      #load PC and execute

        #upload  datablock and output
        if startaddr is not None:
            if goaddr:                                  #if a program was started...
                raise NotImplementedError
                #TODO:
                #sys.stderr.write("Waiting to device for reconnect for upload: ")
            data = jtag.uploadData(startaddr, size)     #upload data
            if outputformat == HEX:                     #depending on output format
                hexdump( (startaddr, data) )            #print a hex display
            elif outputformat == INTELHEX:
                makeihex( (startaddr, data) )           #ouput a intel-hex file
            else:
                sys.stdout.write(data)                  #binary output w/o newline!
            wait = 0    #wait makes no sense as after the upload the device is still stopped

        if wait:                                        #wait at the end if desired
            sys.stderr.write("Press <ENTER> ...\n")     #display a prompt
            raw_input()                                 #wait for newline
        
        _parjtag.reset(1, 1)                            #reset and release target
        #~ jtag.actionReset()
        jtag.close()                                    #Release communication port

if __name__ == '__main__':
    try:
        main()
    except SystemExit:
        raise                                           #let pass exit() calls
    except KeyboardInterrupt:
        if DEBUG: raise                                 #show full trace in debug mode
        sys.stderr.write("user abort.\n")               #short messy in user mode
        sys.exit(1)                                     #set errorlevel for script usage
    except Exception, msg:                              #every Exception is caught and displayed
        if DEBUG: raise                                 #show full trace in debug mode
        sys.stderr.write("\nAn error occoured:\n%s\n" % msg) #short messy in user mode
        sys.exit(1)                                     #set errorlevel for script usage    

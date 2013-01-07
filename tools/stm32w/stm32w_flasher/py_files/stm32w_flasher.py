#!/usr/bin/env python
import optparse, os, sys, time

# The binary file at [contiki]/tools/stm32w/stm32w_flasher/stm32w_flasher
# contained python code.
# This binary file did not work anymore on Ubuntu 12.04, so 
# it was replaced by this extracted python code, as discussed
# with people at ST Crolles (France).
#
# Extraction and little adaptation performed by E.Duble (CNRS, LIG).

try:
    import serial
    import ftdi
except:
    print 'Python modules serial and ftdi could not be loaded.'
    print 'Please install these dependencies:'
    print '(On Ubuntu) $ sudo apt-get install python-serial python-ftdi'
    sys.exit(-1)

from messages import infoMessage, errorMessage
from rs232_interface import rs232Interface, getFirstAvailableSerialPort

versionMajor = 2
versionMinor = 0
versionPatch = 1
versionQualifier = "b2"
version = "%d.%d.%d%s"%(versionMajor, versionMinor, versionPatch, versionQualifier)

commandTable = [
        ['-i', '--interface', 'store', "<rs232 | rf > specify flasher interface (default rs232)",None],
        ['-p', '--port',      'store', "</dev/ttyXXX | auto>: specify rs232 port or auto mode (only relevant if interface selected is rs232 or rf)", None],
        ['-f', '--flash-image', 'store_true', "program flash"],
        ['-a', '--address',    'store', "0xHHHHHHHH specify baseaddress (only relevat for .bin images)", "0x08000000"],
        ['-v', '--verify', 'store_true', "verify flash content against file", False],
        #['-d', '--device-info', 'store_true', 'print information about the STM32W device', False],
        ['-r', '--reset', 'store_true', 'Reset device', False],
        ['-s', '--start', 'store_true', 'Start application', False],
        ['-m', '--masserase', 'store_true', 'Erase user flash (masserase)', False],
        ['-b', '--bootloader-mode', 'store_true', 'Device is already in bootloader mode.(Only relevant for rs232 interface)', False],
        ['-e', '--eui64', 'store', "0xHHHHHHHHHHHHHHHH specify EUI64 (only relevat for rf interface)", "0xFFFFFFFFFFFFFFFF"],
        ['', '--enable-readout-protection', 'store', "<0|1> Disable or enable readout protection", None],
        ['', '--set-write-protection', 'store', "0xHHHHHHHH Set write protection to hex value (each bit set to zero is protecting 4K)" , "0xFFFFFFFF"],
    ]

def initCommandLineParser():
    """Create a command-line parser and return it.
    """
    USAGE = '%prog [option...] [filename{.s37|.bin}] ...'
    DESCR = """ Flash programming utility for STM32W108 device """

    parser = optparse.OptionParser(usage=USAGE, description=DESCR,
        version='%%prog %s' % version)

    for item in commandTable:
        if len(item) > 4:
            default = item[4]
        else:
            default = None
        parser.add_option(item[0], item[1], action=item[2], help=item[3], default=default)

    return parser

def getFileVersionDate(fname):
    returnValue = ""
    return returnValue

def initInterface (interface, port, noReset=False, rfMode=False, eui64=0):
    returnValue = rs232Interface(port, noReset, rfMode, eui64)
    if returnValue.init():
        returnValue = None
    return returnValue

def terminateInterface(interface):
    if (isinstance(interface, rs232Interface)):
        interface.terminate()

def flashImage (interface, inputFile, startAddress, doErase=True):
    def progressReport(size, fileSize):
        infoMessage ("Programming %05d/%05d\r"%(size,fileSize))
    infoMessage ("Programming user flash\n")
    if not interface.programUserFlash(inputFile, startAddress, progressReport, doErase):
        infoMessage("Failed                    \n")
    else:
        infoMessage("Done                      \n")

def verifyFlashImage (interface, inputFile, startAddress):
    def progressReport(size, fileSize):
        infoMessage ("Verifying %05d/%05d\r"%(size,fileSize))

    infoMessage ("Verifying user flash\n")
    if not interface.verifyFlash(inputFile, startAddress, progressReport):
        infoMessage("Failed                \n")
    else:
        infoMessage("Done                   \n")

def resetDevice (interface):
    infoMessage ("Resetting device\n")

    returnValue = not interface.startApplication(0)

    if (returnValue):
        infoMessage("Failed                \n")
    else:
        infoMessage("Done                   \n")

def startApplication (interface, startAddress):
    infoMessage ("Starting application from address: %08x\n"%startAddress)
    if not interface.startApplication(startAddress):
        infoMessage("Failed                \n")
    else:
        infoMessage("Done                   \n")

def eraseUserFlash (interface):
    infoMessage ("Erasing user flash (mass erase)\n")
    if not interface.eraseUserFlash():
        infoMessage("Failed                \n")
    else:
        infoMessage("Done                   \n")       

def printBanner():
    infoMessage ("STM32W flasher utility version %s\n"%version)

if __name__ == '__main__':
    
    try:
        absolutePath = os.path.abspath(sys.argv[0])
        if "frozen" in dir(sys):
            version += " " + getFileVersionDate(sys.executable)
        else:
            version += "-" + time.strftime("%Y%m%d",time.localtime(os.stat(absolutePath).st_ctime))
        
        version += " for Linux"

        interfaceHandle = None
        printBanner()

        parser = initCommandLineParser()
        (options, args) = parser.parse_args()
        rfMode = False

        # Check command line option
        if (len(args) == 1):
            if (len(sys.argv) == 2):
                # No option means flashing by default
                options.flash_image = True
            options.image = args[0]

        if (len(args) > 1):
            parser.print_help()
            sys.exit(-1)

        if (len(sys.argv) == 1):
            parser.print_help()
            sys.exit(0)

        if (options.interface is None):
            # Set defualt for interface
            if (options.port is None):
                options.interface = "rs232"
                options.port = "auto"
            else:
                options.interface = "rs232"
        elif (options.interface == "rf"):
            rfMode = True
            options.address = "0x08003000"
            options.interface = "rs232"
            options.bootloader_mode = True

        options.interface = options.interface.lower()

        if ((options.interface == "rs232") and (options.port == None)):
            parser.print_help()
            sys.exit(-1)

        if (options.interface != "rs232" and options.interface != "jlink"):
            parser.print_help()
            sys.exit(-1)

        startAddress = int(options.address,16)

        if (options.interface == "rs232"):
            optionArg = options.port.upper()
            if (optionArg == "AUTO"):
                port = getFirstAvailableSerialPort()
                if port is None:
                    errorMessage ("Unable to find serial port in auto mode: no STM32W boards detected\n")
                    sys.exit(-1)
                infoMessage ("Auto mode selected serial port: %s\n"%port)
            elif (options.port[:4] == "/dev"): ## For Linux
                port = options.port
            else:
                try:
                    port = int(optionArg)
                except ValueError:
                    errorMessage("Invalid port: %s\n"%options.port)
                    sys.exit(-1)
        else:
            port = None
            
        if (options.interface == "jlink"):
            errorMessage("JLink not yet supported.\n")
            sys.exit(-1)

        if (options.start and options.reset):
            errorMessage ("Only one option between -s (--start) and -r (--reset) may be specified")
            sys.exit(-1)

        interfaceHandle = initInterface(options.interface, port, options.bootloader_mode, rfMode, int(options.eui64, 16))

        if interfaceHandle is None:
            errorMessage("Error while initiliazing interface\n")
            sys.exit(-1)

        readoutProtection = interfaceHandle.isReadProtectionActive()

        if readoutProtection:
            infoMessage("!Warning: Readout protection is active\n")
            
        options.device_info = False
        
        if (options.masserase):
            eraseUserFlash(interfaceHandle)

        if (options.enable_readout_protection is not None):
            if (options.enable_readout_protection == "1"):
                if not readoutProtection:
                    infoMessage("Enabling readout protection\n")
                    interfaceHandle.enableReadProtection(True)
                    infoMessage("Done\n")
                else:
                    infoMessage("Readout protection already enabled, no action\n")
                    
            elif (options.enable_readout_protection == "0"):
                if readoutProtection:
                    infoMessage("Disabling readout protection (this will clear user flash and CIB)\n")
                    interfaceHandle.enableReadProtection(False)
                    infoMessage("Done\n")
                else:
                    infoMessage("Readout protection already disaabled, no action\n")
            else:
                errorMessage("Invalid value for --enable-readout-protection\n")
                terminateInterface(interfaceHandle)
                sys.exit(-1)

        if options.flash_image:
            flashImage(interfaceHandle, options.image, startAddress, not options.masserase)

        if options.verify and not readoutProtection:
            verifyFlashImage(interfaceHandle, options.image, startAddress)


        if (options.start):
            startApplication(interfaceHandle, startAddress)

        if (options.reset):
            resetDevice(interfaceHandle)

        terminateInterface(interfaceHandle)

        sys.exit (0)

    except KeyboardInterrupt:
        infoMessage ("User break\n")
        terminateInterface(interfaceHandle)
        sys.exit(-1)

    except Exception, inst:
        if not ("frozen" in dir(sys)):
            raise
        else:
            errorMessage("Internal error\n")
            errorMessage("%s\n"%repr(type(inst)))     # the exception instance
            errorMessage("%s\n"%repr(inst.args))      # arguments stored in .args
            if interfaceHandle:
                terminateInterface(interfaceHandle)
            sys.exit(-1)



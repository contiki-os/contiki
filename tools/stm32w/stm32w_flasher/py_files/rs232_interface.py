
# See comment in stm32w_flasher.py.
# Extraction and little adaptation performed by E.Duble (CNRS, LIG).

import ftdi
import os
import serial
import struct
import subprocess
import sys
import time
import ymodem
from messages import errorMessage
from file_utils import fileFormatReader
from messages import infoMessage
from messages import warningMessage

ACK = 121
BOOTLOADER_COMMAND_BAUDRATE = 50
CBUS_IOMODE = 10
FT232R_PRODUCT = 24577
FT232R_VENDOR = 1027
NAK = 31
STM32F103_PRODUCT = 22337
STM32F103_PRODUCT_OLD = 22336
STM32F103_VENDOR = 1155
StringTypes = None
TIMEOUT_TO_SWITCH_BETWEEN_APPLICATION_AND_BOOTLOADER = 10
commands = {'WRITE': [49, 3], 'GO': [33, 3], 'GET_ID': [2, 5], 'ERASE': [67, 2], 'GET': [0, 2], 'READ': [17, 259], 'WRITE_INCREMENTAL': [54, 2], 'GET_VERSION': [1, 5], 'READOUT_PROTECT': [130, 2], 'READOUT_UNPROTECT': [146, 2]}
pcIfHandle = None
rs232PortsList = None

class FTDI_Interface(object):
	def __init__(self, port):
		self.port = port
		self.resetValue = None
		self.nBootValue = None

	def close(self):
		return None

	def getFTDIIdFromComPort(self, port):
		returnValue = None
		deviceIDs = None
		mapping = self.mapComPortsToFTDIId()
		if deviceIDs == None:
			returnValue = (mapping[port])
		else:
			for id in deviceIDs:
				if (mapping[port]) == id:
					returnValue = id
					break
				else:
					continue
			if returnValue is None:
				for id in deviceIDs:
					if (mapping[port]).upper() == id.upper():
						returnValue = id
						break
					else:
						continue
		return returnValue

	def getFTDIIdFromDevPort(self, port):
		command = (('for udi in `hal-find-by-capability --capability serial`\n				do\n						parent=`hal-get-property --udi ${udi} --key "info.parent"`\n						device=`hal-get-property --udi ${udi} --key "linux.device_file"`\n						grandpa=`hal-get-property --udi ${parent} --key "info.parent"`\n						serial=`hal-get-property --udi ${grandpa} --key "usb_device.serial" 2>/dev/null`\n						if [ "${device}" = "' + port) + '" ]\n						then\n								printf "%s" "${serial}"\n								break\n						fi\n				done ')
		p = subprocess.Popen([command], shell=True, stdout=subprocess.PIPE)
		out = p.communicate()
		return (out[0])

	def guessResetDirection(self, h):
		if (self.resetValue is None or self.nBootValue is None):
			CBUS_2 = 4
			CBUS_0 = 1
			RESET_LINE = CBUS_2
			GPIO5_LINE = CBUS_0
			resetLow = ((RESET_LINE << 4) | 0)
			resetHigh = ((RESET_LINE << 4) | RESET_LINE)
			GPIO5Low = ((GPIO5_LINE << 4) | 0)
			GPIO5High = ((GPIO5_LINE << 4) | GPIO5_LINE)
			h.usb_write_timeout = 1000
			h.usb_read_timeout = 1000
			ret = ftdi.ftdi_set_baudrate(h, 115200)
			ret = (ret + ftdi.ftdi_setflowctrl(h, ftdi.SIO_DISABLE_FLOW_CTRL))
			ret = (ret + ftdi.ftdi_set_line_property(h, ftdi.BITS_8, ftdi.STOP_BIT_1, ftdi.NONE))
			for i in range(2):
				if (i % 2) == 0:
					resetValue = resetHigh
					nbootValue = GPIO5High
				else:
					resetValue = resetLow
					nbootValue = GPIO5Low
				ftdi.ftdi_set_bitmode(h, resetValue, ftdi.BITMODE_CBUS)
				time.sleep(0.10000000000000001)
				ftdi.ftdi_set_bitmode(h, nbootValue, ftdi.BITMODE_CBUS)
				time.sleep(0.10000000000000001)
				ftdi.ftdi_set_bitmode(h, 0, ftdi.BITMODE_CBUS)
				ftdi.ftdi_usb_purge_rx_buffer(h)
				ftdi.ftdi_usb_purge_tx_buffer(h)
				ftdi.ftdi_write_data(h, struct.pack('B', 127), 1)
				startTime = time.time()
				inbuff = '\x00'
				nbyte_rcvd = 0
				while (nbyte_rcvd == 0 and (time.time() - startTime) < 1):
					nbyte_rcvd = ftdi.ftdi_read_data(h, inbuff, 1)
					continue
				if nbyte_rcvd > 0:
					reply = struct.unpack('B', inbuff)
					if (reply[0]) == 121:
						self.resetValue = resetValue
						self.nBootValue = nbootValue
						break
					else:
						continue
				continue
		return (self.resetValue, self.nBootValue)

	def open(self):
		return 0

	def reset(self, bootloader=False):
		handle = ftdi.ftdi_context()
		if ftdi.ftdi_init(handle) < 0:
			print 'Initialization error.',
			print
			sys.exit(-1)
		serialnum = self.getFTDIIdFromDevPort(self.port)
		if subprocess.call(['rmmod', 'ftdi_sio']) != 0:
			print 'Close all processes that may access serial ports. You may also need to run the program as root.',
			print
			sys.exit(-1)
		error = ftdi.ftdi_usb_open_desc(handle, FT232R_VENDOR, FT232R_PRODUCT, None, serialnum)
		if error < 0:
			print ('Unable to open port. Error:' + str(error)),
			print
			subprocess.call(['modprobe', 'ftdi_sio'])
			sys.exit(-1)
		item0, item1 = self.guessResetDirection(handle)
		resetValue = item0
		nBootValue = item1
		if (resetValue, nBootValue) == (None, None):
			ftdi.ftdi_usb_close(handle)
			subprocess.call(['modprobe', 'ftdi_sio'])
			return 1
		else:
			ftdi.ftdi_set_bitmode(handle, resetValue, ftdi.BITMODE_CBUS)
			time.sleep(0.5)
			if bootloader:
				ftdi.ftdi_set_bitmode(handle, nBootValue, ftdi.BITMODE_CBUS)
				time.sleep(0.10000000000000001)
			ftdi.ftdi_set_bitmode(handle, 0, ftdi.BITMODE_CBUS)
			ftdi.ftdi_usb_close(handle)
			subprocess.call(['modprobe', 'ftdi_sio'])
			time.sleep(1)
			return None

	def setNBOOT(self, value=1):
		returnValue = True
		return returnValue


class STBL(object):
	def __init__(self, serialPort, inputFile=None, startAddress=None, verboseMode=None, packetSize=256, serialMode=True):
		self.serialPort = serialPort
		self.inputFile = inputFile
		self.startAddress = startAddress
		self.verbose = verboseMode
		self.packetSize = packetSize
		self.serialMode = serialMode

	def bootloaderInit(self):
		returnValue = True
		if (returnValue and self.serialMode):
			reply = None
			if self.verbose:
				infoMessage('Sending byte 7f\n')
			self.serialPort.write(struct.pack('B', 127))
			startTime = time.time()
			while (self.serialPort.inWaiting() == 0 and (time.time() - startTime) < 1):
				continue
			r = self.serialPort.read(1)
			if r:
				reply = struct.unpack(('B' * len(r)), r)
			else:
				reply = None
			if self.verbose:
				infoMessage(("Reply received '%s'\n" % hexString(reply)))
			if reply == None:
				errorMessage('No reply received\n')
				returnValue = False
			else:
				if (reply[0]) != ACK:
					errorMessage(('Unexpected reply %s\n' % hexString(reply)))
					returnValue = False
		if returnValue:
			reply = self.sendCommand('GET', [], 2)
			if (len(reply) < 4 or ((reply[0]) != ACK or (reply[-1]) != ACK)):
				errorMessage(('Unexpected reply %s\n' % hexString(reply)))
				returnValue = False
			else:
				if self.verbose:
					infoMessage(('Bootloader version %d.%d\n' % (((reply[2]) & 15), ((reply[2]) >> 4))))
		if returnValue:
			reply = self.sendCommand('GET_ID')
			if (len(reply) == 0 or ((reply[0]) != ACK or (reply[-1]) != ACK)):
				errorMessage(('Unexpected reply %s\n' % hexString(reply)))
				returnValue = False
			else:
				if self.verbose:
					infoMessage((('Product ID = 0x' + ''.join(('%02X' % a) for a in reply[2:-1])) + '\n'))
		return returnValue

	def checksum(self, packet):
		if len(packet) > 1:
			checksum = 0
			for p in packet:
				checksum = (checksum ^ p)
				continue
		else:
			checksum = ((~(packet[0])) & 255)
		return checksum

	def enableReadProtection(self, enable):
		returnValue = True
		if enable == False:
			reply = self.sendCommand('READOUT_UNPROTECT')
			if reply != [ACK, ACK]:
				errorMessage((('Unexpected reply (' + ','.join(('%02x' % a) for a in reply)) + ') \n'))
				returnValue = False
		else:
			item0, item1 = self.memoryRead(134481920, 512)
			returnValue = item0
			cibData = item1
			if returnValue:
				reply = self.sendCommand('ERASE', [[254]], timeout=5)
				if reply != [ACK, ACK]:
					errorMessage(('Unexpected reply %s\n' % repr(reply)))
					returnValue = False
			if returnValue:
				arg1 = u32ToArray(134481922)
				arg1.reverse()
				packet = cibData[2:256]
				packet = ([(len(packet) - 1)] + packet)
				reply = self.sendCommand('WRITE', [arg1, packet], 5)
				if reply != [ACK, ACK, ACK]:
					errorMessage((('Unexpected reply (' + ','.join(('%02x' % a) for a in reply)) + ') \n'))
					returnValue = False
			if returnValue:
				arg1 = u32ToArray(134482176)
				arg1.reverse()
				packet = cibData[256:]
				packet = ([(len(packet) - 1)] + packet)
				reply = self.sendCommand('WRITE', [arg1, packet], 5)
				if reply != [ACK, ACK, ACK]:
					errorMessage((('Unexpected reply (' + ','.join(('%02x' % a) for a in reply)) + ') \n'))
					returnValue = False
			if returnValue:
				reply = self.sendCommand('READOUT_PROTECT')
				if reply != [ACK, ACK]:
					errorMessage((('Unexpected reply (' + ','.join(('%02x' % a) for a in reply)) + ') \n'))
					returnValue = False
		return returnValue

	def eraseUserFlash(self):
		returnValue = True
		reply = self.sendCommand('ERASE', [[255]], timeout=5)
		if reply != [ACK, ACK]:
			errorMessage(('Unexpected reply %s\n' % repr(reply)))
			returnValue = False
		return returnValue

	def isReadProtectionActive(self):
		returnValue, memread = self.memoryRead(134479872, 4)
		return (not returnValue)

	def loadFile(self, inputFile=None, startAddress=None, progressReport=None, doErase=True):
		if inputFile is not None:
			self.inputFile = inputFile
		if startAddress is not None:
			self.startAddress = startAddress
		returnValue = True
		f = fileFormatReader(self.inputFile, self.startAddress)
		try:
			item0, item1 = f.getRawBinary()
			self.startAddress = item0
			file_content = item1
		except IOError:
			errorMessage((('File ' + self.inputFile) + ' open failed\n'))
			returnValue = False
		if returnValue:
			file_size = len(file_content)
			packet = []
			address = self.startAddress
			pages = int(((file_size + 1023) / 1024))
			startPage = (((self.startAddress & 4294966272L) - 134217728) / 1024)
			eraseArg = ([(pages - 1)] + range(startPage, (startPage + pages), 1))
			infoMessage(('Erasing pages from %d to %d...' % (startPage, ((startPage + pages) - 1))))
			if ('STM32W_FLASHER_JLINK_DONT_ERASE' in os.environ or (not doErase)):
				infoMessage('(skipped)', False)
			else:
				reply = self.sendCommand('ERASE', [eraseArg], timeout=10)
				if reply != [ACK, ACK]:
					errorMessage(('Unexpected reply %s\n' % repr(reply)))
					returnValue = False
			if returnValue:
				infoMessage('done\n', False)
		size = 0
		while returnValue:
			packet = []
			packet_size = self.packetSize
			packet_string = file_content[size:(size + packet_size)]
			packet_size = len(packet_string)
			if packet_size == 0:
				infoMessage('\n')
				break
			else:
				size = (size + packet_size)
				packet.extend(packet_string)
				if (len(packet) % 2) != 0:
					packet = (packet + [255])
				packet = ([(len(packet) - 1)] + packet)
				if progressReport:
					progressReport(size, file_size)
				else:
					infoMessage(('Programming %05d/%05d\r' % (size, file_size)))
				arg1 = u32ToArray(address)
				arg1.reverse()
				if (self.serialMode or size == packet_size):
					reply = self.sendCommand('WRITE', [arg1, packet])
					if reply != [ACK, ACK, ACK]:
						errorMessage(('\n\n+Unexpected reply %s, packet_size=%d\n' % (repr(reply), packet_size)))
						returnValue = False
				else:
					retries = 0
					MAX_RETRIES = 3
					while returnValue:
						reply = self.sendCommand('WRITE_INCREMENTAL', [packet])
						if reply != [ACK, ACK]:
							retries = (retries + 1)
							if retries > MAX_RETRIES:
								errorMessage(('\n\nUnexpected reply %s, packet_size=%d\n' % (repr(reply), packet_size)))
								returnValue = False
								break
							else:
								errorMessage(('\n\nUnexpected reply %s, packet_size=%d\n' % (repr(reply), packet_size)))
								infoMessage('Retrying...\n')
								continue
						else:
							break
				address = (address + packet_size)
				continue
		if returnValue:
			if progressReport == None:
				infoMessage('Done\n')
		return returnValue

	def memoryRead(self, address, size):
		returnValue = True
		currentSize = 0
		memRead = []
		while (returnValue and currentSize < size):

				packet_size = min(self.packetSize, (size - currentSize))
				arg1 = u32ToArray((address + currentSize))
				arg1.reverse()
				reply = self.sendCommand('READ', [arg1, [(packet_size - 1)]])
				if reply == [NAK]:
					returnValue = False
				else:
					if reply[:3] != [ACK, ACK, ACK]:
						errorMessage(('\n\nXXUnexpected reply %s, packet_size=%d\n' % (repr(reply), packet_size)))
						returnValue = False
					else:
						memRead = (memRead + reply[3:])
				currentSize = (currentSize + packet_size)
				continue
		return (returnValue, memRead)

	def programCibData(self, cibData):
		returnValue = True
		if returnValue:
			reply = self.sendCommand('ERASE', [[254]], timeout=5)
			if reply != [ACK, ACK]:
				errorMessage(('Unexpected reply %s\n' % repr(reply)))
				returnValue = False
		if returnValue:
			arg1 = u32ToArray(134481920)
			arg1.reverse()
			packet = cibData[:256]
			packet = ([(len(packet) - 1)] + packet)
			reply = self.sendCommand('WRITE', [arg1, packet], 5)
			if reply != [ACK, ACK, ACK]:
				errorMessage((('Unexpected reply (' + ','.join(('%02x' % a) for a in reply)) + ') \n'))
				returnValue = False
		if returnValue:
			arg1 = u32ToArray(134482176)
			arg1.reverse()
			packet = cibData[256:]
			packet = ([(len(packet) - 1)] + packet)
			reply = self.sendCommand('WRITE', [arg1, packet], 5)
			if reply != [ACK, ACK, ACK]:
				errorMessage((('Unexpected reply (' + ','.join(('%02x' % a) for a in reply)) + ') \n'))
				returnValue = False
		return returnValue

	def programUserFlash(self, inputFile, startAddress=134217728, progressReport=None, doErase=True):
		return self.loadFile(inputFile, startAddress, progressReport, doErase)

	def sendCommand(self, command, args=[], timeout=2, traceCommands=False):
		def timedRead(port, timeout, serialMode, bytes=1):
			reply = []
			r = ''
			for byte in range(bytes):
				startTime = time.time()
				r1 = ''
				while (time.time() - startTime) < timeout:
					r1 = port.read(1)
					if len(r1) > 0:
						r = (r + r1)
						startTime = time.time()
						break
					else:
						if serialMode:
							continue
						else:
							time.sleep(0.001)
							continue
				if len(r1) == 0:
					break
				else:
					continue
			if len(r) > 0:
				reply.extend(struct.unpack(('B' * len(r)), r))
			return reply
		reply = []
		error = False
		if command in commands:
			item0, item1 = (commands[command])
			commandID = item0
			replyLength = item1
			if command == 'READ':
				replyLength = (((args[1])[0]) + 4)
		else:
			error = True
		if error:
			pass
		else:
			if traceCommands:
				infoMessage(('Sending command: %02x %02x\n' % (commandID, (255 - commandID))))
			self.serialPort.write(struct.pack('BB', commandID, (255 - commandID)))
			r = timedRead(self.serialPort, timeout, self.serialMode, 1)
			reply = (reply + r)
			if (len(r) != 1 or (r[0]) != ACK):
				error = True
		if error:
			pass
		else:
			for arg in args:
				arg = (arg + [self.checksum(arg)])
				if traceCommands:
					infoMessage((('Sending arg:' + ','.join(('%02x' % a) for a in arg)) + '\n'))
				self.serialPort.write(struct.pack(('B' * len(arg)), *arg))
				r = timedRead(self.serialPort, timeout, self.serialMode, 1)
				reply = (reply + r)
				if (len(r) != 1 or (r[0]) != ACK):
					error = True
					break
				else:
					continue
		if ((not error) and len(reply) < replyLength):
			reply = (reply + timedRead(self.serialPort, timeout, self.serialMode, (replyLength - len(reply))))
			if (command == 'GET' and len(reply) == replyLength):
				reply = (reply + timedRead(self.serialPort, timeout, self.serialMode, ((reply[1]) + 2)))
		if traceCommands:
			infoMessage(('Reply was %s\n' % hexString(reply)))
		return reply

	def setPort(self, serialPort):
		self.serialPort = serialPort

	def startApplication(self, startAddress=134217728):
		returnValue = True
		if (self.startAddress is not None and startAddress != 0):
			startAddress = self.startAddress
		arg1 = u32ToArray(startAddress)
		arg1.reverse()
		reply = self.sendCommand('GO', [arg1])
		if reply != [ACK, ACK, ACK]:
			errorMessage(('\n\nUnexpected reply %s\n' % repr(reply)))
			returnValue = False
		return returnValue

	def verifyFile(self, inputFile=None, startAddress=None, progressReport=None):
		if inputFile is not None:
			self.inputFile = inputFile
		if startAddress is not None:
			self.startAddress = startAddress
		returnValue = True
		f = fileFormatReader(self.inputFile, self.startAddress)
		try:
			item0, item1 = f.getRawBinary()
			self.startAddress = item0
			file_content = item1
		except IOError:
			errorMessage((('File ' + self.inputFile) + ' open failed\n'))
			returnValue = False
		if returnValue:
			file_size = len(file_content)
			packet = []
			address = self.startAddress
		size = 0
		errors = 0
		while returnValue:
			packet = []
			packet_size = self.packetSize
			packet_string = file_content[size:(size + packet_size)]
			packet_size = len(packet_string)
			if packet_size == 0:
				infoMessage('\n')
				break
			else:
				size = (size + packet_size)
				packet.extend(packet_string)
				if progressReport:
					progressReport(size, file_size)
				else:
					infoMessage(('Verifying %05d/%05d\r' % (size, file_size)))
				arg1 = u32ToArray(address)
				arg1.reverse()
				reply = self.sendCommand('READ', [arg1, [(len(packet) - 1)]])
				if reply[:3] != [ACK, ACK, ACK]:
					errorMessage(('\n\nUnexpected reply %s, packet_size=%d\n' % (repr(reply), packet_size)))
					returnValue = False
				if len(reply[3:]) != len(packet):
					errorMessage(('Invalid data read, expected length = %d, received bytes = %d\n' % (len(packet), len(reply[3:]))))
					returnValue = False
				if (returnValue and reply[3:] != packet):
					returnValue = False
					infoMessage('Verify failed								 \n')
					infoMessage(('%-8s: %5s %5s\n' % ('Addr', 'Flash', 'file')))
					for i in range(len(packet)):
						if (reply[(3 + i)]) != (packet[i]):
							infoMessage(('%08x:		%02x		%02x\n' % ((address + i), (reply[(i + 3)]), (packet[i]))))
							errors = (errors + 1)
							if errors > 64:
								break
							else:
								continue
						else:
							continue
				address = (address + packet_size)
				continue
		if returnValue:
			if progressReport == None:
				infoMessage('Done\n')
		return returnValue

	def verifyFlash(self, inputFile, startAddress=134217728, progressReport=None):
		return self.verifyFile(inputFile, startAddress, progressReport)


class STM32F_Interface(object):
	APP_RUNNING = 1
	BL_RUNNING = 0
	DOWNLOAD_BL_IMAGE = 10
	DOWNLOAD_IMAGE = 5
	FAIL = 0
	FIRMWARE_VERSION_NONE = 4294967295L
	GET_APP_VERSION = 3
	GET_BL_VERSION = 9
	GET_CODE_TYPE = 2
	IS_APP_PRESENT = 4
	IS_BL_VERSION_OLD = 11
	OK = 1
	REPLY_START_BYTE = 187
	RUN_APPLICATION = 6
	RUN_BOOTLOADER = 7
	SET_nBOOTMODE = 1
	SET_nRESET = 0
	START_BYTE = 170
	STOP_BYTE = 85
	UNKNOWN_COMMAND = 15
	UPLOAD_IMAGE = 8
	replyLength = [4, 4, 4, 7, 4, 4, 4, 4, 4, 7, 4, 4]
	def __init__(self, port, firmwareName):
		self.port = port
		self.bootloaderPort = port
		self.firmwareName = firmwareName
		self.serialPort = None

	def close(self):
		if self.serialPort:
			self.serialPort.close()

	def getBootloaderFirmwareVersion(self):
		returnValue = self.FIRMWARE_VERSION_NONE
		reply = self.sendCommand(self.GET_BL_VERSION)
		if reply:
			returnValue = ((((reply[2]) + ((reply[3]) << 8)) + ((reply[4]) << 16)) + ((reply[5]) << 24))
		return returnValue

	def getFirmwareType(self):
		returnValue = None
		reply = self.sendCommand(self.GET_CODE_TYPE)
		if reply:
			returnValue = (reply[2])
		return returnValue

	def getFirmwareVersion(self):
		returnValue = self.FIRMWARE_VERSION_NONE
		reply = self.sendCommand(self.GET_APP_VERSION)
		if reply:
			returnValue = ((((reply[2]) + ((reply[3]) << 8)) + ((reply[4]) << 16)) + ((reply[5]) << 24))
		return returnValue

	def getFirmwareVersionFromFile(self, filename=None, stringMode=False):
		version = None
		if filename is None:
			filename = self.firmwareName
		f = open(filename)
		f.seek(28, 0)
		tag = f.read(4)
		versionValue = f.read(4)
		versionValue = struct.unpack(('B' * len(versionValue)), versionValue)
		f.close()
		if tag == '\xaaU\xaaU':
			version = ((((versionValue[0]) + ((versionValue[1]) << 8)) + ((versionValue[2]) << 16)) + ((versionValue[3]) << 24))
			if stringMode:
				version = self.versionInStringFormat(version)
		return version

	def isBootloaderFirmwareVersionOld(self):
		returnValue = False
		reply = self.sendCommand(self.IS_BL_VERSION_OLD)
		if reply:
			returnValue = (reply[2]) == self.FAIL
		return returnValue

	def isFirmwarePresent(self):
		returnValue = None
		reply = self.sendCommand(self.IS_APP_PRESENT)
		if reply:
			returnValue = (reply[2])
		return returnValue

	def isSTMicroelectronics(self, port):
		return port in self.getSTMPorts()

	def mapSTMCompositeComPortToBootloaderCOMPort(self, port):
		warningMessage('The following procedure may fail if there are other attached USB devices.\nIn this case, try to unplug all the devices and insert only the one to be programmed, then run the command again.\n')
		return port

	def open(self):
		error = 0
		try:
			self.serialPort = serial.Serial(port=self.port, baudrate=BOOTLOADER_COMMAND_BAUDRATE, timeout=1)
			time.sleep(0.10000000000000001)
			self.serialPort.flushInput()
		except:
			errorMessage((('Trouble opening port : ' + repr(self.port)) + '\n'))
			error = 1
		return error

	def reset(self, bootloader=False):
		returnValue = True
		if returnValue:
			reply = self.sendCommand(self.SET_nBOOTMODE, [1])
			returnValue = reply != None
		if returnValue:
			reply = self.sendCommand(self.SET_nRESET, [0])
			returnValue = reply != None
		if returnValue:
			if bootloader:
				reply = self.sendCommand(self.SET_nBOOTMODE, [0])
				returnValue = reply != None
		time.sleep(0.10000000000000001)
		if returnValue:
			reply = self.sendCommand(self.SET_nRESET, [1])
			returnValue = reply != None
		time.sleep(0.10000000000000001)
		time.sleep(0.10000000000000001)
		return returnValue

	def runBootloader(self):
		returnValue = None
		reply = self.sendCommand(self.RUN_BOOTLOADER, [], True)
		if reply:
			commandReturnValue = (reply[2])
		time.sleep(TIMEOUT_TO_SWITCH_BETWEEN_APPLICATION_AND_BOOTLOADER)
		for i in range(5):
			try:
				self.serialPort = serial.Serial(port=self.bootloaderPort, baudrate=BOOTLOADER_COMMAND_BAUDRATE, timeout=1)
				self.serialPort = checkSerialPort(self.bootloaderPort, self.serialPort)
				time.sleep(0.5)
				self.serialPort.flushInput()
				if self.serialPort is None:
					returnValue = None
				else:
					returnValue = commandReturnValue
				break
			except Exception, inst:
				errorMessage((' '.join(repr(a) for a in inst.args) + '\n'))
				time.sleep(0.5)
				continue
		return returnValue

	def runFirmware(self):
		returnValue = None
		reply = self.sendCommand(self.RUN_APPLICATION, [], True)
		if reply:
			commandReturnValue = (reply[2])
		time.sleep(TIMEOUT_TO_SWITCH_BETWEEN_APPLICATION_AND_BOOTLOADER)
		for i in range(5):
			try:
				self.serialPort = serial.Serial(port=self.port, baudrate=BOOTLOADER_COMMAND_BAUDRATE, timeout=1)
				self.serialPort = checkSerialPort(self.port, self.serialPort)
				if self.serialPort is None:
					returnValue = None
				else:
					returnValue = commandReturnValue
				break
			except Exception, inst:
				errorMessage((' '.join(repr(a) for a in inst.args) + '\n'))
				time.sleep(0.5)
				continue
		return returnValue

	def sendCommand(self, command, args=[], closePort=False):
		verbose = False
		returnValue = None
		if command == self.DOWNLOAD_IMAGE:
			filename = (args[0])
			args = []
		replyLength = self.replyLength
		packet = (([self.START_BYTE, (1 + len(args)), command] + args) + [self.STOP_BYTE])
		if verbose:
			print '==>',
			print ':'.join(('%02X' % a) for a in packet),
			print
		packedCommand = struct.pack(('B' * len(packet)), *packet)
		self.serialPort.write(packedCommand)
		if command == self.DOWNLOAD_IMAGE:
			yModem = ymodem.Ymodem(self.serialPort, None)
			if yModem.loadFile(filename):
				returnValue = None
		r = self.serialPort.read((replyLength[command]))
		if closePort:
			self.serialPort.close()
		if len(r) == (replyLength[command]):
			reply = struct.unpack(('B' * len(r)), r)
			if ((reply[0]) == self.REPLY_START_BYTE and ((reply[1]) == ((replyLength[command]) - 3) and (reply[-1]) == self.STOP_BYTE)):
				returnValue = reply
		if verbose:
			if r:
				reply = struct.unpack(('B' * len(r)), r)
				print '<==',
				print ':'.join(('%02X' % a) for a in reply),
				print
			else:
				print '<==',
				print
		return returnValue

	def setNBOOT(self, value=1):
		returnValue = True
		if returnValue:
			reply = self.sendCommand(self.SET_nBOOTMODE, [value])
			returnValue = reply != None
		return returnValue

	def upgradeBootloaderFirmwareVersion(self):
		returnValue = True
		reply = self.sendCommand(self.DOWNLOAD_BL_IMAGE)
		if reply:
			returnValue = (reply[2]) == self.OK
		else:
			returnValue = False
		return returnValue

	def upgradeFirmwareVersion(self, filename=None):
		returnValue = True
		if filename is None:
			filename = self.firmwareName
		if returnValue:
			firmwareType = self.getFirmwareType()
			if firmwareType == self.APP_RUNNING:
				reply = self.runBootloader()
				if reply is None:
					returnValue = False
			else:
				if firmwareType == self.BL_RUNNING:
					pass
				else:
					returnValue = False
		if returnValue:
			if self.getFirmwareType() != self.BL_RUNNING:
				errorMessage('Unable to read from port, please use a USB HUB 1.1 and rerun the command\n')
				returnValue = False
			else:
				reply = self.sendCommand(self.DOWNLOAD_IMAGE, [filename])
				returnValue = (reply[2]) == self.OK
		return returnValue

	def versionInStringFormat(self, version):
		return ('%d.%d.%d' % (((version >> 16) & 255), ((version >> 8) & 255), (version & 255)))


class rs232Interface(object):
	def __init__(self, port, noReset=False, rfMode=False, eui64=0):
		self.port = port
		self.noReset = noReset
		self.rfMode = rfMode
		self.eui64 = eui64
		self.serialPort = None
		self.portType = portType(port)
		if self.portType == 'FTDI':
			self.portHandle = FTDI_Interface(port)
		else:
			if self.portType == 'STM32':
				firmwareName = 'CompositeForSTM32W.bin'
				if 'STM32W_FLASHER_FORCE_FIRMWARE_NAME' in os.environ:
					firmwareName = (os.environ['STM32W_FLASHER_FORCE_FIRMWARE_NAME'])
				flasher_py_files = os.path.dirname(os.path.abspath(sys.argv[0]))
				firmware_dir = os.path.dirname(flasher_py_files) # parent directory
				self.portHandle = STM32F_Interface(port, os.path.join(firmware_dir, firmwareName))

	def closePort(self):
		self.serialPort.close()

	def enableReadProtection(self, flag):
		return self.STBL.enableReadProtection(flag)

	def eraseUserFlash(self):
		return self.STBL.eraseUserFlash()

	def init(self, checkFirmwareImage=True):
		error = 0
		if self.noReset:
			if self.rfMode:
				error = self.reset(False)
				time.sleep(0.10000000000000001)
			else:
				error = 0
		else:
			error = self.reset(True, checkFirmwareImage)
			if error == 1:
				errorMessage((('Trouble while resetting board on port : ' + repr(self.port)) + '\n'))
		if error == 0:
			error = self.openPort()
		if error == 0:
			if self.rfMode:
				self.packetSize = 96
				def sendBinary(port, data, length):
					for i in range(length):
						port.write(chr(((data >> (i * 8)) & 255)))
						continue
				sendBinary(self.serialPort, self.eui64, 8)
				sendBinary(self.serialPort, 45067, 2)
				sendBinary(self.serialPort, 15, 1)
				time.sleep(0.5)
				while self.serialPort.inWaiting() > 0:
					sys.stdout.write(self.serialPort.read(self.serialPort.inWaiting()))
					continue
			else:
				self.packetSize = 256
			self.STBL = STBL(self.serialPort, packetSize=self.packetSize, serialMode=(not self.rfMode))
			if self.STBL is not None:
				tmp = self.STBL.bootloaderInit()
				if tmp:
					pass
				else:
					error = 1
		return error

	def isReadProtectionActive(self):
		return self.STBL.isReadProtectionActive()

	def memoryRead(self, address, size):
		return self.STBL.memoryRead(address, size)

	def openPort(self):
		error = 0
		try:
			self.serialPort = serial.Serial(port=self.port, bytesize=8, baudrate=115200, parity='N', timeout=0)
		except:
			errorMessage((('Trouble opening port : ' + repr(self.port)) + '\n'))
			error = 1
		return error

	def programCibData(self, cibData):
		return self.STBL.programCibData(cibData)

	def programUserFlash(self, inputFile, startAddress=134217728, progressReport=None, doErase=True):
		return self.STBL.programUserFlash(inputFile, startAddress, progressReport, doErase)

	def reset(self, bootloader=False, checkSTM32FFirmware=True):
		returnValue = 0
		if self.portType == 'FTDI':
			self.portHandle.reset(bootloader)
		else:
			if self.portType == 'STM32':
				returnValue = self.portHandle.open()
				if checkSTM32FFirmware:
					if returnValue == 0:
						firmwareVersionCurrent = self.portHandle.getFirmwareVersionFromFile(stringMode=False)
					if returnValue == 0:
						firmwareType = self.portHandle.getFirmwareType()
						if (firmwareType is None or (firmwareType != self.portHandle.BL_RUNNING and firmwareType != self.portHandle.APP_RUNNING)):
							errorMessage('Failed to get firmware type: Invalid reply\n')
							returnValue = 1
					if returnValue == 0:
						firmwareVersion = self.portHandle.getFirmwareVersion()
						if firmwareVersion is None:
							errorMessage('Failed to get firmware version:Invalid reply\n')
							returnValue = 1
						else:
							if (firmwareVersion & 4278190080L):
								firmwareVersion = self.portHandle.FIRMWARE_VERSION_NONE
					if returnValue == 0:
						bootloaderFirmwareVersion = self.portHandle.getBootloaderFirmwareVersion()
						if bootloaderFirmwareVersion is None:
							errorMessage('Failed to get bootloader firmware version:Invalid reply\n')
							returnValue = 1
						if bootloaderFirmwareVersion == self.portHandle.FIRMWARE_VERSION_NONE:
							self.portHandle.bootloaderPort = self.portHandle.mapSTMCompositeComPortToBootloaderCOMPort(self.portHandle.port)
					if returnValue == 0:
						if (firmwareType == self.portHandle.BL_RUNNING and (firmwareVersion != self.portHandle.FIRMWARE_VERSION_NONE and firmwareVersion >= firmwareVersionCurrent)):
							if bootloaderFirmwareVersion == self.portHandle.FIRMWARE_VERSION_NONE:
								errorMessage('Please unplug and replug the board to the PC\n')
								returnValue = 2
							else:
								result = self.portHandle.runFirmware()
								if result is not self.portHandle.OK:
									errorMessage('Failed to run firmware: Invalid reply\n')
									returnValue = 1
					if returnValue == 0:
						if (firmwareVersion == self.portHandle.FIRMWARE_VERSION_NONE or firmwareVersion < firmwareVersionCurrent):
							if firmwareVersion == self.portHandle.FIRMWARE_VERSION_NONE:
								infoMessage(('Missing STM32F103 firmware, upgrading to version %s\n' % self.portHandle.getFirmwareVersionFromFile(stringMode=True)))
							else:
								infoMessage(('Old STM32F103 firmware version %s, upgrading to version %s\n' % (self.portHandle.versionInStringFormat(firmwareVersion), self.portHandle.getFirmwareVersionFromFile(stringMode=True))))
								warningMessage('Changing firmware version can break backward compatibility and older version of stm32w_flasher may not work\n')
							if self.portHandle.upgradeFirmwareVersion():
								if ((firmwareVersion == self.portHandle.FIRMWARE_VERSION_NONE or firmwareVersion < 131072) and firmwareVersionCurrent >= 131072):
									errorMessage(('Switching to firmware %s may change your COM port number, please unplug and replug your USB device and re-run this command again\n' % self.portHandle.getFirmwareVersionFromFile(stringMode=True)))
									returnValue = 2
								else:
									result = self.portHandle.runFirmware()
									if result is not self.portHandle.OK:
										errorMessage('Failed to run firmware: Invalid reply\n')
										returnValue = 1
							else:
								errorMessage('Failed to upgrade firmware version: Invalid reply\n')
								returnValue = 1
					if returnValue == 0:
						firmwareVersion = self.portHandle.getFirmwareVersion()
						if firmwareVersion is None:
							errorMessage('Invalid reply\n')
							returnValue = 1
					if returnValue == 0:
						if firmwareVersion >= 131076:
							if self.portHandle.isBootloaderFirmwareVersionOld():
								infoMessage('Upgrading STM32F Bootloader firmware\n')
								if self.portHandle.upgradeBootloaderFirmwareVersion():
									pass
								else:
									errorMessage('Upgrade of the STM32F bootloader failed. This is a CRITICAL error and your board may brick your board\n')
									returnValue = 1
					if returnValue == 0:
						if firmwareVersion > firmwareVersionCurrent:
							warningMessage('Device firmware is more recent than expected. Are you using an old version of this software ?\n')
				if returnValue == 0:
					result = self.portHandle.reset(bootloader)
					if result is None:
						errorMessage('Failed to reset STM32W: Invalid reply\n')
						returnValue = 1
				self.portHandle.close()
			else:
				errorMessage(('Failed to detect port type for %s\n' % self.port))
				returnValue = 1
		return returnValue

	def setNBOOT(self, value):
		returnValue = 0
		if (self.portHandle and self.portType == 'STM32'):
			self.portHandle.open()
			returnValue = self.portHandle.setNBOOT(value)
			self.portHandle.close()
		return returnValue

	def startApplication(self, address=134217728):
		if self.noReset is False:
			self.closePort()
			self.setNBOOT(1)
			self.openPort()
			self.STBL.setPort(self.serialPort)
		return self.STBL.startApplication(address)

	def terminate(self):
		if self.noReset is False:
			self.closePort()
			self.setNBOOT(1)
			self.openPort()

	def verifyFlash(self, inputFile, startAddress, progressReport):
		return self.STBL.verifyFlash(inputFile, startAddress, progressReport)



def checkSerialPort(portName, serialPort):
	returnValue = serialPort
	try:
		serialPort.write(' ')
	except serial.serialutil.SerialException, inst:
		errorMessage((' '.join(repr(a) for a in inst.args) + '\n'))
		infoMessage('Write to serial port failed: please try a different USB port or a USB hub 1.1\n')
		returnValue = None
	return returnValue

def getAvailableSerialPorts(refreshList=True):
	global rs232PortsList
	if (refreshList or rs232PortsList is None):
		returnValue = {}
		id = [{ 'vendor': FT232R_VENDOR , 'product': FT232R_PRODUCT , 'type': 'FTDI' }, { 'vendor': STM32F103_VENDOR , 'product': STM32F103_PRODUCT , 'type': 'STM32' }, { 'vendor': STM32F103_VENDOR , 'product': STM32F103_PRODUCT_OLD , 'type': 'STM32' }]
		for i in range(len(id)):
			command = (((('for udi in `hal-find-by-capability --capability serial`\n								do\n										parent=`hal-get-property --udi ${udi} --key "info.parent"`\n										device=`hal-get-property --udi ${udi} --key "linux.device_file"`\n										serial=`hal-get-property --udi ${parent} --key "usb.serial" 2>/dev/null`\n										vendor=`hal-get-property --udi ${parent} --key "usb.vendor_id" 2>/dev/null`\n										product=`hal-get-property --udi ${parent} --key "usb.product_id" 2>/dev/null`\n										if [ "${vendor}" = "' + str(((id[i])['vendor']))) + '" ] && [ "${product}" = "') + str(((id[i])['product']))) + '" ]\n										then\n												printf "%s %s\n" "${device}" "${serial}"\n										fi\n								done')
			p = subprocess.Popen([command], shell=True, stdout=subprocess.PIPE)
			out = p.communicate()
			for line in (out[0]).splitlines():
				device_serial = line.split(' ')
				returnValue[(device_serial[0])] = [((id[i])['type']), (device_serial[1])]
				continue
			continue
		rs232PortsList = returnValue
	return rs232PortsList

def getFirstAvailableSerialPort():
	returnValue = None
	ports = getAvailableSerialPorts(False)
	if ports != {}:
		returnValue = (sorted(ports.keys())[0])
	return returnValue

def hexString(reply):
	if reply is None:
		return ''
	else:
		return ' '.join(('%02x' % a) for a in reply)

def portType(port):
	returnValue = None
	ports = getAvailableSerialPorts(False)
	if port in ports:
		returnValue = ((ports[port])[0])
	return returnValue

def u32ToArray(v):
	return [(v & 255), ((v >> 8) & 255), ((v >> 16) & 255), ((v >> 24) & 255)]


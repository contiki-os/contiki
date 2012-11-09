
# See comment in stm32w_flasher.py.
# Extraction and little adaptation performed by E.Duble (CNRS, LIG).

import os
import serial
import struct
import time
from messages import errorMessage
from messages import infoMessage


class Ymodem():
	ABORT1 = 65
	ABORT2 = 97
	ACK = 6
	CA = 24
	CRC16 = 67
	EOT = 4
	NAK = 21
	PACKET_1K_SIZE = 1024
	PACKET_HEADER = 3
	PACKET_OVERHEAD = 5
	PACKET_SIZE = 128
	PACKET_TRAILER = 2
	SOH = 1
	STATE_DONE = 6
	STATE_READY = 2
	STATE_SEND_EOT = 3
	STATE_SEND_SESSION_DONE = 4
	STATE_START_APPLICATION = 5
	STATE_WAITING_ACK = 1
	STATE_WAITING_CRC16 = 0
	STX = 2
	YMODEM_RX_TO_TX = 2
	YMODEM_TX_TO_RX = 0
	YMODEM_TX_TO_RX2 = 1
	def Crc16X(self, packet, count):
		j = 0
		crc = 0
		for j in range(count):
			crc = (crc ^ ((packet[j]) << 8))
			for i in range(8):
				if (crc & 32768):
					crc = ((crc << 1) ^ 4129)
					continue
				else:
					crc = (crc << 1)
					continue
			continue
		return crc

	def __init__(self, serialPort, port, updateAction=None):
		self.serialPort = serialPort
		self.updateAction = updateAction
		self.port = port
		return None

	def bootloaderInit(self):
		returnValue = False
		systemRestartPacket = [170, 1, 7, 85]
		self.serialPort.write(struct.pack(('B' * len(systemRestartPacket)), *systemRestartPacket))
		self.serialPort.close()
		time.sleep(5)
		self.serialPort = serial.Serial(port, 10, timeout=4)
		self.serialPort.flushInput()
		systemRestartPacket = [170, 1, 5, 85]
		self.serialPort.write(struct.pack(('B' * len(systemRestartPacket)), *systemRestartPacket))
		self.serialPort.read(4)
		for i in range(10):
			startTime = time.time()
			while (time.time() - startTime) < 0.5:
				if self.serialPort.inWaiting() > 0:
					c = self.serialPort.read(1)
					c = (struct.unpack('B', c)[0])
					if c == ord('C'):
						returnValue = True
						break
				time.sleep(0.01)
				continue
			continue
		return returnValue

	def getByte(self):
		data = None
		if self.serialPort.inWaiting() > 0:
			data = (struct.unpack('B', self.serialPort.read(1))[0])
		if data:
			pass
		return data

	def loadFile(self, name):
		returnValue = True
		state = self.STATE_WAITING_CRC16
		filename = os.path.basename(name)
		startTime = time.time()
		try:
			f = open(name, 'rb')
			infoMessage((('File ' + name) + ' opened\n'))
			f.seek(0, 2)
			file_size = f.tell()
			f.seek(0, 0)
			packet_number = 0
			loop_counter = 1
			size = file_size
			retry = 0
			packet = []
			prevState = self.STATE_READY
			while True:
				newState = state
				if state == self.STATE_WAITING_CRC16:
					if self.updateAction:
						self.updateAction((file_size - max(size, 0)), file_size)
					data = self.getByte()
					if data == self.CRC16:
						newState = self.STATE_READY
				else:
					if state == self.STATE_WAITING_ACK:
						if self.updateAction:
							self.updateAction((file_size - max(size, 0)), file_size)
						data = self.getByte()
						if data is not None:
							if data == self.ACK:
								if prevState == self.STATE_READY:
									retry = 0
									if loop_counter > 1:
										size = (size - packet_size)
									packet_number = ((packet_number + 1) % 256)
									loop_counter = (loop_counter + 1)
									packet_done = True
									if self.updateAction:
										self.updateAction((file_size - max(size, 0)), file_size)
									else:
										infoMessage(('Sent %05d/%05d\r' % ((file_size - max(size, 0)), file_size)))
									newState = self.STATE_READY
								else:
									if prevState == self.STATE_SEND_EOT:
										newState = self.STATE_SEND_SESSION_DONE
									else:
										if prevState == self.STATE_SEND_SESSION_DONE:
											newState = self.STATE_START_APPLICATION
							else:
								if data == self.CA:
									errorMessage('Transaction aborted by client\n')
									newState = self.STATE_DONE
								else:
									if data == self.CRC16:
										pass
									else:
										if prevState == self.STATE_READY:
											infoMessage('Retrying\n')
											retry = (retry + 1)
											if retry > 3:
												errorMessage('Too many retry exiting\n')
												newState = self.STATE_DONE
											else:
												newState = self.STATE_READY
					else:
						if state == self.STATE_READY:
							if size <= 0:
								newState = self.STATE_SEND_EOT
							else:
								if retry == 0:
									packet = []
									if loop_counter == 1:
										packet.extend(struct.unpack(('%uB' % len(filename)), filename))
										packet = (packet + [0])
										size_string = ('%d ' % file_size)
										packet.extend(struct.unpack(('%uB' % len(size_string)), size_string))
										packet_size = self.PACKET_SIZE
									else:
										packet_size = self.PACKET_1K_SIZE
										packet_string = f.read(packet_size)
										packet.extend(struct.unpack(('%uB' % len(packet_string)), packet_string))
									packet = (packet + ([0] * (packet_size - len(packet))))
								if self.sendYModemPacket(packet, packet_number):
									newState = self.STATE_DONE
								else:
									newState = self.STATE_WAITING_ACK
						else:
							if state == self.STATE_SEND_EOT:
								if self.updateAction:
									self.updateAction((file_size - max(size, 0)), file_size)
								else:
									infoMessage(('Sent %05d/%05d\r' % ((file_size - max(size, 0)), file_size)))
								self.sendByte(self.EOT)
								newState = self.STATE_WAITING_ACK
							else:
								if state == self.STATE_SEND_SESSION_DONE:
									self.sendYModemPacket(([0] * 128), 0)
									newState = self.STATE_WAITING_ACK
								else:
									if state == self.STATE_START_APPLICATION:
										self.startApplication()
										newState = self.STATE_DONE
									else:
										if state == self.STATE_DONE:
											returnValue = False
											endTime = time.time()
											infoMessage('\nDone\n')
											break
										else:
											errorMessage(('Unknonw state = %d' % state))
											newState = self.STATE_DONE
				if state != newState:
					prevState = state
					state = newState
					continue
				else:
					continue
			return returnValue
		except:
			errorMessage((('File ' + name) + ' open failed\n'))
			if self.updateAction:
				self.updateAction(0, 0)
			return returnValue

	def sendByte(self, byte):
		self.serialPort.write(struct.pack('B', byte))
		return None

	def sendYModemPacket(self, myPacket, packet_number):
		returnValue = 0
		packet = myPacket[:]
		packet_crc = self.Crc16X(packet, len(packet))
		packet.append(((packet_crc >> 8) & 255))
		packet.append((packet_crc & 255))
		packet.insert(0, (255 - packet_number))
		packet.insert(0, packet_number)
		if len(myPacket) == self.PACKET_SIZE:
			packet.insert(0, self.SOH)
		else:
			packet.insert(0, self.STX)
		self.serialPort.write(struct.pack(('B' * len(packet)), *packet))
		return returnValue

	def startApplication(self):
		return None




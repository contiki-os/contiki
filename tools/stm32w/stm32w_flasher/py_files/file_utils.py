
# See comment in stm32w_flasher.py.
# Extraction and little adaptation performed by E.Duble (CNRS, LIG).

import struct


class Error(Exception):
	"""Base class for exceptions in this module."""
	args = None
	message = None

class FileFormatError(Error):
	"""
	Exception raised for errors in the file format
	
			Attributes:
					filename -- filename with unknown format
					message	-- format error message
			
	"""
	args = None
	message = None
	def __init__(self, filename, message):
		self.filename = filename
		self.message = message


class fileFormatReader(object):
	def __init__(self, filename, startAddress=None):
		self.filename = filename
		self.startAddress = startAddress

	def getRawBinary(self):
		fileContent = None
		bytes = None
		f = open(self.filename, 'rb')
		if self.filename[-4:] == '.bin':
			bytesRaw = f.read()
			bytes = []
			bytes.extend(struct.unpack(('B' * len(bytesRaw)), bytesRaw))
			f.close()
		else:
			if self.filename[-4:] == '.s37':
				fillChar = 255
				fileContent = f.readlines()
				f.close()
				startAddress = None
				currentAddress = None
				bytes = []
				for line in fileContent:
					if line[:2] == 'S3':
						count = int(line[2:4], 16)
						if startAddress is None:
							startAddress = int(line[4:12], 16)
							currentAddress = startAddress
						address = int(line[4:12], 16)
						if currentAddress < address:
							bytes = (bytes + ([fillChar] * (address - currentAddress)))
							currentAddress = address
						else:
							if currentAddress > address:
								raise FileFormatError(self.filename, 'S37, Non progressing addresses detected')
						for i in range((count - 5)):
							bytes = (bytes + [int(line[(12 + (i * 2)):((12 + (i * 2)) + 2)], 16)])
							continue
						currentAddress = (currentAddress + (count - 5))
						continue
					else:
						if line[:2] == 'S0':
							continue
						else:
							if line[:2] == 'S7':
								break
							else:
								raise FileFormatError(self.filename, 'S37: unknown field type')
				self.startAddress = startAddress
			else:
				raise FileFormatError(self.filename, 'Unknown extension')
		return (self.startAddress, bytes)




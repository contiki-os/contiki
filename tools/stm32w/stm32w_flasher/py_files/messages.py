
# See comment in stm32w_flasher.py.
# Extraction and little adaptation performed by E.Duble (CNRS, LIG).

import sys


def errorMessage(msg, header=True):
	if header:
		sys.stderr.write('ERROR: ')
	sys.stderr.write(msg)
	sys.stderr.flush()

def infoMessage(msg, header=True):
	if header:
		sys.stdout.write('INFO: ')
	sys.stdout.write(msg)
	sys.stdout.flush()

def warningMessage(msg, header=True):
	if header:
		sys.stderr.write('WARNING: ')
	sys.stderr.write(msg)
	sys.stderr.flush()


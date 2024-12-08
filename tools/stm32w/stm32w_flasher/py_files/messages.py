
# See comment in stm32w_flasher.py.
# Extraction and little adaptation performed by E.Duble (CNRS, LIG).

import sys
import time


def errorMessage(msg, header=True):
	if header:
		sys.stderr.write('ERROR: ')
	sys.stderr.write(msg)
	sys.stderr.flush()
    time.sleep(1)
def infoMessage(msg, header=True):
	if header:
		sys.stdout.write('INFO: ')
	sys.stdout.write(msg)
	sys.stdout.flush()
        time.sleep(1)
def warningMessage(msg, header=True):
	if header:
		sys.stderr.write('WARNING: ')
	sys.stderr.write(msg)
	sys.stderr.flush()
	time.sleep(1)


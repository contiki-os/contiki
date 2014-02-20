#!/usr/bin/python

from datetime import datetime
from datetime import timedelta
import signal
import os, socket, sys, time

def do_alarm(sig, stack):
	print "Timeout"
	sys.exit(1)

def timeDeltaToMillis(time):
	return time.days * 24 * 3600 * 1000 + time.seconds * 1000 + time.microseconds / 1000.

def main():
	signal.signal(signal.SIGALRM, do_alarm)
	host = sys.argv[1]
	port = 1234
	size = 32

	s = socket.socket(socket.AF_INET6, socket.SOCK_DGRAM)
	s.connect((host, port))
	data = ""
	for i in range(size):
		data += "a"

	start = datetime.now()
	signal.alarm(4)
	
	s.sendall(data)
	resp = s.recv(size)

	end = datetime.now()
	signal.alarm(0)
	duration = timeDeltaToMillis(end - start)

	print "Duration: %d ms" %(duration)
	print "Request:  %s" %(data)
	print "Response: %s" %(resp)

main()

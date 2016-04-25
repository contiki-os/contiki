#!/usr/bin/env python

# program a mote

import os, sys, time, json
import urllib2

host = "http://localhost:12100/post?action=program"
filename = "control.avr-rss2.hex"

if len(sys.argv) > 1:
    filename = sys.argv[1]

def extractStatus(reply):
    try:
        obj = json.loads(reply)
        return (obj["status"] == "OK")
    except:
        return False

def test():
    try:
        with open(filename, "r") as f:
            contents = f.read()
    except Exception as e:
        print("Exception occurred while reading hex file: " + str(e))
        return False

    try:
        opener = urllib2.build_opener(urllib2.HTTPHandler)
        request = urllib2.Request(host, data = contents)
        request.add_header('Content-Type', 'text/plain')
        request.get_method = lambda: 'POST'
        url = opener.open(request)
        reply = url.read()
        print("Reply data:")
        print(reply)
        return extractStatus(reply)
    except Exception as e:
        print("exception occurred while POSTing the firmware to " + host + ": "  + str(e))
        return False
    return status

def main():
    for i in range(2):
        if test():
            sys.stderr.write("test succeeded!\n")
            return
        time.sleep(1.0)

    sys.stderr.write("test failed!\n")


if __name__ == '__main__':
    if len(sys.argv) > 1:
	filename = sys.argv[1]
    if len(sys.argv) > 2:
	host = sys.argv[2]
    main()

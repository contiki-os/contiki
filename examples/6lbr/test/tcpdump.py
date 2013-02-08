#!/usr/bin/python

import subprocess
import sys

class TcpDump:
    def call_tcpdump(self, itf, count, filter, timeout):
        try:
            subprocess.check_output("./tcpdump.sh %d -i %s -c %d %s >/dev/null 2>&1" % (timeout, itf, count, filter), shell=True)
            return True
        except subprocess.CalledProcessError as e:
            return False
    
    def expect_ra(self, itf, timeout):
        print >> sys.stderr, "Waiting RA..."
        return self.call_tcpdump(itf, 1, "icmp6 and ip6[40]=134", timeout)
#!/usr/bin/python

import subprocess
import sys
from time import sleep

class TcpDump:
    instance=None
    def call_tcpdump_fg(self, itf, count, filter, timeout):
        try:
            subprocess.check_output("./tcpdump.sh %d -i %s -c %d %s >/dev/null 2>&1" % (timeout, itf, count, filter), shell=True)
            return True
        except subprocess.CalledProcessError as e:
            return False

    def call_tcpdump_bg(self, itf, count, filter, timeout):
        self.instance = subprocess.Popen("./tcpdump.sh %d -i %s -c %d %s >/dev/null 2>&1" % (timeout, itf, count, filter), shell=True)
        sleep(1)
        return self.instance != None

    def call_tcpdump(self, itf, count, filter, timeout, bg):
        if bg:
            return self.call_tcpdump_bg(itf, count, filter, timeout)
        else:
            return self.call_tcpdump_fg(itf, count, filter, timeout)

    def check_result(self):
        if self.instance:
            self.instance.wait()
            result = self.instance.returncode
            self.instance = None
            return result == 0
        else:
            return False

    def expect_ra(self, itf, timeout, bg=False):
        print >> sys.stderr, "Waiting RA..."
        return self.call_tcpdump(itf, 1, "icmp6 and ip6[40]=134", timeout, bg)
    
    def expect_ns(self, itf, target, timeout, bg=False):
        print >> sys.stderr, "Waiting NS..."
        return self.call_tcpdump(itf, 1,
                                 "icmp6 and ip6[40]=135 and ip6[48:2]=0x%x and ip6[50:2]=0x%x and ip6[52:2]=0x%x and ip6[54:2]=0x%x and ip6[56:2]=0x%x and ip6[58:2]=0x%x and ip6[60:2]=0x%x and ip6[62:2]=0x%x" % (target[0], target[1], target[2],target[3],target[4],target[5],target[6],target[7]),
                                 timeout, bg)

    def expect_na(self, itf, timeout, bg=False):
        print >> sys.stderr, "Waiting NA..."
        return self.call_tcpdump(itf, 1, "icmp6 and ip6[40]=136", timeout, bg)

    def expect_ping_request(self, itf, target, timeout, bg=False):
        print >> sys.stderr, "Waiting ping request..."
        return self.call_tcpdump(itf, 1, "icmp6 and ip6[40]=128 and dst host %s" % target, timeout, bg)

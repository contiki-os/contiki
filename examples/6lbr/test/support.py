#!/usr/bin/python

import sys
from os import system
from subprocess import Popen
import signal
from time import sleep
import config

sys.path.append('../../../tools/sky')

import serial

class BRProxy:
    mode=None
    nvm_file=None
    itf=None
    def setUp(self):
        pass
    def tearDown(self):
        pass

    def set_mode(self, mode, nvm_file):
        self.mode=mode
        self.nvm_file = nvm_file

    def start_6lbr(self, log):
        pass

    def stop_6lbr(self):
        pass

class LocalNativeBR(BRProxy):
    process=None
    
    def setUp(self):
        self.itf="tap0"
        self.log=None

    def tearDown(self):
        if ( self.process ):
            self.stop_6lbr()

    def start_6lbr(self, log_file):
        print >> sys.stderr, "Starting 6LBR..."
        #self.process = Popen(args="./start_br %s -s %s -R -t %s -c %s" % (self.mode, config.radio_dev, self.itf, self.nvm_file), shell=True)
        conf=open("test.conf", 'w')
        print >>conf, "MODE=%s" % self.mode
        print >>conf, "DEV_ETH=eth0"
        print >>conf, "DEV_TAP=%s" % self.itf
        print >>conf, "RAW_ETH=0"
        print >>conf, "DEV_RADIO=%s" % config.radio_dev
        print >>conf, "NVM=%s" % self.nvm_file
        print >>conf, "LIB_6LBR=.."
        print >>conf, "IFUP=../package/usr/lib/6lbr/6lbr-ifup"
        conf.close()
        self.log=open(log_file, "w")
        self.process = Popen(args=["../package/usr/bin/6lbr",  "test.conf"], stdout=self.log)
        sleep(1)
        return self.process != None

    def stop_6lbr(self):
        print >> sys.stderr, "Stopping 6LBR..."
        self.process.send_signal(signal.SIGTERM)
        sleep(1)
        self.log.close()
        self.process = None
        return True

class RemoteNativeBR(BRProxy):    
    def setUp(self):
        self.itf="eth0"

    def tearDown(self):
        if ( self.process ):
            self.stop_6lbr()

    def start_6lbr(self, log):
        print >> sys.stderr, "Starting 6LBR..."
        return False

    def stop_6lbr(self):
        print >> sys.stderr, "Stopping 6LBR..."
        return False

class MoteProxy:
    def setUp(self):
        pass

    def tearDown(self):
        if self.is_mote_started():
            self.stop_mote()
    
    def reset_mote(self):
        pass

    def start_mote(self):
        pass

    def stop_mote(self):
        pass

    def ping(self, address, expect_reply=False, count=0):
        pass

    def is_mote_started(self):
        return False

class TelosMote(MoteProxy):
    def setUp(self):
        self.serialport = serial.Serial(
            port=config.mote_dev,
            baudrate=config.mote_baudrate,
            parity = serial.PARITY_NONE,
            timeout = 2
        )
        self.serialport.flushInput()
        self.serialport.flushOutput()
        self.reset_mote()

    def tearDown(self):
        MoteProxy.tearDown(self)
        self.serialport.close()

    def reset_mote(self):
        print >> sys.stderr, "Reseting mote..."
        self.serialport.flushInput()
        self.serialport.flushOutput()
        self.serialport.write("\r\nreboot\r\n")
        lines = self.serialport.readlines()
        #print >> sys.stderr, lines
        if "Starting '6LBR Demo'\n" not in lines:
            return False
        return True

    def start_mote(self):
        print >> sys.stderr, "Starting mote..."
        self.serialport.write("\r\nstart6lbr\r\n")
        lines = self.serialport.readlines()
        if "done\r\n" not in lines:
            return False
        #print >> sys.stderr, lines
        return True

    def stop_mote(self):
        print >> sys.stderr, "Stopping mote..."
        self.serialport.flushInput()
        self.serialport.flushOutput()
        self.serialport.write("\r\nreboot\r\n")
        lines = self.serialport.readlines()
        #print >> sys.stderr, lines
        if "Starting '6LBR Demo'\n" not in lines:
            return False
        return True

    def ping(self, address, expect_reply=False, count=0):
        print >> sys.stderr, "Ping %s..." % address
        self.serialport.write("\r\nping %s\r\n" % address)
        if expect_reply:
            lines = self.serialport.readlines()
            if "Received an icmp6 echo reply\n" not in lines:
                return False
            #print >> sys.stderr, lines
        return True

class InteractiveMote(MoteProxy):
    mote_started=False
    def start_mote(self,):
        print >> sys.stderr, "*** Press enter when mote is powered on"
        dummy = raw_input()
        self.mote_started=True
        return True

    def stop_mote(self):
        print >> sys.stderr, "*** Press enter when mote is powered off"
        dummy = raw_input()
        self.mote_started=False
        return True

    def ping(self, address, expect_reply=False, count=0):
        print >> sys.stderr, "*** Ping from mote not implemented"
        return False

    def is_mote_started(self):
        return self.mote_started
    
class Platform:
    def setUp(self):
        pass
    
    def tearDown(self):
        pass

    def configure_if(self, itf, address):
        pass

    def unconfigure_if(self, itf):
        pass

    def add_route(self, dest, gw=None, itf=None):
        pass

    def rm_route(self, dest, gw=None, itf=None):
        pass

    def start_ra(self, itf):
        pass

    def stop_ra(self):
        pass

    def check_prefix(self, itf, prefix):
        pass

#    def check_prefix(self, itf, prefix, count):
#        for n in range(count):
#            if (self.check_prefix(itf, prefix)):
#                return True
#            sleep(1)
#        return False

    def accept_ra(self, itf):
        pass

    def accept_rio(self, itf):
        pass

    def ping(self, target):
        pass

class MacOSX(Platform):
    def configure_if(self, itf, address):
        result = system("ifconfig %s inet6 %s/64 add" % (itf, address))
        return result == 0

    def unconfigure_if(self, itf):
        if itf:
            #return system("ifconfig %s down" % itf) == 0
            system("ifconfig %s down" % itf)
            return True
        else:
            return True

    def add_route(self, dest, gw=None, itf=None):
        if itf:
            result = system("route add -inet6 -prefixlen 64 %s -interface %s" % (dest, itf))
        else:
            result = system("route add -inet6 -prefixlen 64 %s %s" % (dest, gw))
        return result == 0

    def rm_route(self, dest, gw=None, itf=None):
        if itf:
            result = system("route delete -inet6 -prefixlen 64 %s -interface %s" % (dest, itf))
        else:
            result = system("route delete -inet6 -prefixlen 64 %s %s" % (dest, gw))
        return result == 0

    def start_ra(self, itf):
        system("sysctl -w net.inet6.ip6.forwarding=1")
        self.rtadvd = Popen(args="rtadvd  -f -c rtadvd.%s.conf %s" % (itf, itf), shell=True)
        return self.rtadvd != None

    def stop_ra(self):
        self.rtadvd.send_signal(signal.SIGTERM)
        self.rtadvd = None
        return True

    def check_prefix(self, itf, prefix):
        result = system("ifconfig %s | grep '%s'" % (itf, prefix))
        return result == 0

    def accept_ra(self, itf):
        system("sysctl -w net.inet6.ip6.accept_rtadv=1")
        return True

    def support_rio(self):
        return False

    def accept_rio(self, itf):
        return False

    def ping(self, target):
        print >> sys.stderr, "ping..."
        result = system("ping6 -c 1 %s > /dev/null 2>/dev/null" % target)
        if result != 0:
            sleep(1)
        return result == 0

class Linux(Platform):
    radvd = None
    
    def configure_if(self, itf, address):
        result = system("ip addr add %s/64 dev %s" % (address, itf))
        return result == 0

    def unconfigure_if(self, itf):
        if itf:
            return system("ifconfig %s down" % itf) == 0
        else:
            return True

    def add_route(self, dest, gw=None, itf=None):
        if gw:
            result = system("route -A inet6 add %s/64 gw %s" % (dest, gw))
        else:
            result = system("route -A inet6 add %s/64 %s" % (dest, itf))            
        return result == 0

    def rm_route(self, dest, gw=None, itf=None):
        if gw:
            result = system("route -A inet6 del %s/64 gw %s" % (dest, gw))
        else:
            result = system("route -A inet6 del %s/64 %s" % (dest, itf))
        return result == 0

    def start_ra(self, itf):
        system("sysctl -w net.ipv6.conf.%s.forwarding=1" % itf)
        self.radvd = Popen(args="radvd -m stderr -d 5 -n -C radvd.%s.conf" % itf, shell=True)
        return self.radvd != None

    def stop_ra(self):
        self.radvd.send_signal(signal.SIGTERM)
        self.radvd = None
        return True

    def check_prefix(self, itf, prefix):
        result = system("ifconfig %s | grep '%s'" % (itf, prefix))
        return result == 0

    def accept_ra(self, itf):
        system("sysctl -w net.ipv6.conf.%s.accept_ra=1" % itf)
        return True

    def support_rio(self):
        return True

    def accept_rio(self, itf):
        system("sysctl -w net.ipv6.conf.%s.accept_ra_rt_info_max_plen=64" % itf)
        return True

    def ping(self, target):
        result = system("ping6 -W 1 -c 1 %s > /dev/null" % target)
        if result >> 8 == 2:
            sleep(1)
        return result == 0

if __name__ == '__main__':
    mote=TelosMote()
    mote.setUp()
    mote.start_mote()
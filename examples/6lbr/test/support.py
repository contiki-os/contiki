#!/usr/bin/python

import sys
from os import system
import subprocess
import signal
from time import sleep
import config
import re
import pty
import os

sys.path.append('../../../tools/sky')

import serial

class BRProxy:
    itf=None
    def setUp(self):
        pass
    def tearDown(self):
        pass

    def set_mode(self, mode, channel, ra_daemon=False, accept_ra=False):
        pass

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

    def set_mode(self, mode, channel, ra_daemon=False, accept_ra=False):
        self.mode=mode
        conf=open("test.conf", 'w')
        print >>conf, "MODE=%s" % mode
        print >>conf, "DEV_ETH=eth0"
        print >>conf, "DEV_TAP=%s" % self.itf
        print >>conf, "RAW_ETH=0"
        if hasattr(config, 'radio_dev'):
            print >>conf, "DEV_RADIO=%s" % config.radio_dev
        if hasattr(config, 'radio_sock'):
	    print >>conf, "SOCK_RADIO=%s" % config.radio_sock
        print >>conf, "NVM=test.dat"
        print >>conf, "LIB_6LBR=.."
        print >>conf, "IFUP=../package/usr/lib/6lbr/6lbr-ifup"
        print >>conf, "IFDOWN=../package/usr/lib/6lbr/6lbr-ifdown"
        conf.close()
        subprocess.check_output("../tools/nvm_tool --new --channel=%d --wsn-accept-ra=%d --eth-ra-daemon=%d test.dat" % (channel, accept_ra, ra_daemon), shell=True)

    def start_6lbr(self, log_file):
        print >> sys.stderr, "Starting 6LBR..."
        #self.process = Popen(args="./start_br %s -s %s -R -t %s -c %s" % (self.mode, config.radio_dev, self.itf, self.nvm_file), shell=True)
        self.log=open(log_file, "w")
        self.process = subprocess.Popen(args=["../package/usr/bin/6lbr",  "./test.conf"], stdout=self.log)
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

    def set_mode(self, mode, channel, ra_daemon=False, accept_ra=False):
        pass

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

    def start_mote(self, channel):
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
            timeout = 1
        )
        self.reset_mote()
        self.serialport.flushInput()
        self.serialport.flushOutput()

    def tearDown(self):
        MoteProxy.tearDown(self)
        self.serialport.close()

    def wait_until(self, text, count):
        for n in range(count):
            lines = self.serialport.readlines()
            #print >> sys.stderr, lines
            if text in lines:
                return True
        return False

    def reset_mote(self):
        print >> sys.stderr, "Reseting mote..."
        system("../../../tools/sky/msp430-bsl-linux --telosb -c %s -r" % config.mote_dev)
        sleep(2)

    def start_mote(self, channel):
        print >> sys.stderr, "Starting mote..."
        self.serialport.flushInput()
        self.serialport.flushOutput()
        self.serialport.write("\r\nrfchannel %d\r\n" % channel)
        self.serialport.write("\r\nstart6lbr\r\n")
        return self.wait_until("done\r\n", 5)

    def stop_mote(self):
        print >> sys.stderr, "Stopping mote..."
        self.serialport.flushInput()
        self.serialport.flushOutput()
        self.serialport.write("\r\nreboot\r\n")
        return self.wait_until("Starting '6LBR Demo'\n", 5)

    def ping(self, address, expect_reply=False, count=0):
        print >> sys.stderr, "Ping %s..." % address
        self.serialport.write("\r\nping %s\r\n" % address)
        if expect_reply:
            return self.wait_until("Received an icmp6 echo reply\n", 10)
        else:
            return True

class VirtualTelosMote(MoteProxy):
    def setUp(self):
        print("Setting up Cooja, compiling node firmwares...")
        self.cooja = subprocess.Popen(['java', '-jar', '../../../tools/cooja/dist/cooja.jar', 
                                       '-nogui=./cooja-small-nogui.csc'], stdout=subprocess.PIPE)
        line = self.cooja.stdout.readline()
        while 'Simulation main loop started' not in line: # Wait for simulation to start      
            line = self.cooja.stdout.readline()
        print("Cooja simulation started")
        sleep(2)

        master, slave = pty.openpty()
        self.socat = subprocess.Popen(['socat', '-d', '-d', 'TCP:localhost:60002', 'PTY'], shell=False, stdout=slave, stderr=slave, close_fds=True)
        socat_output = os.fdopen(master)

        line = socat_output.readline()
        while 'N PTY is' not in line: # TODO: Generalize and make Mac-friendly
            line = socat_output.readline()
        print >> sys.stderr, line

        re_socat_ok = re.compile(".+(/dev/pts/[0-9]+).+") # TODO: Generalize and make Mac-friendly
        re_socat_res = re_socat_ok.match(line)
        if re_socat_res:
            mote_dev = re_socat_res.group(1).rstrip()
            print >> sys.stderr, mote_dev
            self.serialport = serial.Serial(
                port=mote_dev,
                baudrate=config.mote_baudrate,
                parity = serial.PARITY_NONE,
                timeout = 1
            )
            self.reset_mote()
            self.serialport.flushInput()
            self.serialport.flushOutput()
    
    def tearDown(self):
        MoteProxy.tearDown(self)
        self.serialport.close()
        print("Killing Cooja")
        self.socat.terminate()
        self.cooja.terminate()


    def wait_until(self, text, count):
        for n in range(count):
            lines = self.serialport.readlines()
            #print >> sys.stderr, lines
            if text in lines:
                return True
        return False

    def reset_mote(self):
        print >> sys.stderr, "Resetting mote..."
        self.serialport.flushInput()
        self.serialport.flushOutput()
        self.serialport.write("\r\nreboot\r\n")
        return self.wait_until("Starting '6LBR Demo'\n", 5)

    def start_mote(self, channel):
        print >> sys.stderr, "Starting mote..."
        self.serialport.flushInput()
        self.serialport.flushOutput()
        self.serialport.write("\r\nrfchannel %d\r\n" % channel)
        self.serialport.write("\r\nstart6lbr\r\n")
        return self.wait_until("done\r\n", 5)

    def stop_mote(self):
        print >> sys.stderr, "Stopping mote..."
        self.serialport.flushInput()
        self.serialport.flushOutput()
        self.serialport.write("\r\nreboot\r\n")
        return self.wait_until("Starting '6LBR Demo'\n", 5)

    def ping(self, address, expect_reply=False, count=0):
        print >> sys.stderr, "Ping %s..." % address
        self.serialport.write("\r\nping %s\r\n" % address)
        if expect_reply:
            return self.wait_until("Received an icmp6 echo reply\n", 10)
        else:
            return True
    

class InteractiveMote(MoteProxy):
    mote_started=False
    def start_mote(self,channel):
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

    def unconfigure_if(self, itf, address):
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

    def get_address_with_prefix(self, itf, prefix):
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
    rtadvd=None

    def tearDown(self):
        if self.rtadvd:
            self.stop_ra()

    def configure_if(self, itf, address):
        result = system("ifconfig %s inet6 %s/64 add" % (itf, address))
        return result == 0

    def unconfigure_if(self, itf, address):
        if itf:
            #return system("ifconfig %s down" % itf) == 0
            system("ifconfig %s inet6 %s/64 delete" % (itf, address))
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
        print >> sys.stderr, "Start RA daemon..."
        system("sysctl -w net.inet6.ip6.forwarding=1")
        system("sysctl -w net.inet6.ip6.accept_rtadv=0")
        self.rtadvd = subprocess.Popen(args="rtadvd  -f -c rtadvd.%s.conf %s" % (itf, itf), shell=True)
        return self.rtadvd != None

    def stop_ra(self):
        print >> sys.stderr, "Stop RA daemon..."
        self.rtadvd.send_signal(signal.SIGTERM)
        self.rtadvd = None
        return True

    def check_prefix(self, itf, prefix):
        result = system("ifconfig %s | grep '%s'" % (itf, prefix))
        return result == 0

    def get_address_with_prefix(self, itf, prefix):
        return subprocess.check_output("ifconfig %s | egrep -o '(%s[:0-9a-f]+)'" % (itf, prefix), shell=True)

    def accept_ra(self, itf):
        system("sysctl -w net.inet6.ip6.forwarding=0")
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

    def tearDown(self):
        if self.radvd:
            self.stop_ra()
    
    def configure_if(self, itf, address):
        result = system("ip addr add %s/64 dev %s" % (address, itf))
        return result == 0

    def unconfigure_if(self, itf, address):
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
        print >> sys.stderr, "Start RA daemon..."
        system("sysctl -w net.ipv6.conf.%s.forwarding=1" % itf)
        self.radvd = subprocess.Popen(args="radvd -m stderr -d 5 -n -C radvd.%s.conf" % itf, shell=True)
        return self.radvd != None

    def stop_ra(self):
        print >> sys.stderr, "Stop RA daemon..."
        self.radvd.send_signal(signal.SIGTERM)
        self.radvd = None
        return True

    def check_prefix(self, itf, prefix):
        result = system("ifconfig %s | grep '%s'" % (itf, prefix))
        return result == 0

    def get_address_with_prefix(self, itf, prefix):
        return subprocess.check_output("ifconfig %s | egrep -o '(%s[:0-9a-f]+)'" % (itf, prefix), shell=True)

    def accept_ra(self, itf):
        system("sysctl -w net.ipv6.conf.%s.forwarding=0" % itf)
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

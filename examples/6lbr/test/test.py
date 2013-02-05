#!/usr/bin/python

import unittest
import sys
from os import system
from subprocess import Popen
from time import sleep
import config
from support import *

class TestSupport:
    br=config.br
    mote=config.mote
    platform=config.platform
    mode=None

    def setUp(self, mode):
        self.mode=mode
        self.mode.support=self
        self.platform.setUp()
        self.br.setUp()
        self.mote.setUp()
        self.mode.setUp()

    def tearDown(self):
        self.mode.tearDown()
        self.mote.tearDown()
        self.br.tearDown()
        self.platform.tearDown()

    def set_up_network(self):
        return self.mode.set_up_network()

    def tear_down_network(self):
        return self.mode.tear_down_network()

    def set_mode(self, mode, nvm_file):
        return self.br.set_mode(mode, nvm_file)

    def start_ra(self, itf):
        return self.platform.start_ra(itf)

    def stop_ra(self):
        return self.platform.stop_ra()

    def start_6lbr(self, log):
        return self.br.start_6lbr(log)

    def stop_6lbr(self):
        return self.br.stop_6lbr()

    def start_mote(self, load):
        return self.mote.start_mote(load)

    def stop_mote(self):
        return self.mote.stop_mote()

    def ping(self, target):
        return self.platform.ping(target)

    def wait_mote_in_6lbr(self, count):
        return True

    def wait_ping(self, count, target):
        for n in range(count):
            if (self.ping(target)):
                return True
        return False

    def ping_6lbr(self):
        return self.ping( self.mode.ip_6lbr )

    def wait_ping_6lbr(self, count):
        return self.wait_ping( count, self.mode.ip_6lbr )

    def ping_mote(self):
        return self.ping( self.mode.ip_mote )

    def wait_ping_mote(self, count):
        return self.wait_ping( count, self.mode.ip_mote )

class TestScenarios:
    support=TestSupport()

    def test_S0(self):
        """
        Check 6LBR start-up anbd connectivity
        """
        self.assertTrue(self.support.start_6lbr('test_S0.log'), "Could not start 6LBR")
        self.assertTrue(self.support.set_up_network(), "Could not set up network")
        self.assertTrue(self.support.wait_ping_6lbr(40), "6LBR is not responding")
        self.assertTrue(self.support.tear_down_network(), "Could not tear down network")
        self.assertTrue(self.support.stop_6lbr(), "Could not stop 6LBR")

    #@unittest.skip("test")
    def test_S1(self):
        """
        Ping from the computer to the mote when the PC knows the BR but the BR does not know the
        mote.
        """
        self.assertTrue(self.support.start_6lbr('test_S1.log'), "Could not start 6LBR")
        self.assertTrue(self.support.set_up_network(), "Could not set up network")
        self.assertTrue(self.support.start_mote('default'), "Could not start up mote")
        self.assertTrue(self.support.wait_mote_in_6lbr(30), "Mote not detected")
        self.assertTrue(self.support.wait_ping_mote(60), "Mote is not responding")
        self.assertTrue(self.support.stop_mote(), "Could not stop mote")
        self.assertTrue(self.support.tear_down_network(), "Could not tear down network")
        self.assertTrue(self.support.stop_6lbr(), "Could not stop 6LBR")

    @unittest.skip("test")
    def test_S2(self):
        """
        Ping from the computer to the mote when the PC does not know the BR and the BR knows
        the mote.
        """
        self.assertTrue(self.support.start_6lbr('test_S2.log'), "Could not start 6LBR")
        self.assertTrue(self.support.start_mote('default'), "Could not start up mote")
        self.assertTrue(self.support.wait_mote_in_6lbr(30), "Mote not detected")
        self.assertTrue(self.support.set_up_network(), "Could not set up network")
        self.assertTrue(self.support.wait_ping_mote(60), "Mote is not responding")
        self.assertTrue(self.support.stop_mote(), "Could not stop mote")
        self.assertTrue(self.support.tear_down_network(), "Could not tear down network")
        self.assertTrue(self.support.stop_6lbr(), "Could not stop 6LBR")

    @unittest.skip("test")
    def test_S3(self):
        """
        Ping from the computer to the mote when everyone is known but the mote has been disconnected.
        """
        self.assertTrue(self.support.start_6lbr('test_S3.log'), "Could not start 6LBR")
        self.assertTrue(self.support.start_mote('default'), "Could not start up mote")
        self.assertTrue(self.support.wait_mote_in_6lbr(30), "Mote not detected")
        self.assertTrue(self.support.set_up_network(), "Could not set up network")
        self.assertTrue(self.support.wait_ping_mote(60), "Mote is not responding")
        self.assertTrue(self.support.stop_mote(), "Could not stop mote")
        self.assertFalse(self.support.wait_ping_mote(10), "Mote is still responding")
        self.assertTrue(self.support.tear_down_network(), "Could not tear down network")
        self.assertTrue(self.support.stop_6lbr(), "Could not stop 6LBR")

    @unittest.skip("test")
    def test_S4(self):
        """
        Starting from a stable RPL topology, restart the border router and observe how it attaches
        to the RPL DODAG.
        """
        self.assertTrue(self.support.start_6lbr('test_S4_a.log'), "Could not start 6LBR")
        self.assertTrue(self.support.start_mote('default'), "Could not start up mote")
        self.assertTrue(self.support.wait_mote_in_6lbr(30), "Mote not detected")
        self.assertTrue(self.support.set_up_network(), "Could not set up network")
        self.assertTrue(self.support.wait_ping_mote(60), "Mote is not responding")
        self.assertTrue(self.support.tear_down_network(), "Could not tear down network")
        self.assertTrue(self.support.stop_6lbr(), "Could not stop 6LBR")
        self.assertTrue(self.support.start_6lbr('test_S4_b.log'), "Could not start 6LBR")
        self.assertTrue(self.support.set_up_network(), "Could not set up network")
        self.assertTrue(self.support.wait_ping_mote(60), "Mote is not responding")
        self.assertTrue(self.support.stop_mote(), "Could not stop mote")
        self.assertTrue(self.support.tear_down_network(), "Could not tear down network")
        self.assertTrue(self.support.stop_6lbr(), "Could not stop 6LBR")

    @unittest.skip("test")
    def test_S5(self):
        """
        Wait for a DAD between the computer and the BR, then disconnect and reconnect the com-
        puter and observe the reaction of the BR to a computer's DAD.
        """
        pass

    @unittest.skip("test")
    def test_S6(self):
        """
        Observe the NUDs between the computer and the BR.
        """
        pass

    @unittest.skip("test")
    def test_S7(self):
        """
        Test the Auconfiguration process of the BR in bridge mode and observe its ability to take a
        router prefix (by using the computer as a router), and deal with new RA once configured.
        """
        pass

    @unittest.skip("test")
    def test_S8(self):
        """
        Observe the propagation of the RIO in the WSN side (when supported in the WPAN).
        """
        pass

    @unittest.skip("test")
    def test_S9(self):
        """
        Test the using of the default router.
        """
        pass

#@unittest.skip("test")
class SmartBridgeManual(unittest.TestCase,TestScenarios):
    def setUp(self):
        self.support.setUp(SmartBridgeManualMode())
    def tearDown(self):
        self.support.tearDown()

#@unittest.skip("test")
class SmartBridgeAuto(unittest.TestCase,TestScenarios):
    def setUp(self):
        self.support.setUp(SmartBridgeAutoMode())
    def tearDown(self):
        self.support.tearDown()

class Router(unittest.TestCase,TestScenarios):
    def setUp(self):
        self.support.setUp(RouterMode())
    def tearDown(self):
        self.support.tearDown()

#@unittest.skip("test")
class RouterNoRa(unittest.TestCase,TestScenarios):
    def setUp(self):
        self.support.setUp(RouterNoRaMode())
    def tearDown(self):
        self.support.tearDown()

if __name__ == '__main__':
    unittest.main()

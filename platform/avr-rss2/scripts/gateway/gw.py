#!/usr/bin/env python

# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#  * Redistributions of source code must retain the above copyright notice,
#    this list of  conditions and the following disclaimer.
#  * Redistributions in binary form must reproduce the above copyright
#   notice, this list of conditions and the following disclaimer in the
#   documentation and/or other materials provided with the distribution.
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS OR
# CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
# EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
# PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
# OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
# WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
# OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
# ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#
# Gateway <-> WSN communication daemon
#

#
# Author: Atis Elsts, 2014-2016
#

import os, sys, time, struct, json, datetime, tempfile, re
import threading, signal, select, serial, socket, traceback

# add library directory to path
SELF_DIR = os.path.dirname(os.path.abspath(__file__))
sys.path.append(os.path.join(SELF_DIR, '..', "lib"))

import daemon
from util import *
from component_id import *
from flow_objects import *
import motelist

if isPython3():
    from http.server import *
    from socketserver import *
    from urllib.parse import *
else:
    from BaseHTTPServer import *
    from SocketServer import *
    from urlparse import *
    from urllib import *

################################################

SUPPORTED_MOTES = ["sky", "z1", "avr-rss2"]

BSL_SCRIPT_PATH = {
    "z1" : os.path.normpath('../../contiki/tools/z1/z1-bsl-nopic'),
    "sky" : os.path.normpath('../../contiki/tools/sky/msp430-bsl-linux'),
    "avr-rss2" : os.path.normpath('/usr/bin/avrdude')
}

PROFUN_MAGIC_NUMBER = 0x4FA1

NUM_REPROGRAMMING_TRIES = 3

SERIAL_RECONNECT_INTERVAL = 5  # seconds

SERIAL_READ_INTERVAL      = 0.1    # seconds; lower means more CPU load

SERIAL_CMD_EXCHANGE_INTERVAL = SERIAL_READ_INTERVAL * 4

ALERT_CAUSE_NO_PACKETS          = 1
ALERT_CAUSE_FAULTY_READINGS     = 2
ALERT_CAUSE_LARGE_DELAY         = 3
ALERT_CAUSE_NODE_DEAD           = 4
ALERT_CAUSE_UNKNOWN_SOURCE      = 5
ALERT_CAUSE_UNKNOWN_DESTINATION = 6

MAX_ALERTS = 10

# for all kinds of data
MAX_LINE_LENGTH = 256
# TODO: collect data from different nodes in different dictionaries
#MAX_READINGS = 100 # of one kind
MAX_READINGS = 10
# for raw output data collection
MAX_NUM_LINES = 256 # leading to max 64kb buffers (assuming Python char is 1 byte)

# -- Command sending

#MIN_COMMAND_SEND_INTERVAL = SERIAL_READ_INTERVAL
# make this larger if the network is slow
MIN_COMMAND_SEND_INTERVAL = 1.0 

RECV_ACK_WAIT_TIME        = 2.0 # seconds

#DELIVER_ACK_WAIT_TIME     = 15.0 # seconds
DELIVER_ACK_WAIT_TIME      = 10.0 # seconds
#DELIVER_ACK_WAIT_TIME     = 5.0 # seconds

COMMAND_SERIAL_SEND_RETRIES    = 5
COMMAND_WIRELESS_SEND_RETRIES  = 20

# kept in GW server memory; resend on e.g. node reboots
MAX_COMMANDS_TOTAL        = 100

# for synchronization of router's time
LOCAL_TIME_OFFSET = 0.0

reReceivedAck = Matcher(r'^T=R S=([0-9]+)$')
reDeliveryAck = Matcher(r'^T=A S=([0-9]+)$')
reDataMsg     = Matcher(r'^T=D A=([0-9]+).([0-9]+) F=([0-9]+) S=([0-9]+) V=([0-9]+)$')
reAlertMsg    = Matcher(r'^T=W A=([0-9]+).([0-9]+) S=([0-9]+) C=([0-9]+) D=([0-9]+)$')

# --

DEFAULT_CONFIG_FILE = "config.json"

DEFAULT_CONFIG = {
    "httpPort" : 12100,
    "serialPort" : "/dev/ttyUSB0",
    "moteSerialID" : None,
    "discoveryServerAddress" : "localhost",
    "discoveryServerPort" : 6100,
    "serialBaudrate" : 115200,
    "moteAddress" : None,
    "allOutput" : False,
    "moteType" : "z1",
    "daemonize" : False,
    "simulatedNet" : False,
    "logLevel" : 2,
    "logFile" : "log.log"
}

LOG_FATAL = 0
LOG_ERROR = 1
LOG_INFO  = 2
LOG_DEBUG = 3

portLock = threading.Lock()
commandLock = threading.Lock()
bufferLock = threading.Lock()
readBufferLock = threading.Lock()

enc = json.JSONEncoder()

DO_WRITE_BYTEARRAY = sys.version_info[0] >= 3 or sys.version_info[1] >= 7

################################################

# this section is for radio PDR test

RADIOTEST_COMMAND_ACCEPTED = "command accepted"
RADIOTEST_COMMAND_FINISHED = "ready to accept commands"
RADIOTEST_COMMAND_TX_FINISHED = "send done"
RADIOTEST_COMMAND_STAT_FINISHED  = "end of statistics"

RADIO_TEST_NO_COMMAND       = 1
RADIO_TEST_COMMAND_QUEUED   = 2
RADIO_TEST_COMMAND_ACCEPTED = 3
RADIO_TEST_COMMAND_FINISHED = 4

RADIO_TEST_MAX_RETRIES = 5

################################################

config = {}

def getConfigSetting(name, default = None, useHardcodedConfig = True):
    if name in config:
        return config[name]
    if useHardcodedConfig:
        return DEFAULT_CONFIG.get(name, default)
    return default

def log(loglevel, msg):
    if loglevel <= int(getConfigSetting("logLevel", 1)):
        msg += "\n"
        sys.stderr.write(msg)
        with open(getConfigSetting("logFile"), "a+") as f:
            f.write(msg)

def getSyncTime():
    return time.time() + LOCAL_TIME_OFFSET

################################################

#
# This function emulates the Sky/Z1 upload sequence defined in Contiki Makefiles
#
def programMote(bslScript, portname, platformArgs, filename):
    allOutput = ''

    # Mass erase
    time.sleep(1.0)
    args = ["python", bslScript, "-c", portname, "-e"]
    args.extend(platformArgs)
    (retcode, output) = runSubprocess(" ".join(args))
    allOutput += output
    if retcode:
        return (retcode, allOutput)

    # Program
    time.sleep(2.0)
    args = ["python", bslScript, "-c", portname, "-I"]
    args.extend(platformArgs)
    args.append("-p")
    args.append(filename)
    (retcode, output) = runSubprocess(" ".join(args))
    allOutput += output
    if retcode:
        return (retcode, allOutput)

    # Reset
    time.sleep(2.0)
    args = ["python", bslScript, "-c", portname, "-r"]
    args.extend(platformArgs)
    (retcode, output) = runSubprocess(" ".join(args))
    allOutput += output

    return (retcode, allOutput)

#
# AVR-RSS2 reprogramming: send "upgrade" command first to reboot the mote in bootloader mode
#
def programMoteAvrRSS2(bslScript, portname, platformArgs, filename):
    allOutput = ''

    baudrate = int(getConfigSetting("serialBaudrate"))

    try:
        port = serial.Serial(portname, baudrate, timeout=1, 
                             parity=serial.PARITY_NONE, rtscts=0)
        if port:
            port.write("upgrade\n")
            port.close()
    except serial.SerialException as ex:
        log(LOG_ERROR, "Serial exception:\n\t{0}".format(ex))
        return

    time.sleep(0.2) # wait for the node to reboot

#avrdude -p m256rfr2 -c stk500v2 -P /dev/ttyUSB0 -b 38400 -e -U flash:w:control.avr-rss2.hex

    args = [bslScript, "-p", "m256rfr2", "-c", "stk500v2", "-P", portname, "-b", "38400", "-e", "-U",
            "flash:w:" + filename]
    (retcode, output) = runSubprocess(" ".join(args))

    return (retcode, allOutput)

################################################

class BinaryCommand:
    def __init__(self, data, command):
        self.data = data
        self.command = command
        self.remainingSerialRetries = COMMAND_SERIAL_SEND_RETRIES
        self.remainingWirelessRetries = COMMAND_WIRELESS_SEND_RETRIES
        self.sendTime = None
        self.seqnum = 0

class CommandSender:
    def __init__(self, globalState):
        self.gs = globalState
        self.timerId = None
        self.doQuit = False

        self.allCommandList = []
        # all commands are partitioned in blocks; next block is not sent
        # before all commands from the previous block are acknowledged.
        self.blockWriteSeqnum = 0
        self.blockReadSeqnum = 0
        # 0 and 1 are reserved sequence numbers
        self.nextSeqnum = 2

        self.reset()

    def cleanup(self):
        with commandLock:
            if self.timerId:
                self.timerId.cancel()
                self.timerId = None

            self.doQuit = True

    def reset(self):
        with commandLock:
            self.commandQueue = []
            # resend all past commands
            if len(self.allCommandList):
                log(LOG_INFO, "will resend {0} previously issued commands".format(
                    len(self.allCommandList)))

            # fix the read position (write position remains the same)
            for (_, _, orderNum) in self.allCommandList:
                if orderNum >= self.blockReadSeqnum:
                    self.blockReadSeqnum = orderNum - 1

            self.unackedCommand = None
            self.undeliveredCommands = []

    def fullReset(self):
        self.allCommandList = []
        self.blockWriteSeqnum = 0
        self.blockReadSeqnum = 0
        with commandLock:
            self.nextSeqnum = 2
        self.reset()

    def addCommands(self, gs, newCommands):
        newBinaryCommands = []

        # delete old constraints first
        inc = False
        for c in newCommands:
            if c.type == MSG_FIELD_DEL_CONSTRAINT:
                log(LOG_INFO, "add command " + c.getDescription(gs))
                if not inc:
                    inc = True
                    self.blockWriteSeqnum += 1
                newBinaryCommands.append((c.getBinary(gs), c, self.blockWriteSeqnum))
        # all other commands go here
        inc = False
        for c in newCommands:
            if c.type not in [MSG_FIELD_ADD_CONSTRAINT, MSG_FIELD_DEL_CONSTRAINT]:
                log(LOG_INFO, "add command " + c.getDescription(gs))
                if not inc:
                    inc = True
                    self.blockWriteSeqnum += 1
                newBinaryCommands.append((c.getBinary(gs), c, self.blockWriteSeqnum))
        # add new constraints last
        inc = False
        for c in newCommands:
            if c.type == MSG_FIELD_ADD_CONSTRAINT:
                log(LOG_INFO, "add command " + c.getDescription(gs))
                if not inc:
                    inc = True
                    self.blockWriteSeqnum += 1
                newBinaryCommands.append((c.getBinary(gs), c, self.blockWriteSeqnum))

        self.allCommandList = limitLength(
            self.allCommandList + newBinaryCommands, MAX_COMMANDS_TOTAL)

        with commandLock:
            if len(self.commandQueue) == 0:
                self.commandQueue = self.getNextCommands()

    def getNextCommands(self):
        result = []
        if len(self.allCommandList) == 0: return result
        nextSeqnum = self.blockReadSeqnum + 1

        alreadyIn = set()

        # XXX: this goes through the whole past command list: can be slow?
        for (data, c, orderNum) in self.allCommandList:
            if orderNum < nextSeqnum:
                continue
            if orderNum > nextSeqnum:
                break

            # TODO: also should use semantic equivalence test here!
            if data not in alreadyIn:
                result.append(BinaryCommand(data, c))
                alreadyIn.add(data)
        if len(result):
            self.blockReadSeqnum = nextSeqnum
        return result

    def rxRecvAck(self, seqnum):
        with commandLock:
            if not self.unackedCommand:
                log(LOG_INFO, "Got a recv ACK, but no outstanding commands")
                return

            if self.unackedCommand.seqnum != seqnum:
                log(LOG_INFO, "Got a recv ACK, but sequence number {0} instead of {1}".format(
                    seqnum, self.unackedCommand.seqnum))
                return

            # ok, so it's acked; free it.
            self.unackedCommand = None

    def rxDeliveryAck(self, seqnum):
        with commandLock:
            # remove command with that seqnum from undelivered ones
            self.undeliveredCommands = [x for x in self.undeliveredCommands \
                                        if x.seqnum != seqnum]
            # also check if it can be used for recv ACK
            if self.unackedCommand and self.unackedCommand.seqnum == seqnum:
                self.unackedCommand = None

    # compare an old command with the current ones,
    # and skip it if the contents are identical
    def isInQueue(self, binaryCommand):
        # Note: caller must hold commandLock
        # Note: this will be slow with large number of commands

        # TODO: should compare the commands semanticaly
        # (e.g. if "add task x" with new parameters is present,
        # the old "add task x" command should be dropped!

        for c in self.commandQueue:
            if c.data == binaryCommand.data:
                return True
        return False

    def process(self):
#        global startTime

        with commandLock:
            if self.doQuit: return

            if self.unackedCommand:
                elapsedTime = time.time() - self.unackedCommand.sendTime
                if elapsedTime > RECV_ACK_WAIT_TIME:
                    if self.unackedCommand.remainingSerialRetries > 0:
                        # retry sending the last command
                        log(LOG_INFO, "Will resend the last command: not recv ACKed!")
                        self.unackedCommand.remainingSerialRetries -= 1
                        self.commandQueue = [self.unackedCommand] + self.commandQueue
                    self.unackedCommand = None

            elif len(self.undeliveredCommands):
                oldCommands = [x for x in self.undeliveredCommands \
                               if time.time() - x.sendTime > DELIVER_ACK_WAIT_TIME]
                for c in oldCommands:
                    if c.remainingWirelessRetries > 0:
                        if not self.isInQueue(c):
                            log(LOG_INFO, "Will resend a command: not delivery ACKed!")
                            c.remainingWirelessRetries -= 1
                            self.commandQueue = [c] + self.commandQueue
                # filter out the resent commands
                self.undeliveredCommands = [x for x in self.undeliveredCommands \
                                            if x not in oldCommands]

            elif len(self.commandQueue) == 0:
                self.commandQueue = self.getNextCommands()

            if len(self.commandQueue) > 0 \
               and self.gs.isMoteReadyForRx \
               and not self.unackedCommand:
                cmd = self.commandQueue[0]
                self.unackedCommand = cmd
                self.undeliveredCommands.append(cmd)
                self.commandQueue = self.commandQueue[1:]

                # XXX: the description may be wrong here in case the global state has changed
                # since the command was created
                log(LOG_INFO, "Sending a command with seqnum " + str(self.nextSeqnum) \
                    + ": " + cmd.command.getDescription(self.gs))

                data = struct.pack("<HH", PROFUN_MAGIC_NUMBER, self.nextSeqnum)
                data += cmd.data
                # append checksum
                data += struct.pack("B", crc8(data))

                with bufferLock:
                    self.gs.writeBuffer += data

                self.unackedCommand.sendTime = time.time()
                self.unackedCommand.seqnum = self.nextSeqnum

                self.nextSeqnum += 1
                if self.nextSeqnum >= 0x10000:
                    self.nextSeqnum = 2
#            elif startTime is not None:
#                if self.unackedCommand is None \
#                        and len(self.undeliveredCommands) == 0 \
#                        and len(self.commandQueue) == 0:
#                    duration = time.time() - startTime
#                    print("\nTime to setup: {}\n".format(duration))
#                    startTime = None
#                if len(self.commandQueue) == 0:
#                    print "cq empty"
#                if not self.gs.isMoteReadyForRx:
#                    print "mote not ready"
#                if self.unackedCommand:
#                    print "unacked cmd"

            self.timerId = threading.Timer(MIN_COMMAND_SEND_INTERVAL,
                                           CommandSender.process, args = [self]).start()

################################################

class RuntimeState:
    def __init__(self):
        self.port = None
        self.portname = ""
        self.serialPortBusy = False
        self.cmdSender = CommandSender(self)
        self.reset()

    def reset(self):
        # -- own state
        with bufferLock:
            self.writeBuffer = ""
        with readBufferLock:
            self.readBuffer = ""
            self.readLines = []
        # -- task graph state
        self.network = {}
        self.tasks = {}
        self.constraints = {}
        self.selfNodeId = None
        self.taskId2TaskNum = {}
        self.constraintId2ConstraintNum = {}
        self.lastTaskNumber = 0
        self.lastConstraintNumber = 0
        self.networkVersion = None
        # -- connected mote state
        self.realMoteAddress = None
        self.isMoteReadyForRx = False # has it been started and alive at all?

        self.alerts = []
        self.readings = {}
        self.cmdSender.reset()
        # -- connected mote state for radio test
        self.radioTestMoteCommandState = RADIO_TEST_NO_COMMAND

    def fullReset(self):
        self.cmdSender.fullReset()
        self.reset()

    def cleanup(self):
        self.cmdSender.cleanup()
        self.closeSerial()

    def processAlert(self, address, taskNr, cause, data):
        log(LOG_INFO, "received alert: address=" + raddrStr(address) + " taskNr=" + str(taskNr) \
            + " cause=" + str(cause) + " data=" + str(data))
        node = self.findNodeByAddress(address)
        if not node:
            log(LOG_ERROR, "received alert from unknown address " + raddrStr(address))
            return

        if taskNr:
            taskId = self.getTaskIdByNumber(taskNr)
            if not taskId:
                log(LOG_ERROR, "received alert from unknown task number " + str(taskNr))
                return
        else:
            taskId = None

        self.addAlert(node, taskId, cause)

    def addAlert(self, node, taskId, cause):
        #print "add alert on ", node.getIdentifier(True), " task:", taskId

        # limit the buffer's size
        self.alerts = limitLength(self.alerts, MAX_ALERTS)
        # append a new alert
        alert = {"nodeId": node.id, "cause": cause, "timestamp": getSyncTime()};
        if taskId: alert["taskId"] = taskId;
        self.alerts.append(alert)

    def processData(self, address, functionId, seqnum, value):
        log(LOG_DEBUG, "received data: address=" + raddrStr(address) + " functionId=" \
            + str(functionId) +  " seqnum=" + str(seqnum) + " value=" + str(value))

        node = self.findNodeByAddress(address)
        if not node:
            log(LOG_DEBUG, "received data from an unknown address " + raddrStr(address))

        dataName = findFunctionNameById(functionId)
        if dataName is None:
            log(LOG_ERROR, "unknown function id " + str(functionId))
            return

        if dataName not in self.readings:
            self.readings[dataName] = []

        # TODO: check the seqnum for already seen data

        # TODO: use the seqnum to reorder data

        # limit the buffer's size
        self.readings[dataName] = limitLength(self.readings[dataName], MAX_READINGS)

        # append the new measurement
        dataItem = {"v": value, "ts": getSyncTime()}
        if node:
            dataItem["nodeId"] = node.id
            dataItem["nodeAddress"] = node.getIdentifier(getConfigSetting("simulatedNet"))
        self.readings[dataName].append(dataItem)

    def getTaskIdByNumber(self, taskNr):
        for t in self.taskId2TaskNum:
            if self.taskId2TaskNum[t] == taskNr:
                return t
        return None

    def findTaskByFunctionId(self, functionId):
        for taskId in self.tasks:
            task = self.tasks[taskId]
            if "runtimeId" in task:
                if int(task["runtimeId"]) == functionId:
                    return task
                continue
            taskType = task.get("type", "")
            if taskType in functionNameToIdMapping:
                if functionNameToIdMapping[taskType] == functionId:
                    return task
        return None

    def allocateTaskNumber(self, id):
        if id in self.taskId2TaskNum:
            return self.taskId2TaskNum[id]
        self.lastTaskNumber += 1
        self.taskId2TaskNum[id] = self.lastTaskNumber
        return self.lastTaskNumber

    def freeTaskNumber(self, taskId):
        try:
            del self.taskId2TaskNumber[taskId]
        except:
            pass

    def allocateConstraintNumber(self, id):
        if id in self.constraintId2ConstraintNum:
            return self.constraintId2ConstraintNum[id]

        self.lastConstraintNumber = findUnusedKey(
            self.constraintId2ConstraintNum, self.lastConstraintNumber, 0, 255)
        if self.lastConstraintNumber == None:
            log(LOG_ERROR, "Too many tasks! Maximum 256 supported")
            return None

        self.constraintId2ConstraintNum[id] = self.lastConstraintNumber
        return self.lastConstraintNumber

    def freeConstraintNumber(self, id):
        try:
            del self.constraintId2ConstraintNum[id]
        except:
            pass

    def taskIdToFunctionId(self, taskId):
        if taskId not in self.tasks:
            return 0
        task = self.tasks[taskId]
        if task.runtimeId is not None:
            return task.runtimeId
        if task.taskType in functionNameToIdMapping:
            return functionNameToIdMapping[task.taskType]
        return 0

    def nodeIdToNodeAddress(self, nodeId):
        if nodeId not in self.network:
            # log(LOG_ERROR, "nodeIdToNodeAddress: unknown node ID " + nodeId)
            return 0
        return self.network[nodeId].getIdentifier(getConfigSetting("simulatedNet"))

    def nodeAddressToNodeId(self, nodeAddress):
        return nodeAddress - 1

    def inputString(self, line):
        log(LOG_INFO, str(round(getSyncTime(), 1)) + " > " + line)
        # XXX: the fact that we need to do this may mean a bug in the middleware
        line = line.rstrip()
        # testing pdr without return commands

        # -- radio test
        if line == RADIOTEST_COMMAND_ACCEPTED:
            if self.radioTestMoteCommandState < RADIO_TEST_COMMAND_ACCEPTED:
                self.radioTestMoteCommandState = RADIO_TEST_COMMAND_ACCEPTED
            return
        if line == RADIOTEST_COMMAND_FINISHED:
            self.radioTestMoteCommandState = RADIO_TEST_COMMAND_FINISHED
            return
        if line.find(RADIOTEST_COMMAND_TX_FINISHED) == 0:
            self.radioTestMoteCommandState = RADIO_TEST_COMMAND_FINISHED
            return
        if line.find(RADIOTEST_COMMAND_STAT_FINISHED) == 0:
            self.radioTestMoteCommandState = RADIO_TEST_COMMAND_FINISHED
            return


        # if the line has a checksum, check it
        if len(line) > 3 and line[len(line) - 3] == ',':
            # handle checksum
            calcCrc = crc8(line[:-3])
            try:
                recvCrc = int(line[-2:], 16) 
            except:
                recvCrc = 0xffff # invalid value
            if calcCrc != recvCrc:
                log(LOG_INFO, "the received line has a bad checksum")
                return
            # cut the end of the line
            line = line[:-3]

        # heartbeat message?
        addr = extractRimeAddress(line, prefix = "^T=H A=")
        if addr:
            self.isMoteReadyForRx = True
            self.realMoteAddress = addr
            return

        # "received" ACK
        m = reReceivedAck.match(line)
        if m.matched():
            seqnum = m.as_int(1)
            self.cmdSender.rxRecvAck(seqnum)
            return

        # "delivered" ACK
        m = reDeliveryAck.match(line)
        if m.matched():
            seqnum = m.as_int(1)
            self.cmdSender.rxDeliveryAck(seqnum)
            return

        # data message?
        m = reDataMsg.match(line)
        if m.matched():
            self.processData(m.as_int(2) * 256 + m.as_int(1), m.as_int(3), m.as_int(4),
                             m.as_int(5))
            return

        # alert message?
        m = reAlertMsg.match(line)
        if m.matched():
            self.processAlert(m.as_int(2) * 256 + m.as_int(1), m.as_int(3), m.as_int(4), m.as_int(5))
            return

    def closeSerial(self):
        with portLock:
            try:
                if self.port:
                    self.port.close()
            except:
                pass
            finally:
                self.port = None
        self.reset()

    def serveRadioTestControl(self, testTimestamp, serverTime, command, cmdParams, doSync):
        global LOCAL_TIME_OFFSET
        if serverTime:
            # fix the local timestamp (with max error up to 1 second)
            LOCAL_TIME_OFFSET = serverTime - time.time()

        #if command in ["send", "recv"]:
        #    print("test start: " + str(getSyncTime()))

        if not self.port:
            return '{"status":"error"}'

        if not command:
            # this is a status request message
            isReady = self.radioTestMoteCommandState == RADIO_TEST_COMMAND_FINISHED
            numSync = RADIO_TEST_MAX_RETRIES if doSync else 0
            while not isReady and numSync > 0:
                # yield and wait
                numSync -= 1
                time.sleep(SERIAL_CMD_EXCHANGE_INTERVAL)
            return '{"status":true,"ready":' + str(isReady).lower() + '}'

        if self.radioTestMoteCommandState == RADIO_TEST_COMMAND_ACCEPTED and command != "end":
            if command == "ping":
                with bufferLock:
                    state.writeBuffer += "ping\n"

            numSync = RADIO_TEST_MAX_RETRIES if doSync else 0
            while numSync and self.radioTestMoteCommandState == RADIO_TEST_COMMAND_ACCEPTED:
                # wait for reply from serial
                time.sleep(SERIAL_CMD_EXCHANGE_INTERVAL)
                if numSync: numSync -= 1

            if self.radioTestMoteCommandState == RADIO_TEST_COMMAND_FINISHED:
                print("test end: " + str(getSyncTime()))
                return '{"status":true,"ready":true}'
            return '{"status":true,"ready":false}'

        if command in ["tx", "rx", "stat", "ch", "txp", "te", "end"]:
            self.radioTestMoteCommandState = RADIO_TEST_COMMAND_QUEUED
            with bufferLock:
                state.writeBuffer += cmdParams + "\n"

            if command in ["rx", "ch", "txp"]
                self.radioTestMoteCommandState = RADIO_TEST_COMMAND_FINISHED
            else if command in ["tx", "stat"]
                self.radioTestMoteCommandState = RADIO_TEST_COMMAND_ACCEPTED

            numSync = RADIO_TEST_MAX_RETRIES if doSync else 1
            while numSync > 0:
                numSync -= 1
                # yield and let the serial cmd exchange take place
                time.sleep(SERIAL_CMD_EXCHANGE_INTERVAL)

                if self.radioTestMoteCommandState == RADIO_TEST_COMMAND_FINISHED:
                    if command == "end":
                        print("test end: " + str(getSyncTime()))
                    return '{"status":"true","ready":"true"}'
                elif (not doSync) and self.radioTestMoteCommandState == RADIO_TEST_COMMAND_ACCEPTED:
                    if command == "end":
                        print("test end: " + str(getSyncTime()))
                    return '{"status":"true","ready":"false"}'

            if command == "end":
                print("test end: " + str(getSyncTime()))
            return '{"status":"false","ready":"false"}'

        if self.radioTestMoteCommandState == RADIO_TEST_COMMAND_QUEUED:
            with bufferLock:
                state.writeBuffer += "ping\n"
            return '{"status":"false","ready":"false"}'

        return '{"status":"true","ready":"true"}'

    def getStatus(self, lastTimestamp):
        # include local time here (as the clock is not required to be synched with frontend!)
        reply = {"alerts":[],"timestamp":getSyncTime()}
        #print "getStatus, alerts=", self.alerts, "lastTimestamp=", lastTimestamp
        for a in self.alerts:
            if (lastTimestamp is None) or a["timestamp"] > lastTimestamp:
                reply["alerts"].append(a)

        return reply

    def getReadings(self, sensors, lastTimestamp, faultySensors):
        reply = []
        for sensorName in sensors.split(","):
            sensorData = {"values":[], "sensor":sensorName}
            for d in self.readings.get(sensorName, []):
                if (lastTimestamp is None) or d["ts"] > lastTimestamp + 0.1: # XXX
                    sensorData["values"].append(d)
            reply.append(sensorData)

        if faultySensors:
            try:
                for faultyNameAddress in faultySensors.split(","):
                    (name, address) = faultyNameAddress.split(".")
                    node = self.findNodeByAddress(int(address))
                    task = self.findTaskByName(name)
                    print ("fault, address: {0} task: {1}".format(address, name))
                    if not node or not task:
                        if not node: print("no such node")
                        if not task: print("no such task")
                        continue
                    self.addAlert(node, task.id, ALERT_CAUSE_FAULTY_READINGS)
            except Exception as e:
                log(LOG_INFO, "Get readings: failed to parse the faulty sensor string: " + str(e))

        return reply

    def getOutput(self, lastTimestamp):
        with readBufferLock:
            result = "\n".join(self.readLines)
            # Note: the timestamp argument is left unused for now;
            # instead, the state is fully reset on each GET (in the next line)
            self.readLines = []
        return result

    def processProgrammingRequest(self, handler, contents, qs):
        if not self.port:
            handler.respondToPost('{"status":"error", "msg":"serial port not opened"}')
            return

        #moteType = qsExtractString(qs, "moteType", "z1")
        moteType = getConfigSetting("moteType")

        if moteType == "sky":
            platformArgs = ["--telosb"]
        elif moteType == "z1":
            platformArgs = ["--z1"]
        elif moteType == "avr-rss2":
            platformArgs = []
        else:
            handler.respondToPost('{"status":"error", "msg":"unknown mote type"}')
            return

        (fd, filename) = tempfile.mkstemp()
        f = os.fdopen(fd, 'w')
        f.write(contents)
        f.close()

        self.closeSerial()
        self.serialPortBusy = True

        bslScript = BSL_SCRIPT_PATH[moteType]

        ok = False
        for i in range(NUM_REPROGRAMMING_TRIES):
            log(LOG_INFO, "Trying to program a mote, port=" + self.portname)
            # call the programmer script
            if moteType != "avr-rss2":
                (retcode, output) = programMote(bslScript, self.portname, platformArgs, filename)
            else:
                (retcode, output) = programMoteAvrRSS2(bslScript, self.portname, platformArgs, filename)
            if retcode == 0:
                log(LOG_INFO, "Successfully reprogrammed, port=" + self.portname)
                ok = True
                response = '{"status":"OK"}'
                break
            response = '{"status":"error", "msg":"' + enc.encode(output) + '"}'
            time.sleep(2.0)

        if not ok:
            log(LOG_ERROR, "Giving up on reprogramming, port=" + self.portname)

        # cleanup
        self.serialPortBusy = False
        os.remove(filename)
        # reopen serial port
        reconnectSerial()
        # send HTTP reply to the remote side
        handler.respondToPost(response)      

    @staticmethod
    def updateFlowObject(container, type, json, id = None, extraArgs = None):
        # print("update flow object " + type + ": " + str(json))
        if id is None:
            id = json["id"]
        if id not in container:
            container[id] = FlowObject.create(type, id)
        container[id].setJSON(json)
        if extraArgs is not None:
            container[id].setExtraArgs(extraArgs)
        return container[id]

    @staticmethod
    def advanceGeneration(container):
        for n in container:
            container[n].advanceGeneration()

    @staticmethod
    def garbageCollectFlowObjects(container):
        # garbage collect it, if it's not alive in this version (generation) of the flow
        removed = []
        for id in container:
            if container[id].garbageCollect():
                removed.append(id)
        for n in removed:
            del container[n]

    def findReferences(self, container):
        for id in container:
            container[id].findReferences(self)

    def processReconfigurationRequest(self, handler, contents, qs):
        try:
            root = json.loads(contents)
        except:
            handler.respondToPost('{"error":"invalid request format"}')
            return

        portsChanged = False
        if self.port:
            newPortname = queryPortName()
            oldPortname = self.portname if self.port else None
            if not (newPortname is None) and newPortname != oldPortname:
                log(LOG_INFO, "Simulator serial ports has changed, reconnecting")
                portsChanged = True
                self.closeSerial() # automatically resets current taskgraph state
                # TODO: wait here until the thread has exited!
                time.sleep(2.0)
                reconnectSerial()

        newNetworkVersion = qsExtractString(qs, "version")
        if self.networkVersion != newNetworkVersion:
            self.fullReset()
            self.networkVersion = newNetworkVersion

        self.advanceGeneration(self.network)
        self.advanceGeneration(self.tasks)
        self.advanceGeneration(self.constraints)

        self.selfNodeId = qsExtractString(qs, "nodeid")
        isSimulator = qsExtractBool(qs, "simulator")
        isWiredRuntime = False
        constraintMinPackets = 0

        for element in root:
            if not "type" in element: continue
            type = element["type"]
            if type == "options":
                if "comm" in element:
                    isWiredRuntime = element["comm"].get("isWiredRuntime", False)
                    try:
                        constraintMinPackets = int(element["comm"].get("constraintMinPackets", 10))
                    except:
                        pass

        for element in root:
            if not "type" in element: continue
            type = element["type"]
            if ("runtimeId" in element) or (type in functionNameToIdMapping):
                # a task
                self.updateFlowObject(self.tasks, "Task", element)

            elif "z" in element and element["z"] == "net_tab":
                # a network node
                self.updateFlowObject(self.network, "Node", element)
                id = element["id"]
                if self.realMoteAddress is not None and \
                        self.network[id].address == self.realMoteAddress:
                    self.selfNodeId = id

            elif type == "globals":
                for c in element.get("constraints", []):
                    if "delayValue" in c:
                        self.updateFlowObject(self.constraints, "Constraint", c, 
                                              id = c["name"]+" delay",
                                              extraArgs = constraintMinPackets)
                    if "pdrValue" in c:
                        self.updateFlowObject(self.constraints, "Constraint", c,
                                              id = c["name"]+" pdr",
                                              extraArgs = constraintMinPackets)
                    if "energyValue" in c:
                        self.updateFlowObject(self.constraints, "Constraint", c,
                                              id = c["name"]+" energy",
                                              extraArgs = constraintMinPackets)

        # do this to get matching IDs
        for taskId in self.tasks:
            self.allocateTaskNumber(taskId)
        for constraintId in self.constraints:
            self.allocateConstraintNumber(constraintId)

        #print("find refs")
        self.findReferences(self.network)
        self.findReferences(self.tasks)
        self.findReferences(self.constraints)

        selfNode = self.network.get(self.selfNodeId, None)
        onlyLocalNode = not isSimulator and isWiredRuntime
        if onlyLocalNode and (not selfNode or not selfNode.isAlive):
            response = '{"status":"error", "msg":"Unable to find self node address in the flow!"}'
        else:
            if onlyLocalNode:
                commands = selfNode.getCommands(self.network)
            else:
                commands = []
                for nodeId in self.network:
                    commands += self.network[nodeId].getCommands(self.network)

            if commands:
                # TODO: print in human readable format
                sys.stdout.write("got {0} new commands\n".format(len(commands)))
                self.cmdSender.addCommands(self, commands)
            response = '{"status":"OK"}'

        self.garbageCollectFlowObjects(self.network)
        self.garbageCollectFlowObjects(self.tasks)
        self.garbageCollectFlowObjects(self.constraints)

        handler.respondToPost(response)

    def findNodeByAddress(self, address):
        for n in self.network:
            node = self.network[n]
            try:
                nodeAddress = node.getIdentifier(getConfigSetting("simulatedNet"))
            except:
                nodeAddress = None
            # print "nodeAddress=", nodeAddress
            if nodeAddress and nodeAddress == address:
                return node
        return None

    def findTaskByName(self, taskName):
        for id in self.tasks:
            if self.tasks[id].taskType == taskName:
                return self.tasks[id]
        return None

state = RuntimeState()

################################################

def queryPortName():
    # get the name from the Serial2Pty webserver
    try:
        address = getConfigSetting("discoveryServerAddress", useHardcodedConfig=False)
        port = int(getConfigSetting("discoveryServerPort", 6100))
        if not address:
            log(LOG_DEBUG, "Serial port discovery server address not configured")
            return None
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        sock.settimeout(3.0)
        sock.connect((address, int(port)))
        data = ""
        while True:
            c = sock.recv(1)
            if c not in ['/', '.'] and not c.isalnum():
                break
            data += c
            if len(data) > 100: break
        sock.close()
        return data
    except Exception as e:
        log(LOG_DEBUG, "Connection to serial port discovery server failed (normal): " + str(e))
        return None

################################################

serialTimer = None

def reconnectSerial():
    global serialTimer
    if not state: return 
    if serialTimer: serialTimer.cancel()  
    if not state.port:
        createBackgroundThread(listenSerial, None)
    serialTimer = threading.Timer(SERIAL_RECONNECT_INTERVAL, reconnectSerial).start()

def decideSerialPortName():
    moteType = getConfigSetting("moteType")
    if moteType not in SUPPORTED_MOTES:
        log(LOG_INFO, "Invalid mote type configured: " + str(moteType))
        log(LOG_INFO, "Defaulting to 'sky'")
        moteType = "sky"

    # if moteSerialID is set, find corresponding port name by searching the output of `motelist`
    moteSerialID = getConfigSetting("moteSerialID", useHardcodedConfig=False)
    if moteSerialID:
        output = motelist.Motelist.getClassicFormatMotelist()
        for line in output.split("\n"):
            if moteSerialID in line:
                try:
                    return line.split()[1]
                except:
                    break
        # do not try the other options
        return None

    # then try explicitly configured port name
    portname = getConfigSetting("serialPort", useHardcodedConfig=False)
    if portname:
        return portname

    # as the last option, try connecting to the discovery server
    portname = queryPortName()
    return portname

def listenSerial(args):
    with portLock:
        if state.port:
            log(LOG_INFO, "Serial port already opened")
            return

        # if busy, fail silently
        if state.serialPortBusy:
            return

        portname = decideSerialPortName()
        if not portname:
            # error already reported
            return

        # check again
        if state.serialPortBusy: return

        baudrate = int(getConfigSetting("serialBaudrate"))
        try:
            port = serial.Serial(portname, baudrate, timeout=1, 
                                       parity=serial.PARITY_NONE, rtscts=0)
        except serial.SerialException as ex:
            log(LOG_ERROR, "Serial exception:\n\t{0}".format(ex))
            return
        # set the port and release the lock
        state.port = port
        state.portname = portname

    try:
        state.port.flushInput()
        state.port.flushOutput()
    except serial.SerialException as ex:
        state.closeSerial()
        return

    log(LOG_ERROR, "Using port {0}, baudrate {1}".format(portname, baudrate))

    try:
        # send a hello message when the port just has been opened
        time.sleep(1.0)
        state.port.write("ping\n")
    except Exception as e:
        log(LOG_ERROR, "Sending ping to serial port failed: " + str(e))

    doAllOutput = bool(getConfigSetting("allOutput"))

    while state:
        if not state.port: break

        # read
        try:
            if state.port.inWaiting():
                s = state.port.read(state.port.inWaiting())
                if type(s) is str:
                    with readBufferLock:
                        state.readBuffer += s
                        if '\n' in state.readBuffer:
                            lines = state.readBuffer.split('\n')
                            for line in lines[:-1]:
                                state.inputString(line)
                                if doAllOutput:
                                    state.readLines.append(line)
                            state.readBuffer = lines[-1]
                            if doAllOutput:
                                # limit the max number of lines
                                state.readLines = state.readLines[:MAX_NUM_LINES]
                        else:
                            # limit the max length
                            state.readBuffer = state.readBuffer[:MAX_LINE_LENGTH]
                else:
                    sys.stdout.write("rx binary data: ")
                    for c in s:
                        sys.stdout.write("{0}".format(chr(c)) )
                    sys.stdout.flush()

        except (serial.SerialException, IOError) as ex:
            log(LOG_ERROR, "Serial exception:\n\t{0}".format(ex))
            state.closeSerial()
            return

        # allow other threads to run
        time.sleep(SERIAL_READ_INTERVAL / 2)

        # write
        with bufferLock:
            if state.writeBuffer:
                for c in state.writeBuffer:
                    try:
                        if DO_WRITE_BYTEARRAY:
                            state.port.write(bytearray([c]))
                        else:
                            state.port.write(c)
                    except (serial.SerialException, IOError) as ex:
                        log(LOG_ERROR, "Serial exception:\n\t{0}".format(ex))
                        state.closeSerial()
                        return
                state.writeBuffer = ""

        # allow other threads to run
        time.sleep(SERIAL_READ_INTERVAL / 2)

    log(LOG_INFO, "Serial listening done")
    state.closeSerial()
    return 0

################################################

class HttpServerHandler(BaseHTTPRequestHandler):
    server_version = 'ProFuN gateway server'

    # overrides base class function, because in some versions
    # it tries to resolve dns and fails...
    def log_message(self, format, *args):
        log(LOG_INFO, "%s - - [%s] %s" %
                         (self.client_address[0],
                          self.log_date_time_string(),
                          format % args))

    def end(self):
        self.wfile.close()

    def sendDefaultHeaders(self, contents, isJSON = True):
        if isJSON:
            self.send_header('Content-Type', 'application/json')
        else:
            self.send_header('Content-Type', 'text/plain')
        # disable caching
        self.send_header('Cache-Control', 'no-store');
        self.send_header('Connection', 'close');
        if contents != None:
            self.send_header('Content-Length', len(contents));

    def serveError(self):
        contents = ""
        self.send_response(404)
        self.sendDefaultHeaders(contents)
        self.end_headers()
        self.end();

    def serveBody(self, response, qs):
        self.wfile.write(toBytes(response))
        self.end()

    def serveStatus(self, qs):
        return state.getStatus(qsExtractFloat(qs, "timestamp"))

    def serveRadioTestControl(self, qs):
        return state.serveRadioTestControl(qsExtractFloat(qs, "timestamp"),
                                           qsExtractFloat(qs, "t"),
                                           unquote(qsExtractString(qs, "command")).split(' ')[0],
                                           unquote(qsExtractString(qs, "command")),
                                           qsExtractBool(qs, "sync"))

    def serveReadings(self, qs):
        return state.getReadings(
            qsExtractString(qs, "sensors", ""),
            qsExtractFloat(qs, "timestamp"),
            qsExtractString(qs, "faulty", ""))

    def serveOutput(self, qs):
        return state.getOutput(qsExtractFloat(qs, "timestamp"))

    def processGet(self, path, qs):
        isJSON = True
        if path == '/status':
            response = self.serveStatus(qs)
        elif path == '/control':
            response = self.serveRadioTestControl(qs)
        elif path == '/readings':
            response = self.serveReadings(qs)
        else:
            isJSON = False
            response = self.serveOutput(qs)

        if isJSON:
            response = enc.encode(response)

        self.send_response(200)
        self.sendDefaultHeaders(response, isJSON)
        # to enable cross-site scripting
        self.send_header("Access-Control-Allow-Origin", "*");
        self.end_headers()
        self.serveBody(response, qs)

    def processPostAction(self, qs):
        action = qsExtractString(qs, "action", "program")

        contentLength = int(self.headers.get('content-length', 0))
        contents = self.rfile.read(contentLength) #.decode("utf-8")

        if action == "program":
            state.processProgrammingRequest(self, contents, qs)
        elif action == "configure":
            state.processReconfigurationRequest(self, contents, qs)
        else:
            log(LOG_ERROR, "Received unknown POST action, ignoring: " + action)

    def do_GET(self):
        o = urlparse(self.path)
        qs = parse_qs(o.query)

        if o.path == "/" or o.path in ["/status", "/readings", "/control", "/output"]:
            self.processGet(o.path, qs)
        else:
            self.serveError()

    def do_POST(self):
        o = urlparse(self.path)
        qs = parse_qs(o.query)

        if o.path == "/post":
            self.processPostAction(qs)
        else:
            self.serveError()

    def do_OPTIONS(self):
        self.send_response(200)
        self.send_header("Access-Control-Allow-Methods", "GET, POST, OPTIONS")
        self.send_header("Access-Control-Allow-Origin", "*");
        self.send_header("Access-Control-Allow-Headers", "content-type")
        self.send_header("Content-Type", "text/plain")
        self.send_header("Content-Length", "0")
        self.end_headers();
        self.end()

    def respondToPost(self, response):
        #print("respond to post:")
        #print(response)
        self.send_response(200)
        self.sendDefaultHeaders(response)
        # to enable cross-site scripting
        self.send_header("Access-Control-Allow-Origin", "*");
        self.end_headers()
        self.serveBody(response, qs = {})

################################################

# Note: this is a single-threaded server, to keep things simple!
class InterruptibleHTTPServer(HTTPServer):
    def __init__(self, address, handler):
        HTTPServer.__init__(self, address, handler)
        # for Python 2.6 compatibility
        if not hasattr(self, '_BaseServer__shutdown_request'):
            self._BaseServer__shutdown_request = False

    # Overrides BaseServer function to get a better control over interrupts
    def serve_forever(self, poll_interval = 0.5):
        """Handle one request at a time until shutdown.

        Polls for shutdown every poll_interval seconds. Ignores
        self.timeout. If you need to do periodic tasks, do them in
        another thread.
        """
        self._BaseServer__is_shut_down.clear()
        try:
            while not self._BaseServer__shutdown_request:
                # XXX: Consider using another file descriptor or
                # connecting to the socket to wake this up instead of
                # polling. Polling reduces our responsiveness to a
                # shutdown request and wastes cpu at all other times.
                r, w, e = select.select([self], [], [], poll_interval)
                if self in r:
                    self._handle_request_noblock()
        except Exception as e:
            log(LOG_ERROR, "base server exception:")
            log(LOG_ERROR, str(e))
            log(LOG_ERROR, traceback.format_exc())
        finally:
            self._BaseServer__shutdown_request = False
            self._BaseServer__is_shut_down.set()
            if os.name == "posix":
                # kill the process to make sure it exits
                os.kill(os.getpid(), signal.SIGKILL)

################################################

def main():
    global config

    configFileName = DEFAULT_CONFIG_FILE
    if len(sys.argv) > 1:
        configFileName = sys.argv[1]

    # load configuration
    try:
        with open(configFileName, "r") as f:
            config = json.load(f)
    except IOError as e:
        log(LOG_ERROR, "\nException occurred while trying to load config file: " + str(e))
        log(LOG_INFO, "Going to use default configuration!\n")

    try:
        # start the server
        with open(getConfigSetting("logFile"), "a+") as f:
            now = datetime.datetime.fromtimestamp(time.time()).strftime('%Y-%m-%d %H:%M:%S')
            f.write("============= ")
            f.write(now)
            f.write("\n")
        # detach from controlling tty and go to background
        if getConfigSetting("daemonize"):
            daemon.createDaemon(getConfigSetting("logFile"))
        # start the server
        port = int(getConfigSetting("httpPort"))
        log(LOG_INFO, "starting server, port: " + str(port))
        server = InterruptibleHTTPServer(('', port), HttpServerHandler)
        # start listening thread
        reconnectSerial()
        # start sending commands through the serial port
        state.cmdSender.process()
        # report ok and enter the main loop
        log(LOG_INFO, "<gateway>: started, listening to TCP port {0}".format(port))
        server.serve_forever()
    except Exception as e:
        log(LOG_ERROR, "<gateway>: exception occurred:")
        log(LOG_ERROR, str(e))
        log(LOG_ERROR, traceback.format_exc())
        return 1
    finally:
        state.cleanup()

if __name__ == '__main__':
    main()

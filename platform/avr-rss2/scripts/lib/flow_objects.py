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
# Author: Atis Elsts, 2014-2016
#

from util import *
from component_id import *
import copy, struct

################################################

MSG_FIELD_DATA               = 1
MSG_FIELD_ADD_TASK           = 2
MSG_FIELD_SET_TASK           = 3
MSG_FIELD_SET_PARAMETERS     = 4
MSG_FIELD_DEL_TASK           = 5
MSG_FIELD_ADD_CONNECTION_OUT = 6
MSG_FIELD_DEL_CONNECTION_OUT = 7
MSG_FIELD_ADD_CONNECTION_IN  = 8
MSG_FIELD_DEL_CONNECTION_IN  = 9
MSG_FIELD_ADD_CONSTRAINT     = 10
MSG_FIELD_DEL_CONSTRAINT     = 11
MSG_FIELD_ADD_BINDING        = 12
MSG_FIELD_DEL_BINDING        = 13

TASK_FLAG_PERIODIC_MESSAGES = 1

################################################

def taskToStr(taskId, gs):
    return str(gs.tasks[taskId].taskType) + " (" + str(taskId[:3]) + "...)"

def nodeToStr(nodeId, gs):
    return str(gs.nodeIdToNodeAddress(nodeId))

class Command(object):
    def __init__(self, type, nodeId = None, taskId = None,
                 otherTaskId = None, otherTaskNodeId = None,
                 fromOutputId = None, toInputId = None,
                 constraint = None):
        self.type = type
        self.nodeId = nodeId
        self.taskId = taskId
        self.otherTaskId = otherTaskId
        self.otherTaskNodeId = otherTaskNodeId
        self.constraint = constraint
        self.fromOutputId = fromOutputId
        self.toInputId = toInputId

    def getDescription(self, gs):
        d = ""
        try:
            d = "on node " + nodeToStr(self.nodeId,gs) + ": "
            if self.type == MSG_FIELD_ADD_TASK:
                d += "add task " + taskToStr(self.taskId,gs)
            elif self.type == MSG_FIELD_DEL_TASK:
                d += "remove task " + taskToStr(self.taskId,gs)
            elif self.type == MSG_FIELD_SET_TASK:
                d += "set task period " + taskToStr(self.taskId,gs)
            elif self.type == MSG_FIELD_SET_PARAMETERS:
                d += "set task parameters " + taskToStr(self.taskId,gs)
            elif self.type == MSG_FIELD_ADD_CONNECTION_OUT:
                d += "add outgoing connection from " + taskToStr(self.taskId,gs) + " to " + taskToStr(self.otherTaskId,gs)
            elif self.type == MSG_FIELD_DEL_CONNECTION_OUT:
                d += "remove outgoing connection from " + taskToStr(self.taskId,gs) + " to " + taskToStr(self.otherTaskId,gs)
            elif self.type == MSG_FIELD_ADD_CONNECTION_IN:
                d += "add incoming connection from " + taskToStr(self.taskId,gs) + " to " + taskToStr(self.otherTaskId,gs)
            elif self.type == MSG_FIELD_DEL_CONNECTION_IN:
                d += "remove incoming connection from " + taskToStr(self.taskId,gs) + " to " + taskToStr(self.otherTaskId,gs)
            elif self.type == MSG_FIELD_ADD_BINDING:
                d += "add binding for " + taskToStr(self.otherTaskId,gs) + " on " + nodeToStr(self.otherTaskNodeId,gs)
            elif self.type == MSG_FIELD_DEL_BINDING:
                d += "remove binding for " + taskToStr(self.otherTaskId,gs)
            elif self.type == MSG_FIELD_ADD_CONSTRAINT:
                d += "add constraint from " + taskToStr(self.otherTaskId,gs) + " to " + taskToStr(self.taskId,gs)
            elif self.type == MSG_FIELD_DEL_CONSTRAINT:
                d += "remove constraint from " + taskToStr(self.otherTaskId,gs) + " to " + taskToStr(self.taskId,gs)
        except:
            pass
        return d

    def getBinary(self, gs):
        self.gs = gs
        if self.type == MSG_FIELD_ADD_TASK:
            return self.cmdAddSetTask()
        elif self.type == MSG_FIELD_DEL_TASK:
            return self.cmdDelTask()
        elif self.type == MSG_FIELD_SET_TASK:
            return self.cmdAddSetTask()
        elif self.type == MSG_FIELD_SET_PARAMETERS:
            return self.cmdAddSetTask()
        elif self.type in [MSG_FIELD_ADD_CONNECTION_OUT, MSG_FIELD_ADD_CONNECTION_IN]:
            return self.cmdAddConnection()
        elif self.type in [MSG_FIELD_DEL_CONNECTION_OUT, MSG_FIELD_DEL_CONNECTION_IN]:
            return self.cmdDelConnection()
        elif self.type == MSG_FIELD_ADD_BINDING:
            return self.cmdAddDelBinding()
        elif self.type == MSG_FIELD_DEL_BINDING:
            return self.cmdAddDelBinding()
        elif self.type == MSG_FIELD_ADD_CONSTRAINT:
            return self.cmdAddConstraint()
        elif self.type == MSG_FIELD_DEL_CONSTRAINT:
            return self.cmdDelConstraint()
        return None

    def cmdAddSetTask(self):
        nodeAddr = self.gs.nodeIdToNodeAddress(self.nodeId)
        taskFunctionId = self.gs.taskIdToFunctionId(self.taskId)
        taskNr = self.gs.allocateTaskNumber(self.taskId)

        task = self.gs.tasks[self.taskId]
        param1 = param2 = param3 = param4 = 0
        try:
            param1 = int(task.parameters[0])
            param2 = int(task.parameters[1])
            param3 = int(task.parameters[2])
            param4 = int(task.parameters[3])
        except:
            pass
        #print "params:", task.period, param1, param2, param3, param4, task.flags
        return struct.pack("<HBBHHLHHHH", nodeAddr, self.type,
                           task.flags, taskFunctionId, taskNr,
                           task.period, param1, param2, param3, param4)

    def cmdDelTask(self):
        nodeAddr = self.gs.nodeIdToNodeAddress(self.nodeId)
        taskNr = self.gs.allocateTaskNumber(self.taskId)
        return struct.pack("<HBH", nodeAddr, self.type, taskNr)

    def cmdAddConnection(self):
        nodeAddr = self.gs.nodeIdToNodeAddress(self.nodeId)
        srcTaskNr = self.gs.allocateTaskNumber(self.taskId)
        dstTaskNr = self.gs.allocateTaskNumber(self.otherTaskId)
        radioChannel = 0 # TODO
        return struct.pack("<HBHHBB", nodeAddr, self.type,
                           srcTaskNr, dstTaskNr,
                           (self.fromOutputId << 4) | self.toInputId, radioChannel)

    def cmdDelConnection(self):
        nodeAddr = self.gs.nodeIdToNodeAddress(self.nodeId)
        srcTaskNr = self.gs.allocateTaskNumber(self.taskId)
        dstTaskNr = self.gs.allocateTaskNumber(self.otherTaskId)
        radioChannel = 0 # TODO
        return struct.pack("<HBHHBB", nodeAddr, self.type,
                           srcTaskNr, dstTaskNr,
                           (self.fromOutputId << 4) | self.toInputId, radioChannel)

    def cmdAddDelBinding(self):
        nodeAddr = self.gs.nodeIdToNodeAddress(self.nodeId)
        dstTaskNr = self.gs.allocateTaskNumber(self.otherTaskId)
        taskNodeAddr = self.gs.nodeIdToNodeAddress(self.otherTaskNodeId)
        return struct.pack("<HBHH", nodeAddr, self.type,
                           dstTaskNr, taskNodeAddr)

    def cmdAddConstraint(self):
        nodeAddr = self.gs.nodeIdToNodeAddress(self.nodeId)
        constraintNr = self.gs.allocateConstraintNumber(self.constraint.id)
        srcTaskNr = self.gs.allocateTaskNumber(self.otherTaskId)
        dstTaskNr = self.gs.allocateTaskNumber(self.taskId)
        pdr = self.constraint.pdr if self.constraint.type == "pdr" else 0
        delay = self.constraint.delay if self.constraint.type == "delay" else 0
        return struct.pack("<HBBHHBBHB", nodeAddr, self.type,
                           constraintNr, srcTaskNr, dstTaskNr,
                           self.constraint.isMax, pdr, delay,
                           self.constraint.minPackets)

    def cmdDelConstraint(self):
        nodeAddr = self.gs.nodeIdToNodeAddress(self.nodeId)
        constraintNr = self.gs.allocateConstraintNumber(self.constraint.id)
        dstTaskNr = self.gs.allocateTaskNumber(self.otherTaskId)
        return struct.pack("<HBBH", nodeAddr, self.type,
                           constraintNr, dstTaskNr)

################################################

class FlowObject(object):
    def __init__(self, id):
        self.isAlive = True
        self.old = None
        self.id = id
        self.json = None

    @staticmethod
    def create(type, id):
        if type == "Task": return Task(id)
        if type == "Constraint": return Constraint(id)
        if type == "Node": return Node(id)

    def advanceGeneration(self):
        self.old = copy.copy(self)
        self.json = None
        self.isAlive = False

    def setJSON(self, json):
        self.json = json
        self.isAlive = True

    def garbageCollect(self):
        self.old = None
        return not self.isAlive

################################################

class Constraint(FlowObject):
    def __init__(self, id):
        super(Constraint, self).__init__(id)
        self.isMax = ""
        self.type = "pdr"
        self.pdr = 0    # XXX: in percent, small granularity
        self.delay = 0  # XXX: in milliseconds, very short dynamic range
        self.energy = 0 # XXX: not supported by the runtime for now
        self.minPackets = 0

    def setJSON(self, json):
        if self.id.endswith("delay"):
            self.setDelay(json)
        elif self.id.endswith("pdr"):
            self.setPDR(json)
        elif self.id.endswith("energy"):
            self.setEnergy(json)
        FlowObject.setJSON(self, json)

    def setDelay(self, c):
        self.type = "delay"
        self.name = c["name"]
        self.isMax = c["delayType"] == "max"
        self.delay = int(c["delayValue"])

    def setPDR(self, c):
        self.type = "pdr"
        self.name = c["name"]
        self.isMax = c["pdrType"] == "max"
        self.pdr = int(c["pdrValue"])

    def setEnergy(self, c):
        self.type = "energy"
        self.name = c["name"]
        self.isMax = c["pdrType"] == "max"
        self.energy = int(c["energyValue"])

    def findReferences(self, gs):
        pass

    def setExtraArgs(self, minPackets):
        self.minPackets = minPackets

    def getChangeCommands(self, nodeId, taskId, otherTaskId):
        result = []
        if not (self.pdr == self.old.pdr and \
                self.delay == self.old.delay and \
                self.energy == self.old.energy and \
                self.isMax == self.old.isMax and \
                self.type == self.old.type):
            result.append(Command(MSG_FIELD_DEL_CONSTRAINT, nodeId, taskId,
                                  otherTaskId = otherTaskId, constraint = self))
            result.append(Command(MSG_FIELD_ADD_CONSTRAINT, nodeId, taskId,
                                  otherTaskId = otherTaskId, constraint = self))

        return result

################################################

class Task(FlowObject):
    def __init__(self, id):
        super(Task, self).__init__(id)
        self.incomingLinks = set()
        self.outgoingLinks = set()
        self.runtimeId = None
        self.taskType = None
        self.period = 1
        self.parameters = None
        self.flags = 0

    def setJSON(self, json):
        self.parameters = getTaskParameters(json)

        try:
            self.period = int(json.get("period", 1000))
        except:
            self.period = 1

        if "runtimeId" in json:
            self.runtimeId = int(json["runtimeId"])
        else:
            self.runtimeId = None

        self.flags = 0
        try:
            if bool(json.get("generateTestPackets", False)):
                self.flags |= TASK_FLAG_PERIODIC_MESSAGES
        except:
            pass

        self.taskType = json.get("type", "")

        self.incomingLinks = set()
        self.outgoingLinks = set()
        for dstArr in json.get("wires", []):
            for dst in dstArr:
                taskId = dst["task"]
                constraintName = dst.get("constraintName", None)
                if "sourcePort" in dst:
                    sourcePort = int(dst["sourcePort"])
                else:
                    sourcePort = 0
                if "targetPort" in dst:
                    targetPort = int(dst["targetPort"])
                else:
                    targetPort = 0
                self.outgoingLinks.add((taskId, constraintName, sourcePort, targetPort))

        FlowObject.setJSON(self, json)

    def addIncomingLink(self, l, otherTask, gs):
        if l[1]:
            constraints = \
                (gs.constraints.get(l[1] + " delay", None),
                 gs.constraints.get(l[1] + " pdr", None),
                 gs.constraints.get(l[1] + " energy", None))
        else:
            constraints = (None, None, None)
        sourcePort = l[2]
        targetPort = l[3]
        self.incomingLinks.add((otherTask, constraints, sourcePort, targetPort))

    def findReferences(self, gs):
        for l in self.outgoingLinks:
            task = gs.tasks.get(l[0], None)
            if task:
                task.addIncomingLink(l, self.id, gs)


    def getAddCommands(self, nodeId):
        # include self
        # include all connections to which this is the source
        # include all constraints to which this is the dst
        result = []

        result.append(Command(MSG_FIELD_ADD_TASK, nodeId, self.id))

        for l in self.incomingLinks:
            result.append(Command(MSG_FIELD_ADD_CONNECTION_IN, nodeId, l[0],
                                  otherTaskId = self.id,
                                  fromOutputId = l[2], toInputId = l[3]))
            for c in l[1]:
                if c:
                    result.append(Command(MSG_FIELD_ADD_CONSTRAINT, nodeId, self.id,
                                          otherTaskId = l[0], constraint = c))

        for l in self.outgoingLinks:
            result.append(Command(MSG_FIELD_ADD_CONNECTION_OUT, nodeId, self.id,
                                  otherTaskId = l[0],
                                  fromOutputId = l[2], toInputId = l[3]))

        return result


    def getRemoveCommands(self, nodeId):
        # include only self
        return [Command(MSG_FIELD_DEL_TASK, nodeId, self.id)]


    def getChangeCommands(self, nodeId):
        # include SET command for period change
        # include SET_PARAMETERS command for parameter change
        # include add/remove commands for all changed constraints and connections
        result = []

        # print "getChangeCommands, period=", self.period, " old=", self.old.period

        if self.period != self.old.period:
            result.append(Command(MSG_FIELD_SET_TASK, nodeId, self.id))

        if self.parameters != self.old.parameters:
            result.append(Command(MSG_FIELD_SET_PARAMETERS, nodeId, self.id))

        # XXX: for now a connection is removed+added also if only the constraint name changes
        # should ignore that for efficiency
        removedOut = self.old.outgoingLinks.difference(self.outgoingLinks)
        addedOut = self.outgoingLinks.difference(self.old.outgoingLinks)

        for l in removedOut:
            result.append(Command(MSG_FIELD_DEL_CONNECTION, nodeId, self.id,
                                  otherTaskId = l[0],
                                  fromOutputId = l[2], toInputId = l[3]))
        for l in addedOut:
            result.append(Command(MSG_FIELD_ADD_CONNECTION, nodeId, self.id,
                                  otherTaskId = l[0],
                                  fromOutputId = l[2], toInputId = l[3]))

        removedIn = self.old.incomingLinks.difference(self.incomingLinks)
        addedIn = self.incomingLinks.difference(self.old.incomingLinks)

        for l in removedIn:
            for c in l[1]:
                if c:
                    result.append(Command(MSG_FIELD_DEL_CONSTRAINT, nodeId, self.id,
                                          otherTaskId = l[0], constraint = c))
        for l in addedIn:
            for c in l[1]:
                if c:
                    result.append(Command(MSG_FIELD_ADD_CONSTRAINT, nodeId, self.id,
                                          otherTaskId = l[0], constraint = c))

        for l in self.incomingLinks:
            if l not in addedIn:
                for c in l[1]:
                    if c:
                        result += c.getChangeCommands(nodeId, self.id, l[0])

        return result


################################################

class Node(FlowObject):
    def __init__(self, id):
        super(Node, self).__init__(id)
        self.taskIds = {}
        self.tasks = {}
        self.bindings = {}
        self.address = 0
        self.coojaId = 0

    def getIdentifier(self, isSimulatedNet):
        if isSimulatedNet:
            return self.coojaId
        return self.address

    def setJSON(self, json):
        try:
            # user config
            self.address = int(json.get("address", None))
        except:
            self.address = 0
        try:
            # simulator's auto-assigned value
            self.coojaId = int(json.get("coojaId", None))
        except:
            self.coojaId = 0
        self.taskIds = arrayToDict(json.get("mappedTasks", []), "task")
        self.tasks = {}
        self.bindings = {}
        FlowObject.setJSON(self, json)

    def findReferences(self, gs):
        for taskId in self.taskIds:
            self.tasks[taskId] = gs.tasks[taskId]

    def getBindings(self):
        result = set()
        mappedTasks = self.json.get("mappedTasks", [])
        for t in mappedTasks:
            for d in t["destinations"]:
                if d["node"] != self.id: # only for remote nodes
                    result.add((d["task"], d["node"]))
        return result

    def getCommands(self, allNodes):
        if not self.isAlive:
            return []

        removedTasks = []
        addedTasks = []

        if self.old:
            for taskId in self.old.tasks:
                if taskId not in self.tasks:
                    removedTasks.append(taskId)
        for taskId in self.tasks:
            if not (self.old and taskId in self.old.tasks):
                addedTasks.append(taskId)

        commands = []
        for id in removedTasks:
            commands += self.old.tasks[id].getRemoveCommands(self.id)
        for id in addedTasks:
            commands += self.tasks[id].getAddCommands(self.id)

        for id in self.tasks:
            if id not in addedTasks:
                commands += self.tasks[id].getChangeCommands(self.id)

        oldBindings = self.old.getBindings() if self.old else set()
        currentBindings = self.getBindings()

        removedBindings = oldBindings.difference(currentBindings)
        addedBindings = currentBindings.difference(oldBindings)

        for elem in removedBindings:
            commands.append(Command(MSG_FIELD_DEL_BINDING, self.id,
                                    otherTaskId = elem[0], otherTaskNodeId = elem[1]))
        for elem in addedBindings:
            commands.append(Command(MSG_FIELD_ADD_BINDING, self.id,
                                    otherTaskId = elem[0], otherTaskNodeId = elem[1]))

        return commands

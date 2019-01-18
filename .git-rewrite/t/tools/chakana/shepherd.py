import re
import socket
import threads
import xml.dom.minidom
import thread

import chakana.error
import chakana.event
import chakana.monitor
import chakana.threads
import chakana.utils
from chakana.debug import *

class Shepherd:
  """The shepherd is a singleton class that manages the connection to COOJA
  and keeps track of user monitor threads.  The user initialises a
  debugging scenario by creating a Shepherd object, and then calls the
  newMonitor method to create Monitor objects, one for each causal
  path through the system that he wishes to monitor.

  The monitor threads automatically register with the shepherd.  The user
  can use the debugger instances to probe variables or create eventpoints.
  When the monitor thread routine decides that is ready for simulation to
  proceed, it calls the waitFor method to block until one of the
  eventpoints occur.

  When a Monitor object enters waitFor, it informs shepherd about the
  eventpoints that the user is waiting for, and waits for the eventpoints
  to happen.  When all threads enter waiting state, the shepherd asks COOJA
  to resume simulation.  When an eventpoint is hit, the shepherd wakes the
  Monitor that is waiting for that eventpoint.

  """

  def __init__(self, threadManager, port, coojaHost = "localhost"):
    debug(MajorEvent, "Creating shepherd")
    self._pollConnLock = threading.Lock()
    self._threadManager = threadManager
    self._port = port
    self._monitors = []
    self._waitingMonitors = []
    # Only basic eventpoints in these lists
    self._eventpoints = []
    self._waitingEventpoints = []
    self._coojaContinueEvent = threading.Event()
    self._waitMap = {}
    self._coojaHost = coojaHost
    self._coojaConnection = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    debug(MajorEvent, "Connecting to COOJA at " + coojaHost + ":" +
          str(self._port))
    self._connectionLock = threading.Lock()
    self._coojaConnection.connect((self._coojaHost, self._port))
    self._coojaStream = self._coojaConnection.makefile()
    self._coojaConnection.close()
    del self._coojaConnection
    self._connectionLock.acquire()
    hello = self.readFromCooja()
    debug(Debug, hello.toprettyxml())
    self._connectionLock.release()
    self._shepherdThread = threads.ManagedThread(
      self._threadManager, target = self._shepherdMainLoop)
    self._shepherdThread.start()
    debug(Debug, "Shepherd created:\n" + repr(self))

  def _shepherdMainLoop(self):
    debug(Event, "Starting shepherd main loop")
    while 1:
      debug(Debug, "Waiting for eventpoints to enter wait state")
      debug(Debug2, repr(self))
      self._coojaContinueEvent.wait()
      self._coojaContinueEvent.clear()
      if self._coojaStream.closed:
        debug(Event, "Terminating shepherd main loop")
        return
      debug(Debug, "Resuming COOJA")
      assert(len(self._eventpoints) == len(self._waitingEventpoints))
      response = self.runCommand("CONTROL_SIM", { "RESUME" : None })
      id = int(response.documentElement.childNodes[0].nodeValue)
      debug(MinorEvent, "Eventpoint " + str(id) + " was triggered")

      # Wake up triggered eventpoint
      self._waitMap[id][1].set()
      
      # Allow new main loop startups
      self._pollConnLock.release()
      

  def quit(self):
    """Send a termination request to the debugger broker, and discard all
    debuggers."""

    debug(Information, "Terminating the shepherd session")
    self.runCommand("EXIT_COOJA")
    self._connectionLock.acquire()
    self._coojaStream.close()
    self._connectionLock.release()
    self._coojaContinueEvent.set()
    debug(Information, "Number of PID errors reported: " + str(self.threadManager().nrPIDerrors()))

    for (eventpoint, event) in self._waitMap.values():
      debug(Debug2, "Awakening eventpoint at exit: " + repr(eventpoint))
      event.set()

  def loadConfiguration(self, fileName):
    return self.runCommand(
      "CREATE_SIM", xmlContent = chakana.utils.readFile(fileName))

  def loadConfigurationXML(self, xmlContent):
    return self.runCommand(
      "CREATE_SIM", xmlContent = xmlContent)

  def setConfigurationXML(self, xmlContent):
    return self.runCommand(
      "CONF_SIM", xmlContent = xmlContent)

  def setPluginsXML(self, xmlContent):
    return self.runCommand(
      "CONF_PLUGINS", xmlContent = xmlContent)

  def newMonitor(self, routine, * args, ** kwArgs):
    return self.newCustomMonitor(
      chakana.monitor.Monitor, routine, * args, ** kwArgs)

  def newCustomMonitor(self, Type, routine = None, * args, ** kwArgs):
    """Start a new monitor thread running routine, with * args and ** kwArgs
    as arguments."""
    debug(Debug, "Creating new monitor of type " + Type.__name__)
    debug(Debug3, "Routine: " + repr(routine) + ", arguments: " + repr(args) +
          ", keyword arguments: " + repr(kwArgs))
    thread = Type(self, routine, * args, ** kwArgs)
    debug(MinorEvent, "New monitor thread created: " + repr(thread))
    return thread

  def newTimeoutMonitor(self, eventpoint, timeout, routine = None):
    return self.newCustomMonitor(
      chakana.monitor.TimeoutWrapperMonitor, routine, eventpoint = eventpoint, timeout = timeout)

  def threadManager(self):
    return self._threadManager

  def registerMonitor(self, monitor):
    debug(Debug, "Registering monitor: " + repr(monitor))
    assert(isinstance(monitor, chakana.monitor.Monitor))
    assert(not (monitor in self._monitors))
    assert(not (monitor in self._waitingMonitors))
    self._monitors.append(monitor)
    self._monitors.sort()
    debug(Debug2, repr(self))

  def unregisterMonitor(self, monitor):
    debug(Debug, "Unregistering monitor: " + repr(monitor))
    assert(isinstance(monitor, chakana.monitor.Monitor))
    assert(monitor in self._monitors)
    assert(not (monitor in self._waitingMonitors))
    self._monitors.remove(monitor)

  def registerWaitingMonitor(self, monitor):
    debug(Debug, "Registering waiting monitor: " + repr(monitor))
    assert(isinstance(monitor, chakana.monitor.Monitor))
    assert(monitor in self._monitors)
    assert(not (monitor in self._waitingMonitors))
    self._waitingMonitors.append(monitor)
    self._waitingMonitors.sort()
    debug(Debug2, repr(self))

  def unregisterWaitingMonitor(self, monitor):
    debug(Debug, "Unregistering waiting monitor: " + repr(monitor))
    assert(isinstance(monitor, chakana.monitor.Monitor))
    assert(monitor in self._monitors)
    assert(monitor in self._waitingMonitors)
    self._waitingMonitors.remove(monitor)
    debug(Debug2, repr(self))

  def registerBasicEventpoint(self, eventpoint):
    debug(Debug2, "Registering basic eventpoint: " + repr(eventpoint))
    assert(isinstance(eventpoint, chakana.event.BasicEventpoint))
    assert(not (eventpoint in self._eventpoints))
    self._eventpoints.append(eventpoint)
    self._eventpoints.sort()
    debug(Debug2, repr(self))

  def unregisterBasicEventpoint(self, eventpoint):
    debug(Debug2, "Unregistering basic eventpoint: " + repr(eventpoint))
    assert(not (eventpoint in self._waitingEventpoints))
    assert(eventpoint in self._eventpoints)
    self._eventpoints.remove(eventpoint)
    debug(Debug2, repr(self))

  def _registerWaitingEventpoint(self, eventpoint):
    debug(Debug2, "Registering waiting eventpoint: " + repr(eventpoint))
    assert(not (eventpoint in self._waitingEventpoints))
    assert(eventpoint in self._eventpoints)
    self._waitingEventpoints.append(eventpoint)
    self._waitingEventpoints.sort()

  def _unregisterWaitingEventpoint(self, eventpoint):
    debug(Debug2, "Unregistering waiting eventpoint: " + repr(eventpoint))
    assert(eventpoint in self._waitingEventpoints)
    assert(eventpoint in self._eventpoints)
    self._waitingEventpoints.remove(eventpoint)
    debug(Debug2, repr(self))

  def pollContinuation(self):
    self._pollConnLock.acquire()
    if len(self._waitingMonitors) != len(self._monitors):
      self._pollConnLock.release()
      return
    for monitor in self._monitors:
      if not monitor._ready:
        self._pollConnLock.release()
        return

    for monitor in self._monitors:
      assert(not monitor._discarded)
    
    for eventpoint in self._eventpoints:
      assert(not eventpoint._discarded)
    
    assert(self._waitingMonitors == self._monitors)
    assert(self._waitingEventpoints == self._eventpoints)
    debug(MinorEvent, "All monitors are waiting, activating COOJA")
    assert(not self._coojaContinueEvent.isSet())
    self._coojaContinueEvent.set()

  def waitForCoojaEventpoint(self, eventpoint):
    response = self.runCommand("ADD_EVENTPOINT", eventpoint.coojaArguments())
    id = int(response.documentElement.childNodes[0].nodeValue)
    debug(Debug, "Waiting for COOJA eventpoint " + str(id))

    resourceAllocated = 0
    while resourceAllocated == 0:
      try:
        self._waitMap[id] = (eventpoint, threading.Event())
        resourceAllocated = 1
      except thread.error:
        resourceAllocated = 0
        self.threadManager().registerPIDerror()

    self._registerWaitingEventpoint(eventpoint)
    eventpoint._isWaitingEvent.set()

    resourceAllocated = 0
    while resourceAllocated == 0:
      try:
        self._waitMap[id][1].wait()
        resourceAllocated = 1
      except thread.error:
        resourceAllocated = 0
        self.threadManager().registerPIDerror()

    del self._waitMap[id]
    self._unregisterWaitingEventpoint(eventpoint)
    try:
      self.runCommand("DELETE_EVENTPOINT", { "id" : id })
      debug(Debug, "COOJA event " + str(id) + " has occurred")
    except chakana.error.CoojaExit:
      pass
    except socket.error:
      pass

  def runCommand(self, name, args = None, xmlContent = ""):
    debug(Debug3, "Running cooja command " + name + ", args: " + repr(args) +
          " xml content: " + repr(xmlContent))
    if args is None:
      args = {}
    self._connectionLock.acquire()
    try:
      if self._coojaStream.closed:
        raise chakana.error.CoojaExit()
      command = ['<command value="' + name + '">']
      for (key, value) in args.items():
        if value is None:
          command.append('<' + key + '/>')
        else:
          command.append('<' + key + '>' + str(value) + '</' + key + '>')
      command.append(xmlContent + '</command>')
      commandStr = "\n".join(command + ['\n'])
      debug(MinorEvent, '--> ' + commandStr)
      self._coojaStream.write(commandStr)
      self._coojaStream.flush()
      response = self.readFromCooja()
      debug(Debug, response.toprettyxml())
      if response.documentElement.tagName == 'error':
        raise chakana.error.CoojaError(response)
      self._connectionLock.release()
      return response
    except socket.error:
      debug(MajorEvent, 'Socket error catched')
    except AttributeError:
      debug(MajorEvent, 'Attribute error catched')

  def readFromCooja(self):
    # XXX: Assume message ends with a newline
    debug(Debug, "Reading message from COOJA")
    responseLines = [self._coojaStream.readline()]
    debug(Debug2, "First line: " + repr(responseLines[0]))
    (rootElement, slash) = re.match(r"^\s*<(\w+)(/?)>",
                                    responseLines[0]).groups()
    debug(Debug3, "Root element: " + rootElement)
    if slash != "/":
      while 1:
        if re.search("</" + re.escape(rootElement) + ">$", responseLines[-1],
                     re.M):
          break
        responseLines.append(self._coojaStream.readline())
        debug(Debug3, "Read line: " + repr(responseLines[-1]))
    result = "".join(responseLines)
    debug(MinorEvent, '<-- ' + result)
    return xml.dom.minidom.parseString(result)
    
  def readVariable(self, variable, ** kwArgs):
    response = self.readMemory(
      type = "variable", variable = variable, ** kwArgs)
    arrayString = response.documentElement.childNodes[0].nodeValue.split()
    bytes = map(eval, arrayString)
    debug(Debug, "Read variable " + variable + " as byte array: " +
          repr(bytes))
    return bytes    

  def readInt(self, ** kwArgs):
    response = self.readMemory(type = "int", ** kwArgs)
    return eval(response.documentElement.childNodes[0].nodeValue)

  def readMemory(self, ** kwArgs):
    return self.runCommand("READ_MEMORY", kwArgs)

  def killNodesInInterval(self, ** kwArgs):
    return self.runCommand("KILL_NODES", kwArgs)

  def sendCustomCommand(self, ** kwArgs):
    return self.runCommand("CUSTOM_COMMAND", kwArgs)

  def getSimulationInfo(self, ** kwArgs):
    response = self.runCommand("GET_INFO", kwArgs)
    return eval(response.documentElement.childNodes[0].nodeValue)

  def __repr__(self):
    return "\n  ".join([
      "Shepherd:",
      "Port: " + repr(self._port),
      "Monitors: " + repr(self._monitors),
      "Waiting monitors: " + repr(self._waitingMonitors),
      "Basic eventpoints: [" + "\n".join(map(repr, self._eventpoints)) + "]",
      "Waiting eventpoints: [" + "\n".join(
      map(repr, self._waitingEventpoints)) + "]",
      "Wait map: " + repr(self._waitMap),
      ])

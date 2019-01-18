import Queue
import thread

import chakana.event
import chakana.threads
from chakana.debug import *

class Monitor(chakana.threads.ManagedThread):
  """Base class for causal path monitors.  Pass the threadManager to the
  constructor.  Override the doRun method or pass a routine argument.  In
  the latter case, the routine will be called with the Monitor object as
  argument plus any extra arguments and keyword arguments specified."""
  
  def __init__(self, shepherd, routine = None, * args, ** kwArgs):
    chakana.threads.ManagedThread.__init__(self, shepherd.threadManager())
    self._shepherd = shepherd
    self._routine = routine
    self._args = args
    self._kwArgs = kwArgs
    self._ready = 0
    self._discarded = 0
    resourceAllocated = 0
    while resourceAllocated == 0:
      try:
        self._isWaitingMonitor = threading.Event()
        resourceAllocated = 1
      except thread.error:
        resourceAllocated = 0
        shepherd.threadManager().registerPIDerror()
    debug(Debug, "Monitor created")
    self._shepherd.registerMonitor(self)

  def startMonitor(self):
    self.start()
    debug(Debug, "New monitor thread started: " + repr(self))

  def discard(self):
    self._discarded = 1
    self._ready = 0
    debug(Debug, "Discarding monitor:\n" + repr(self))

    # Wake up thread with empty results
    if hasattr(self, "_resultQueue"):
      self._resultQueue.put(None)

  def waitMonitor(self, parentEventpoint = None):
    debug(Debug, "Waiting for monitor to finish: " + repr(self))

    # While monitor is registered, check for continuations
    while self in self.shepherd()._monitors:
      resourceAllocated = 0
      while resourceAllocated == 0:
        try:
          self._isWaitingMonitor.wait()
          resourceAllocated = 1
        except thread.error:
          resourceAllocated = 0
          self.shepherd().threadManager().registerPIDerror()

      self._isWaitingMonitor.clear()
      self._ready = 1

      # If all monitors ready, poll continuation
      allMonitorsReady = 1
      for monitor in self.shepherd()._monitors:
        if not monitor._ready:
          allMonitorsReady = 0
      if allMonitorsReady:
        if self in self.shepherd()._monitors:
          self.shepherd().pollContinuation()
      else:
        # A monitor may be waiting for us, tell parent we are ready
        if parentEventpoint is not None:
          parentEventpoint._isWaitingEvent.set()
    self.join()

  def doRun(self):
    debug(Debug, "Running monitor")
    try:
      if (not self._discarded):
        self.runMonitor()
    except chakana.error.Discarded:
      pass
    self.shepherd().unregisterMonitor(self)
    self._isWaitingMonitor.set()
    debug(Debug, "Monitor exiting")

  def runMonitor(self):
      self._routine(self, * self._args, ** self._kwArgs)

  def waitFor(self, eventList):
    """Waits until one of the Events given in 'eventList' have triggered and
    returns that Event."""
    if isinstance(eventList, chakana.event.Eventpoint):
      eventList = [eventList]
    debug(MinorEvent, "Waiting for events to trigger. Event list: " + 
          str(eventList))
    assert eventList != []

    # Clean up eventpoints and abort monitor if discarded
    if self._discarded:
      for ev in eventList:
        ev.discard()
      raise chakana.error.Discarded()

    self.shepherd().registerWaitingMonitor(self)

    resourceAllocated = 0
    while resourceAllocated == 0:
      try:
        self._resultQueue = Queue.Queue()
        resourceAllocated = 1
      except thread.error:
        resourceAllocated = 0
        self.shepherd().threadManager().registerPIDerror()

    for ev in eventList:
      # Start thread and wait until eventpoint is waiting
      ev.await(self._resultQueue)
      
      resourceAllocated = 0
      while resourceAllocated == 0:
        try:
          ev._isWaitingEvent.wait()
          resourceAllocated = 1
        except thread.error:
          resourceAllocated = 0
          self.shepherd().threadManager().registerPIDerror()
      
      ev._isWaitingEvent.clear()

    debug(Event, "Waiting for events: " + repr(eventList))

    assert(not self._discarded)

    self._isWaitingMonitor.set()
    resourceAllocated = 0
    while resourceAllocated == 0:
      try:
        triggeredEvent = self._resultQueue.get()
        resourceAllocated = 1
      except thread.error:
        resourceAllocated = 0
        self.shepherd().threadManager().registerPIDerror()

    self._ready = 0
    self.shepherd().unregisterWaitingMonitor(self)
    debug(Event, "Eventpoint was triggered: " + repr(eventList))
    for ev in eventList:
      if not (ev is triggeredEvent):
        ev.discard()
    
    return triggeredEvent

  def shepherd(self):
    return self._shepherd

  def readVariable(self, ** kwArgs):
    return self.shepherd().readVariable(** kwArgs)

  def readInt(self, ** kwArgs):
    return self.shepherd().readInt(** kwArgs)

  def readMemory(self, ** kwArgs):
    return self.shepherd().readMemory(** kwArgs)

class TimeoutWrapperMonitor(Monitor):
  def __init__(self, shepherd, routine, eventpoint = None, timeout = 1000):
    chakana.monitor.Monitor.__init__(self, shepherd)
    self._eventpoint = eventpoint
    self._timeout = timeout
    self._triggeredEventpoint = None

  def runMonitor(self):
    timeEventpoint = chakana.event.TimeEventpoint(self.shepherd(), self._timeout)
    self._triggeredEventpoint = self.waitFor([timeEventpoint, self._eventpoint])

  def getTriggeredEventpoint(self):
    return self._triggeredEventpoint

  def timedOut(self):
    return self.getTriggeredEventpoint() != self._eventpoint

    
import time
import thread

from debug import *
import threads

class Eventpoint(threads.ManagedThread):

  """ The Eventpoint class and its subclasses represent important events
  happening in the debugged system, and are the primitives that Chakana
  debugging scripts are based around.  When the user calls waitFor with an
  Eventpoint list argument, each Eventpoint thread is started.  The
  Eventpoint inserts appropriate eventpoints into COOJA or creates child
  Eventpoints.  An Eventpoint that is hit posts itself on a trigger queue
  (Queue.Queue), where it is picked up by waitFor.  The Eventpoints that
  have not been triggered are then discarded by the waitFor routine."""

  def __init__(self, shepherd):
    threads.ManagedThread.__init__(self, shepherd.threadManager())
    self._shepherd = shepherd
    self._activated = 0
    self._discarded = 0
    self._triggerQueue = None

    resourceAllocated = 0
    while resourceAllocated == 0:
      try:
        self._isWaitingEvent = threading.Event()
        resourceAllocated = 1
      except thread.error:
        resourceAllocated = 0
        shepherd.threadManager().registerPIDerror()

  def await(self, triggerQueue):
    assert not self._activated, "Eventpoints can only be used once"
    self._triggerQueue = triggerQueue
    self._activated = 1
    self.activatedHook()
    self.start()

  def doRun(self):
    debug(Debug, "Waiting for event: " + repr(self))
    if not self._discarded:
      self.doWait()
    debug(MinorEvent, "Event was triggered: " + repr(self))
    if not self._discarded:
      self._triggerQueue.put(self)
    else:
      debug(Debug, "Event is no longer waited for: " + repr(self))

  def activatedHook(self):
    pass

  def doWait(self):
    raise NotImplementedError()

  def discard(self):
    debug(Debug, "Discarding eventpoint:\n" + repr(self))
    self._discarded = 1
    if (not self._activated):
      self.start()
    for (waitingEv, event) in self.shepherd()._waitMap.values():
      if waitingEv == self:
        event.set()
    self.join()
 
  def isDiscarded(self):
    return self._discarded

  def shepherd(self):
    return self._shepherd

  def __repr__(self):
    return "\n  ".join([
      "event.Eventpoint:",
      "Id: %#x" % ((id(self) + (long(1) << 32)) % (long(1) << 32)),
      ])

class BasicEventpoint(Eventpoint):
  """Eventpoint corresponding to a single COOJA Eventpoint."""

  def __init__(self, shepherd, ** kwArgs):
    Eventpoint.__init__(self, shepherd, ** kwArgs)

  def activatedHook(self):
    self.shepherd().registerBasicEventpoint(self)

  def doWait(self):
    self.shepherd().waitForCoojaEventpoint(self)
    self.shepherd().unregisterBasicEventpoint(self)

class Watchpoint(BasicEventpoint):
  def __init__(self, shepherd, type, mote, variable, ** kwArgs):
    self._type = type
    self._mote = mote
    self._variable = variable
    BasicEventpoint.__init__(self, shepherd, ** kwArgs)
    
  def coojaArguments(self):
    return { "type" : self._type,
             "mote" : self._mote,
             "variable" : self._variable }
      
  def __repr__(self):
    return "\n  ".join([
      BasicEventpoint.__repr__(self),
      "event.Watchpoint:",
      "Type: " + repr(self._type),
      "Mote: " + repr(self._mote),
      "Variable: " + repr(self._variable)
      ])
      
class TimeEventpoint(BasicEventpoint):
  def __init__(self, shepherd, time, ** kwArgs):
    self._time = time
    BasicEventpoint.__init__(self, shepherd, ** kwArgs)

  def coojaArguments(self):
    return { "type" : "time",
             "time" : self._time }
      
  def __repr__(self):
    return "\n  ".join([
      BasicEventpoint.__repr__(self),
      "event.TimeEventpoint:",
      "Time: " + repr(self._time)
      ])


class RadioMessageCompletedEventpoint(BasicEventpoint):
  def __init__(self, shepherd, count = 1, ** kwArgs):
    self._count = count
    BasicEventpoint.__init__(self, shepherd, ** kwArgs)

  def coojaArguments(self):
    return { "type" : "radiomedium",
             "triggeron" : "completed",
             "count" : str(self._count) }

  def __repr__(self):
    return "\n  ".join([
      BasicEventpoint.__repr__(self),
      "event.RadioMessageCompletedEventpoint",
      ])

class MonitorEventpoint(Eventpoint):
  """Eventpoint that triggers after a monitor has finished."""

  def __init__(self, monitor):
    self._monitor = monitor
    Eventpoint.__init__(self, monitor.shepherd())

  def doWait(self):
    debug(Debug, "MonitorEventpoint starting " + repr(self))
    self._monitor.startMonitor()
    self._monitor.waitMonitor(self)

  def discard(self):
    self._discarded = 1
    self._monitor.discard()
    if (not self._activated):
      self.start()
      self._monitor.startMonitor()
      self._monitor.waitMonitor(self)
    else:
      self._monitor.waitMonitor(self)
    self.join()

  def __repr__(self):
    return "\n  ".join([
      Eventpoint.__repr__(self),
      "event.MonitorEventpoint:",
      "Monitor: " + repr(self._monitor)
      ])


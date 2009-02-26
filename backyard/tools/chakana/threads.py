#  
#  Copyright (C) 2005-2007 Swedish Institute of Computer Science.
#  
#  Please refer to the file named LICENSE in the same directory as this file
#  for licensing information.
#  
#  Written and maintained by Lars Albertsson <lalle@sics.se>.
#  

# $Id: threads.py,v 1.1 2009/02/26 13:47:38 fros4943 Exp $

import signal
import sys
import threading
import time
import Queue
import thread

import chakana.error
import chakana.linux
from chakana.debug import *

class ManagedThread(threading.Thread):
  def __init__(self, threadManager, ** kwArgs):
    self._threadManager = threadManager
    self._exceptionInfo = None
    resourceAllocated = 0
    while resourceAllocated == 0:
      try:
        threading.Thread.__init__(self, ** kwArgs)
        resourceAllocated = 1
      except thread.error:
        resourceAllocated = 0
        self.threadManager().registerPIDerror()

    debug(Event, "Initialising thread " + self.getName())
    self._threadManager._registerThread(self)

  def threadManager(self):
    return self._threadManager

  def start(self):
    try:
      self._threadManager
    except:
      raise RuntimeError("ManagedThread started, but not initialised properly")
    return threading.Thread.start(self)

  def run(self):
    debug(Event, self.getName() + " thread is now running")
    self.threadManager()._registerThreadStart(self)
    try:
      self.doRun()
    except Exception, err:
      debug(MajorEvent, "Caught exception in " + self.getName() + " thread" +
            ":\n" + exceptionDump(sys.exc_info()))
      self._exceptionInfo = sys.exc_info()
    except:
      debug(MajorEvent, "Uncaught exception in " + self.getName() + " thread")
      self._exceptionInfo = sys.exc_info()

    if not self._exceptionInfo is None:
      debug(Event, "Exception caught:\n" + exceptionDump(self._exceptionInfo))
    
    debug(Event, self.getName() + " thread is terminating")
    if not self.getName() == 'COOJA' and self._exceptionInfo is None:
      debug(MinorEvent, self.getName() + " terminated quietly")
      self.threadManager()._listLock.acquire()
      del self.threadManager()._startedThreads[self.getName()]
      del self.threadManager()._activeThreads[self.getName()]
      self.threadManager()._listLock.release()
    else:
      debug(MinorEvent, self.getName() + " terminated normally (slow)")
      self.threadManager()._registerResult(self, self._exceptionInfo)
      
    debug(Debug, "End of " + self.getName() + " thread")

  def doRun(self):
    threading.Thread.run(self)
    

class ThreadManager(object):
  """Singleton class for managing active python threads."""
  def __init__(self):
    self._activeThreads = {}
    self._startedThreads = {}
    self._threadTerminations = []
    self._pidErrors = 0
    # Protects the three lists above
    self._listLock = threading.RLock()
    self._threadTerminationQueue = Queue.Queue()
    debug(Debug, self.summary())

  def _registerThread(self, thread):
    "Called by thread classes when a thread is created."
    assert(isinstance(thread, ManagedThread))
    self._listLock.acquire()
    try:
      assert(not thread.getName() in self._activeThreads)
      debug(Debug, "Registering " + thread.getName() + " thread")
      self._activeThreads[thread.getName()] = thread
      debug(Debug, self.summary())
    finally:
      self._listLock.release()

  def _registerThreadStart(self, thread):
    "Called by the thread classes' run method."
    assert(isinstance(thread, ManagedThread))
    self._listLock.acquire()
    try:
      assert(thread.getName() in self._activeThreads)
      assert(not thread.getName() in self._startedThreads)
      debug(Debug, "Registering " + thread.getName() + " thread start")
      self._startedThreads[thread.getName()] = thread
      debug(Debug, self.summary())
    finally:
      self._listLock.release()

  def _registerResult(self, thread, exceptionInfo):
    self._threadTerminationQueue.put((thread.getName(), exceptionInfo))

  def runningThreads(self):
    self._listLock.acquire()
    try:
      ret = []
      for thread in self._startedThreads.values():
        if not thread.getName() in [
          te[0].getName() for te in self._threadTerminations]:
          ret.append(thread)
      return ret
    finally:
      self._listLock.release()

  def unstartedThreads(self):
    self._listLock.acquire()
    try:
      ret = []
      for thread in self._activeThreads.values():
        if not thread.getName() in self._startedThreads:
          ret.append(thread)
      return ret
    finally:
      self._listLock.release()

  def numThreadsLeft(self):
    return len(self.unfinishedThreads())

  def unfinishedThreads(self):
    "Return created threads that have not terminated."
    self._listLock.acquire()
    try:
      return self._activeThreads.values()
    finally:
      self._listLock.release()
    
  def waitAll(self, timeout = 3600, mourningTime = 5):
    "Wait for all threads to terminate."
    debug(MajorEvent, "Waiting for all test threads to terminate")
    debug(MinorEvent, self.summary())
    limit = time.time() + timeout
    while self.numThreadsLeft() > 0:
      try:
        nextTimeout = limit - time.time()
        if nextTimeout <= 0:
          raise chakana.error.Timeout(self.unfinishedThreads()[0], timeout)
        else:
          self.waitOne(nextTimeout)
      except chakana.error.Timeout, err:
        debug(Error, "Timeout waiting for " + err.child.getName() +
              " thread, killing subprocesses.")
        self.killAll()
        while self.numThreadsLeft() > 0:
          try:
            self.waitOne(mourningTime)
          except chakana.error.Timeout:
            debug(Error, "Timeout while mourning threads, aborting")
            signal.signal(signal.SIGABRT, signal.SIG_DFL)
            os.abort()
        raise
    debug(MinorEvent, "Done waiting for " + str(len(self._startedThreads)) +
          " threads")
    debug(Debug, self.summary())
    for (thread, excInfo) in self._threadTerminations:
      if not excInfo is None:
        debug(MinorEvent, "Rethrowing exception from " + thread.getName() +
              " thread")
        raise chakana.error.ChildException(excInfo[1], excInfo)

  def waitOne(self, timeout):
    "Wait for any one thread."
    debug(Event, "Waiting for some thread to finish, timeout = " +
          str(timeout))
    debug(MinorEvent, self.summary())
    debug(Debug, "Threads left: " +
          str(map(ManagedThread.getName, self.unfinishedThreads())))
    assert(self.numThreadsLeft() > 0)
    try:
      (threadName, exception) = self._threadTerminationQueue.get(
        timeout = timeout)
    except Queue.Empty:
      raise chakana.error.Timeout(self.unfinishedThreads()[0], timeout)
    debug(MinorEvent, "Received termination signal from thread " + threadName)
    self._listLock.acquire()
    try:
      assert(threadName in self._activeThreads)
      terminatedThread = self._activeThreads[threadName]
      assert(terminatedThread.getName() == threadName)
      if exception is None:
        debug(Debug, "Thread " + threadName + " completed successfully")
      else:
        debug(Debug, "Thread " + threadName + " raised an exception")
        self._threadTerminations.append((terminatedThread, exception))
        self.killAll()
      terminatedThread.join()
      debug(Debug, "Deleting " + threadName + " thread from active threads")
      del self._activeThreads[threadName]
      del self._startedThreads[threadName]
    finally:
      self._listLock.release()

  def killAll(self, reason = "Error detected, killing remaining processes"):
    debug(MajorEvent, reason)
    chakana.linux.Process(os.getpid()).killAllBelow()    

  def registerPIDerror(self):
    self._pidErrors = self._pidErrors + 1
    if self._pidErrors > 100000:
      self.killAll(reason = "PID allocation errors > 100000")

  def nrPIDerrors(self):
    return self._pidErrors

  def summary(self):
    return "ThreadManager: " + str(self.numThreadsLeft()) + \
           " threads left, " + \
           str(len(self.unstartedThreads())) + " unstarted threads,\n" + \
           str(len(self.runningThreads())) + " running threads"

class TimeoutHelperThread(threading.Thread):
  def __init__(self, func, funcArgs, funcKwArgs, ** kwArgs):
    threading.Thread.__init__(self, ** kwArgs)
    self._func = func
    self._args = funcArgs
    self._kwArgs = funcKwArgs
    self.result = None
    self.error = None

  def run(self):
    try:
      self.result = self._func(* self._args, ** self._kwArgs)
    except Exception, err:
      debug(Debug, "Caught exception in timeout function: " + str(err) +
            ":\n" + exceptionDump(sys.exc_info()))
      self.error = err
      self.exc_info = sys.exc_info()

class RunWithTimeout:
  """Run func in a separate thread.  If timeout seconds elapse, give
  up, raise Timeout(thread, timeout)"""

  def __init__(self, timeout, func, name = None):
    self._timeout = timeout
    self._func = func
    if name is None:
      self._name = "thread running " + str(func)
    else:
      self._name = name

  def __call__(self, * args, ** kwArgs):
    thread = TimeoutHelperThread(self._func, args, kwArgs, name = self._name)
    thread.start()
    thread.join(self._timeout)
    if thread.isAlive():
      raise chakana.error.Timeout(thread, self._timeout)
    if thread.error is None:
      return thread.result
    raise chakana.error.ChildException(thread.error, thread.exc_info)

def waitForAll(threads, timeout):
  startTime = time.time()
  for t in threads:
    if timeout is None:
      t.join()
    else:
      t.join(startTime + timeout - time.time())
    if t.isAlive():
      raise chakana.error.Timeout(t, timeout)

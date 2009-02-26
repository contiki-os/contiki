#  
#  Copyright (C) 2003-2007 Swedish Institute of Computer Science.
#  
# Please refer to the file named LICENSE in the same directory as this
# file for licensing information.
#  

# $Id: debug.py,v 1.1 2009/02/26 13:47:38 fros4943 Exp $

import inspect
import os
import sys
import threading
import StringIO

DebugMin = 0
Always = 0
Fatal = 1
Critical = 2
Error = 3
Warning = 4
Check = 5
Information = 6
MajorEvent = 7
Event = 8
MinorEvent = 9
Debug = 10
Debug2 = 11
Debug3 = 12
Debug4 = 13
Disabled = 14
DebugMax = 15

DefaultDebugLevel = Information

def levelName(level):
  for (varName, value) in globals().items():
    if not varName in ("DebugMin", "DebugMax"):
      if value == level:
        return varName
  return "Unknown"
        
class DebugSingleton:
  def __init__(self, output = sys.stderr, defaultLevel = DefaultDebugLevel,
               logLevel = Debug4):
    self.maxBufferLen = 1024 * 1024
    self.__level = defaultLevel
    self.__output = output
    self.__logBuffer = []
    self.__logBufferSize = 0
    self.__logLevel = Debug4
    self.__logFile = None
    self.__logFileName = None
    self.__lock = threading.RLock()
    self._transformers = []

  def level(self):
    return self.__level

  def setLog(self, fileName):
    assert(not self.__logBuffer is None)
    if not fileName is None:
      self.__logFileName = fileName
      self.__logFile = open(fileName, "w")
      self.__logFile.write("".join(self.__logBuffer))
      self.write(MajorEvent, "Saving debug log in " + fileName + "\n")
    self.__logBuffer = None

  def logFileName(self):
    return self.__logFileName
    
  def logLevel(self):
    return self.__logLevel

  def cleanLog(self):
    self.setLog(None)
  
  def setLogLevel(self, level):
    self.__logLevel = level
    self.write(MinorEvent, "Changed debug log verbosity to " +
               levelName(self.__logLevel) + " (" +
               str(self.__logLevel) + ")\n")

  def setLevel(self, level):
    self.__level = level
    if self.__level < Always:
      self.__level = Always
    if self.__level >= Disabled:
      self.__level = Disabled - 1
    self.write(MinorEvent, "Changed debug verbosity to " +
               levelName(self.__level) + " (" + str(self.__level) + ")\n")
  
  def increaseLevel(self, amount = 1):
    self.setLevel(self.level() + amount)

  def decreaseLevel(self, amount = 1):
    self.increaseLevel(- amount)

  def write(self, level, * messages):
    """Write messages if current verbosity level >= level.  Messages should
    either be callable functors taking a writer function parameter, or they
    should be strings or convertible to strings."""
    
    writers = []
    if self.level() >= level:
      writers.append(self.__output.write)
    if self.logLevel() >= level:
      writers.append(self.logWrite)
    if writers != []:
      self.__lock.acquire()
      try:
        for writer in writers:
          writer(self.transform(level = level,
                                message = self.format(messages)))
      finally:
        self.__lock.release()

  def format(self, messages):
    buf = StringIO.StringIO()
    for msg in messages:
      if callable(msg):
        msg(buf.write)
      else:
        buf.write(msg)
    return buf.getvalue()

  def transform(self, level, message):
    for transform in self._transformers:
      message = transform(level = level, message = message)
    return message

  def writer(self, level):
    return lambda what: self.write(level, what)

  def logWrite(self, what):
    if self.__logFile is None:
      if not self.__logBuffer is None:
        self.__logBuffer.append(what)
        self.__logBufferSize += len(what)
        if self.__logBufferSize > self.maxBufferLen:
          self.__logBuffer = None
          self.write(Warning, "Cleaned log buffer since it overflowed " +
                     str(self.maxBufferLen) + " bytes\n")
    else:
      self.__logFile.write(what)
      self.__logFile.flush()

  def pushTransformer(self, transformer):
    assert(callable(transformer))
    if not transformer in self._transformers:
      self._transformers.append(transformer)

debugStream = DebugSingleton()
      
def debug(level, message):
  debugStream.write(level, message, "\n")


class DebugDumper:
  def __init__(self):
    pass

  def __call__(self, writer):
    raise NotImplementedError()
  
  def __add__(self, other):
    return DebugCombiner(self, other)

  def __radd__(self, other):
    return DebugCombiner(other, self)


class DebugCombiner:
  def __init__(self, first, second):
    self.__first = first
    self.__second = second

  def __call__(self, writer):
    for elem in [self.__first, self.__second]:
      if callable(elem):
        elem(writer)
      else:
        writer(str(elem))

class DebugFile:
  def __init__(self):
    self.__str = ""

  def write(self, msg):
    self.__str += msg

  def __str__(self):
    return self.__str

DebugStderr = DebugFile
  
def exceptionDump((type, value, traceback)):
  """Print an exception stack trace to DebugStderr().  Pass
  sys.exc_info() as parameter."""
  if type is None:
    return ""
  oldStderr = sys.stderr
  debugStderr = DebugStderr()
  sys.stderr = debugStderr
  sys.__excepthook__(type, value, traceback)
  sys.stderr = oldStderr
  return str(debugStderr)

def exceptionMessage((type, value, traceback)):
  import urllib2
  if isinstance(value, urllib2.HTTPError):
    return str(value) + ": " + value.filename
  else:
    return str(value)

class CallTracer:
  def __init__(self, level, prefix = lambda f, e, a: "", traceSystem = 0,
               traceDebug = 0):
    self._level = level
    self._prefix = prefix
    self._traceSystem = traceSystem
    self._traceDebug = traceDebug

  def indent(self, frame):
    stack = inspect.stack()
    depth = len(stack) - 4
    assert(depth >= 0)
    while len(stack) > 0:
      del stack[0]
    del stack
    return "  " * depth

  def prefix(self, frame, event, arg):
    return self._prefix(frame, event, arg) + self.indent(frame)

  def inDebugCode(self, frame):
    if frame is None:
      return 0
    if os.path.basename(frame.f_code.co_filename) == "debug.py":
      ret = 1
    else:
      ret = self.inDebugCode(frame.f_back)
    del frame
    return ret
  
  def __call__(self, frame, event, arg):
    if frame.f_back is None:
      callerDir = ""
    else:
      callerDir = os.path.dirname(frame.f_back.f_code.co_filename)
    sourceFile = os.path.basename(frame.f_code.co_filename)
    funcName = sourceFile + ":" + frame.f_code.co_name + "()"
    if (not self._traceDebug) and self.inDebugCode(frame):
      return None
    if self._traceSystem or \
       not (callerDir.startswith(sys.prefix + "/lib/python")):
      if event == "call":
        debug(self._level, self.prefix(
          frame = frame, event = event, arg = arg) + "-> " + funcName)
      elif event == "return":
        debug(self._level, self.prefix(
          frame = frame, event = event, arg = arg) + "<- " + funcName)
      elif event == "exception":
        debug(self._level, self.prefix(
          frame = frame, event = event, arg = arg) + "X  " + funcName)
    return self

def pidPrefix(message = "", ** kwArgs):
  return str(os.getpid()) + ": " + message

def threadPrefix(message = "", ** kwArgs):
  return threading.currentThread().getName() + ": " + message

def objectAsString(obj):
     names = [(name, value) for name,value in vars(obj).iteritems()]
     names.insert(0,("Instace of", obj.__class__.__name__))
     max_length = len(max([name for (name, value) in names]))
     return "\n".join([name.rjust(max_length) + ": " + str(value) for (name,value) in names])


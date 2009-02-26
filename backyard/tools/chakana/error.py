#  
#  Copyright (C) 2003-2007 Swedish Institute of Computer Science.
#  
# Please refer to the file named LICENSE in the same directory as this
# file for licensing information.
#  

# $Id: error.py,v 1.1 2009/02/26 13:47:38 fros4943 Exp $

import errno
import os

import chakana.debug

class Error(Exception):
  def __init__(self, message):
    Exception.__init__(self, message)

class CommandFailed(Error):
  """Exception thrown when a system command fails.  The command string,
  environment, working directory, output, and exit status are recorded.
  The exit status is typically the value returned by os.system, but may
  also be an exception, for example an error.Timeout object."""

  def __init__(self, command, status, output):
    Error.__init__(self, "Command failed with exit code " + str(status) +
                   ": " + command + "\n" + "Output:\n" + output + "\ncwd: " +
                   os.getcwd() + "\n")
    self._command = command
    self._status = status
    self._output = output
    self._environ = os.environ.copy()
    self._cwd = os.getcwd()

  def command(self):
    return self._command

  def status(self):
    return self._status

  def output(self):
    return self._output

  def environ(self):
    return self._environ

  def cwd(self):
    return self._cwd

class SanityCheckFailed(Error):
  def __init__(self, message):
    Error.__init__(self, "Sanity check failed: " + message)

class NoSuchProcess(OSError):
  def __init__(self):
    OSError.__init__(self, errno.ESRCH, "No such process")

class Timeout(Error):
  def __init__(self, child, timeout):
    Error.__init__(self, "Timeout (" + str(timeout) + " s) executing " +
                   child.getName())
    self.child = child
    self.timeout = timeout

class Discarded(Error):
  def __init__(self):
    Error.__init__(self, "Monitor was discarded")

class ChildException(Error):
  def __init__(self, error, exc_info):
    Error.__init__(self, "Exception in child thread: " + str(error) +
                   ":\n" + chakana.debug.exceptionDump(exc_info))
    self.error = error
    self.exc_info = exc_info

class CoojaError(Error):
  def __init__(self, response):
    Error.__init__(self, "COOJA error: " +
                   response.documentElement.childNodes[0].nodeValue)

class CoojaExit(Error):
  def __init__(self):
    Error.__init__(self, "COOJA has exited")

def errnoError(errNum, fileName = None):
  if isinstance(errNum, str):
    return errnoError(getattr(errno, errNum), fileName)
  return OSError(errNum, os.strerror(errNum), fileName)


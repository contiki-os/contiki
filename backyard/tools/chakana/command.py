#  
#  Copyright 2005-2007 Swedish Institute of Computer Science.
#  
#  Please refer to the file named LICENSE in the same directory as this file
#  for licensing information.
#  
#  Written and maintained by Lars Albertsson <lalle@sics.se>.
#  

# $Id: command.py,v 1.1 2009/02/26 13:47:38 fros4943 Exp $

"""Utility routines for running external commands."""

import errno
import os
import popen2
import re
import signal
import threading

import chakana.error
import chakana.linux
import chakana.threads

def quote(argument, useTick = 1):
  """Quote an argument, or a list of arguments, in order to pass it
  through a shell."""
  if type(argument) in (type([]), type(())):
    return " ".join(map(quote, argument))
  if useTick and not "'" in argument:
    return "'" + argument + "'"
  return '"' + argument.replace('\\', '\\\\').replace('"', '\\"').replace(
    '`', '\\`').replace('$', '\\$') + '"'

def softQuote(argument):
  """Quote an argument, or a list of arguments, in order to pass it through
  a shell if it is necessary.  Always quotes with quotation mark, never
  with tick."""
  if type(argument) in (type([]), type(())):
    return " ".join(map(softQuote, argument))  
  if re.match(r'^[\w,./=-]*$', argument):
    return argument
  else:
    return quote(argument, useTick = 0)
  
def deQuote(argument):
  """Remove quoting in the same manner as the shell (bash) does."""
  if argument == "":
    return ""
  if argument[0] == "'":
    nextQuote = argument.find("'", 1)
    if nextQuote == -1:
      raise ValueError("Unmatched quote \"'\"")
    return argument[1 : nextQuote] + deQuote(argument[nextQuote + 1 :])
    if len(argument) > 1 and argument[-1] == "'":
      return argument[1:-1]
  elif argument[0] == '"':
    ret = []
    index = 1
    try:
      while True:
        if argument[index] == '"':
          return "".join(ret) + deQuote(argument[index + 1 :])
        if argument[index] == '\\':
          index += 1
          if argument[index] != '\n':
            ret.append(argument[index])
        else:
          ret.append(argument[index])
        index += 1
    except IndexError:
      raise ValueError("Unmatched quote '\"'")
  elif argument[0] == '\\':
    if len(argument) == 1:
      return argument[0]
    if argument[1] != '\n':
      return argument[1] + deQuote(argument[2:])
    else:
      return argument[2:]
  return argument[0] + deQuote(argument[1:])

  
def runString(argv, environmentVars = ("PYTHONPATH", )):
  """Return a quoted string that can be pasted into a shell in order
  to rerun a command."""
  
  envPrefix = ""
  for envVar in environmentVars:
    envPrefix += envVar + "=" + os.environ.get(envVar, "") + " "
  return "( cd " + os.getcwd() + " && " + envPrefix + quote(argv) + " )"

class Reader:
  def __call__(self, fileObj):
    self._data = []
    while 1:
      newData = fileObj.read()
      if newData == "":
        return
      self._data.append(newData)

  def result(self):
    return "".join(self._data)

class Writer:
  def __init__(self, fileObj, data):
    self._fileObj = fileObj
    self._data = data
    if self._data == "":
      self._fileObj.close()
      self._threads = []
    else:
      self._threads = [threading.Thread(target = self.write)]
      self._threads[0].start()

  def write(self):
    self._fileObj.write(self._data)
    self._fileObj.close()

  def activeThreads(self):
    return self._threads

class Runner:
  def __init__(self, child, timeout):
    self._child = child
    self._timeout = timeout
    self._threads = []
    self._output = []
    if self._timeout is None:
      self.read()
      self.wait()
    else:
      self._threads.append(threading.Thread(target = self.read))
      self._threads[-1].start()
      self._threads.append(threading.Thread(target = self.wait))
      self._threads[-1].start()

  def activeThreads(self):
    return self._threads

  def read(self):
    while 1:
      newData = self._child.fromchild.read()
      if newData == "":
        return
      self._output.append(newData)

  def wait(self):
    self._status = self._child.wait()
      
  def output(self):
    return "".join(self._output)

  def status(self):
    return self._status
    
def output(command, inputData = "", timeout = None):
  """Run command in a separate subprocess.  Send inputData to stdin.
  Kill subprocess and raise error.Timeout after timeout seconds,
  unless timeout is None."""

  # Due to python bug 1183780 (Popen4.wait not thread-safe), avoid
  # threads if we don't need them.

  child = popen2.Popen4(command)
  writer = Writer(child.tochild, inputData)
  runner = Runner(child, timeout)
  
  try:
    chakana.threads.waitForAll(writer.activeThreads() + runner.activeThreads(),
                               timeout)
  except chakana.error.Timeout, timeoutErr:
    try:
      childProc = chakana.linux.Process(child.pid)
      childProc.killAllBelow()
      childProc.reallyKill()
    except OSError, err:
      if not err.errno in (errno.ESRCH, errno.ECHILD):
        raise
    raise chakana.error.CommandFailed(command, timeoutErr, runner.output())
    
  if runner.status() != 0:
    raise chakana.error.CommandFailed(
      command, runner.status(), runner.output())
  if re.search(r"\blibefence", os.environ.get("LD_PRELOAD", "")):
    lines = runner.output().splitlines()
    if (len(lines) >= 2) and (lines[0] == "") and \
           lines[1].strip().startswith("Electric Fence "):
      return "\n".join(lines[2:] + [""])
  return runner.output()


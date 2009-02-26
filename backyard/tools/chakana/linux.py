#  
#  Copyright (C) 2004-2007 Swedish Institute of Computer Science.
#  
# Please refer to the file named LICENSE in the same directory as this
# file for licensing information.
#  


# $Id: linux.py,v 1.1 2009/02/26 13:47:38 fros4943 Exp $

import errno
import glob
import os
import re
import signal
import string
import time

import chakana.command
import chakana.error
import chakana.utils

from debug import *

def numProcessors(physical = 0, cpuInfo = None):
  return Host().CpuInfo(cpuInfo).count(physical = physical)

class CpuInfo:
  "Parser for /proc/cpuinfo."
  def __init__(self, cpuInfo = None):
    if cpuInfo is None:
      cpuInfo = chakana.utils.readFile("/proc/cpuinfo")
    self.__values = [map(string.strip, l.split(":"))
                     for l in cpuInfo.splitlines()]

  def values(self):
    return self.__values

  def find(self, name):
    debug(Debug3, "Looking for " + name + " in cpuinfo:")
    ret = [val[1] for val in self.values() if val[0] == name]
    debug(Debug3, repr(ret))
    return ret

class IfconfigInfo:
  "Parser for ifconfig output."
  def __init__(self, ifconfigOutput = None):
    if ifconfigOutput is None:
      self._ifconfigOutput = chakana.command.output("/sbin/ifconfig")
    else:
      self._ifconfigOutput = ifconfigOutput

  def interfaces(self):
    return map(InterfaceInfo,
               re.compile("^\\w+", re.M).findall(self._ifconfigOutput))
  
class InterfaceInfo:
  "Parser for ifconfig output for a single interface."
  def __init__(self, interface = None, ifconfigOutput = None):
    if interface is None:
      self._interface = chakana.command.output("/sbin/ifconfig").split()[0]
    else:
      self._interface = interface
    if ifconfigOutput is None:
      self._ifconfigOutput = chakana.command.output("/sbin/ifconfig " +
                                            self._interface)
    else:
      self._ifconfigOutput = ifconfigOutput

  def name(self):
    return self._ifconfigOutput.split()[0]
      
  def address(self):
    match = re.search("inet addr:((\d{1,3}\.){3}\d{1,3})",
                      self._ifconfigOutput)
    if match:
      return match.group(1)
    else:
      return None

  __str__ = name
  
class MemInfo(dict):
  "Parser for /proc/meminfo"
  def __init__(self, memInfo = None):
    if memInfo is None:
      memInfo = chakana.utils.readFile("/proc/meminfo")
    for line in memInfo.splitlines():
      if len(line.split(":")) == 2:
        key, value = line.split(":")
        if len(value.strip().split()) <= 2:
          if len(value.strip().split()) == 2:
            unit = {"KB" : 1024, "MB" : 1024 * 1024,
                    "GB" : 1024 * 1024 * 1024 }[value.split()[-1].upper()]
          else:
            unit = 1
          self[key] = long(value.strip().split()[0]) * unit
  
class UnameInfo:
  "Parser for output from uname -a"
  def __init__(self, unameInfo = None):
    if unameInfo is None:
      unameInfo = chakana.command.output("uname -a")
    self._unameInfo = unameInfo.split()

  def __getitem__(self, i):
    return self._unameInfo[i]

  def __len__(self):
    return len(self._unameInfo)

  def __getattr__(self, attr):
    ret = self[self.indexMap()[attr]]
    if (attr == "architecture") and \
       (re.match(r"(.*intel|i[0-9]86)", ret, re.I)):
      # Work around bogus uname output from e.g. Gentoo
      return "i386"
    else:
      return ret

  def smp(self):
    return "SMP" in self._unameInfo
  
  def indexMap(self):
    return {
      "os" : 0,
      "host" : 1,
      "version" : 2,
      "architecture" : -2,
      }

  def __str__(self):
    return repr(dict([(attr, getattr(self, attr))
                      for attr in self.indexMap()]))

  def __repr__(self):
    return repr(self._unameInfo)
  
class X86:
  class CpuInfo(CpuInfo):
    def count(self, physical = 0):
      logical = len(self.find("processor"))
      if physical:
        siblings = self.find("siblings")
        if len(siblings) > 0:
          return logical / int(siblings[0])
      return logical

    def type(self, num = 0):
      return re.match(r"Intel(\(R\))? (.*) processor [\d]+MHz",
                      self.find("model name")[num]).group(2)

class X86_64(X86):
  pass
    
class Sparc:
  class CpuInfo(CpuInfo):
    def count(self):
      return int(self.find("ncpus active")[0])

    def type(self, num = 0):
      return " ".join(self.find("cpu")[num].split()[1:3])

  class UnameInfo(UnameInfo):
    pass

def freeSpace(path):
  "Return available disk space on device corresponding to path."
  # If the device name is long, df may split the relevant line, so
  # count lines and words from end.
  return long(
    chakana.command.output("df -k " + path).splitlines()[-1].split()[-3]) * 1024

def macAddresses():
  "Return list of mac addresses for all network interfaces."
  return re.findall('..:..:..:..:..:..', os.popen(
    '/sbin/ifconfig -a | grep HWaddr', 'r').read().lower())

class Process:
  """Representation of a running process."""

  def __init__(self, pid):
    self._pid = pid

  def pid(self):
    return self._pid

  def attribute(self, name):
    try:
      lines = chakana.utils.readFile("/proc/" + str(self._pid) +
                             "/status").splitlines()
      for line in lines:
        (key, value) = line.split(":", 1)
        if key.strip().lower() == name.lower():
          return value.strip()
    except EnvironmentError, err:
      if err.errno == errno.ENOENT:
        raise chakana.error.errnoError(errno.ESRCH)
      raise

  def name(self):
    return self.attribute("name")

  def parent(self):
    return Process(int(self.attribute("ppid")))

  def children(self):
    ret = []
    for proc in self.all():
      try:
        if proc.parent().pid() == self.pid():
          ret.append(proc)
      except EnvironmentError, err:
        if err.errno != errno.ESRCH:
          raise
    return ret

  def reallyKill(self, graceTime = 1, sigKillTime = 3, debugLevel = Event):
    """Really kill process after waiting for graceTime.  Send SIGKILL after
    sigKillTime."""
    try:
      debug(debugLevel, "Killing process " + str(self.pid()) +
            " (" + str(self.name()) + ")")
      os.kill(self.pid(), 0)
      time.sleep(graceTime)
      debug(debugLevel + 1, "kill -TERM " + str(self.pid()))
      os.kill(self.pid(), signal.SIGTERM)
      for tick in range(sigKillTime):
        os.kill(self.pid(), 0)
      debug(debugLevel + 1, "kill -KILL " + str(self.pid()))
      os.kill(self.pid(), signal.SIGKILL)
      time.sleep(graceTime)
      if self.attribute("state")[0] != "Z":
        debug(debugLevel, "Process refused to die: " + str(self.pid()) +
              " (" + self.name() + ")")
    except EnvironmentError, err:
      if err.errno != errno.ESRCH:
        raise
      debug(debugLevel + 2, "Pid " + str(self.pid()) + " no longer exists")
  
  def killAllBelow(self, debugLevel = Event, ** kwArgs):
    """Kill all processes below (but not including) a process.  Start from
    bottom of tree."""
    debug(debugLevel, "Killing all processes below " + str(self.pid()) +
          " (" + self.name() + ")")
    children = self.children()
    debug(debugLevel + 2, "Pid " + str(self.pid()) + " has " +
          str(len(children)) + " children")
    for child in children:
      child.killAllBelow(debugLevel = debugLevel, ** kwArgs)
      child.reallyKill(debugLevel = debugLevel, ** kwArgs)
    
  def all(cls):
    ret = []
    for statusFile in glob.glob("/proc/*/status"):
      try:
        ret.append(cls(int(os.path.basename(os.path.dirname(statusFile)))))
      except ValueError:
        continue
    return ret
  all = classmethod(all)

def Host():
  architecture = chakana.command.output("uname -m").strip()
  if re.match("i[0-9]86", architecture):
    return X86
  elif architecture == "x86_64":
    return X86_64

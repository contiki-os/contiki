import errno
import os
import re
import time

import chakana.command
import chakana.error
import chakana.shepherd
import chakana.threads
import chakana.utils

from chakana.debug import *

class Harness:
  def __init__(self, chakanaRoot, coojaTimeout, doCompile="True", withGUI="False"):
    self._chakanaRoot = os.path.abspath(chakanaRoot)
    self._coojaTimeout = coojaTimeout
    self._threadManager = chakana.threads.ThreadManager()
    self._doCompile = doCompile
    self._withGUI = withGUI
    debug(MajorEvent, "Chakana harness created")

  def start(self):
    port = self.startCooja()
    self._shepherd = chakana.shepherd.Shepherd(self._threadManager, port)
    
  def startCooja(self):
    # COMPILE COOJA AND PLUGIN
    if self._doCompile == "True":
      for target in ("cooja", "plugin"):
        buildCommand = 'cd ' + chakana.command.quote(self._chakanaRoot) + ' && ant compile_' + target
        debug(MajorEvent, "Building " + target)
        debug(Event, buildCommand)
        output = chakana.command.output(buildCommand)
        debug(MinorEvent, output)

    coojaOutputFile = os.path.join(self._chakanaRoot, "build/cooja.out")
    # START COOJA

    if os.path.isfile(coojaOutputFile):
      os.remove(coojaOutputFile)
    coojaThread = CoojaThread(self._threadManager, self._chakanaRoot,
                              coojaOutputFile, self._coojaTimeout, withGUI=self._withGUI)
    coojaThread.start()
    return coojaThread.port()

  def shepherd(self):
    return self._shepherd

  def quit(self):
    self._shepherd.quit()

  def killAllProcesses(self):
    self._threadManager.killAll()

  def waitForAllThreads(self, timeout):
    self._threadManager.waitAll(timeout)

class CoojaThread(chakana.threads.ManagedThread):
  def __init__(self, threadManager, chakanaRoot, outputFile,
               timeout = 3600, name = "COOJA", withGUI="False", ** kwArgs):
    chakana.threads.ManagedThread.__init__(
      self, threadManager, name = name, **kwArgs)
    self._chakanaRoot = chakanaRoot
    self._outputFile = outputFile
    self._timeout = timeout
    self._port = None
    self._withGUI = withGUI

  def doRun(self):
    debug(MajorEvent, "Starting COOJA")
    buildDir = os.path.dirname(self._outputFile)
    chakana.utils.makeDirsSafe(buildDir)
    contikiRoot = os.path.join(self._chakanaRoot, '../..')
    contikiRoot = contikiRoot.replace('/cygdrive/c', 'c:')
    if self._withGUI == "True":
      coojaCommand = '( cd ' + chakana.command.quote(buildDir) + ' && java -jar ' + \
                     chakana.command.quote(os.path.join(contikiRoot, 'tools/cooja/dist/cooja.jar')) + ' ' + \
                     '-external_tools_config=../cooja.chakana.properties ' + \
                     '-contiki=' + chakana.command.quote(contikiRoot) + ' < /dev/null > ' + \
                     os.path.basename(self._outputFile) + ' 2>&1 )'
    else:
      coojaCommand = '( cd ' + chakana.command.quote(buildDir) + ' && java -jar ' + \
                     chakana.command.quote(os.path.join(contikiRoot, 'tools/cooja/dist/cooja.jar')) + ' ' + \
                     '-nogui ' + \
                     '-external_tools_config=../cooja.chakana.properties ' + \
                     '-contiki=' + chakana.command.quote(contikiRoot) + ' < /dev/null > ' + \
                     os.path.basename(self._outputFile) + ' 2>&1 )'

    debug(Event, coojaCommand)
    os.system(coojaCommand)
    debug(MajorEvent, "COOJA has finished")

  def port(self):
    if self._port is None:
      laps = 0
      debug(Event, "Waiting for COOJA to open server socket")
      debug(Debug, "Reading: " + self._outputFile)
      while 1:
        if self._timeout > 0 and laps > self._timeout:
          raise chakana.error.Timeout(self, self._timeout)
        logContents = ""
        try:
          logContents = chakana.utils.readFile(self._outputFile)
        except IOError, err:
          if err.errno != errno.ENOENT:
            raise
        match = re.search(r"Chakana server listening on port (\d+).",
                          logContents)
        debug(Debug, "Log contents: " + logContents)
        if match:
          self._port = int(match.group(1))
          debug(Event, "COOJA is now listening on port " + str(self._port))
          break
        else:
          debug(Debug, "Waiting for COOJA to start")
          time.sleep(1)
          laps += 1

        match = re.search(r"Unable to access jarfile",
                          logContents)
        if match:
          raise RuntimeError("Could not locate COOJA JAR: " + logContents)

    return self._port


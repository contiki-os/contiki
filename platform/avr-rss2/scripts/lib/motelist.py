#!/usr/bin/env python

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

import os, sys, threading, time, serial
from get_ports_linux import comports  # @UnusedImport

# Unified way of accessing motes
class Mote(object):
    def __init__(self, mote, manualyAdded = False):
        if mote == None:
            self.__port = None
            self.__name = "No motes found!"
            self.__reference = "Make sure mote(s) are connected and drivers are installed."
            self.__host = None
            self.__userdata = None
            self.__manualyAdded = manualyAdded

        elif len(mote) == 3:
            self.__port = mote[0]
            self.__name = mote[1]
            self.__reference = mote[2]
            self.__host = "Local"
            self.__userdata = None
            self.__manualyAdded = manualyAdded
            
        elif len(mote) == 4:
            self.__port = mote[0]
            self.__name = mote[1]
            self.__reference = mote[2]
            self.__host = mote[3]
            self.__userdata = None
            self.__manualyAdded = manualyAdded
        else:
            print ("Failed to initialize mote from " + str(mote))

    def getNiceName(self):
        if self.__host is None:
            if self.__name.find(self.__port) != -1:
                return "{}".format(self.__name)
            else:
                return "{}({})".format(self.__name, self.__port)
        else:
            if self.__name.find(self.__port) != -1:
                return "{} @ {}".format(self.__name, self.__host)
            else:
                return "{}({}) @ {}".format(self.__name, self.__port, self.__host)

    def getFullName(self):
        return "{} [{}]".format(self.getNiceName(), self.__reference)

    def getCSVData(self):
        if self.__host is None:
            return "{},{},{}".format(self.__reference, self.__port, self.__name)
        else:
            return "{},{},{},{}".format(self.__reference, self.__port, self.__name, self.__host)

    def isUserMote(self):
        return self.__manualyAdded

    def setUserData(self, userData):
        self.__userdata = userData

    def getUserData(self):
        return self.__userdata

    def getPort(self):
        return self.__port

    def getHost(self):
        if self.__host is None:
            return ''
        else:
            return self.__host

    def isLocal(self):
        return self.__host is None \
            or len(self.__host) == 0 \
            or self.__host == "Local"
        
    def getName(self):
        return self.__name

    def getReference(self):
        return self.__reference

    def cmp(self, other):
        host1 = self.getHost()
        host2 = other.getHost()
        if host1 == "Local": host1 = ""
        if host2 == "Local": host2 = ""
        if host1.find("://") != -1: host1 = host1[host1.find("://") + 3:]
        if host2.find("://") != -1: host2 = host2[host2.find("://") + 3:]
        if host1 == host2:
            if self.getPort() == other.getPort(): return 0
            if self.getPort() < other.getPort(): return -1
            return 1
        if host1 < host2: return -1
        return 1
    def __lt__(self, other):
         return self.cmp(other) < 0
    def __gt__(self, other):
        return self.cmp(other) > 0
    def __le__(self, other):
        return self.cmp(other) <= 0
    def __ge__(self, other):
        return self.cmp(other) >= 0
    def __eq__(self, other):
        # Makes equal work on different mote classes
        if type(other) is not type(self): return False
        return self.cmp(other) == 0
    def __ne__(self, other):
        if type(other) is not type(self): return True
        return self.cmp(other) != 0
        
    
class Motelist(object):
    motes = list()
    lock = threading.Lock()
    updateCallbacks = list()
    infinite = False

    @staticmethod
    def initialize(updateCallbacks, startPeriodicUpdate = False, onlyLocalMotes = False):
        if updateCallbacks is None:
            return

        if type(updateCallbacks) is list:
            Motelist.updateCallbacks = updateCallbacks
        else:
            Motelist.updateCallbacks.append(updateCallbacks)

        if startPeriodicUpdate:
            Motelist.startPeriodicUpdate()

    @staticmethod
    def addMote(port, name, reference):
        Motelist.lock.acquire()

        portFound = not Motelist.portExists(port)

        for mote in Motelist.motes:
            if mote.getPort().lower() == port.lower():
                portFound = True
                break

        if not portFound:
            Motelist.motes.append(Mote([port, name, reference], True))

        Motelist.lock.release()
        if not portFound:
            Motelist.__activateCallbacks(True)

        return not portFound

    @staticmethod
    def recreateMoteList(iterator):
        Motelist.lock.acquire()

        newMotes = list()
        haveNewMote = False
       
        for mote in iterator:
            # this filters out fake motes on linux, i hope!
            if mote[2] == "n/a":
                continue
                
            newMote = Mote(mote)
    
            for m in newMotes:
                if newMote == m:
                    newMote = None
                    break;
            
            if newMote is None:
                continue
                
            # Add if no such mote exists, point to it otherwise
            if newMote not in Motelist.motes:
                newMotes.insert(0, newMote)
                haveNewMote = True
            else:
                newMotes.insert(0, Motelist.motes[Motelist.motes.index(newMote)])

        for mote in Motelist.motes:
            if mote.isUserMote() and mote not in newMotes:
                newMotes.append(mote)
        
        haveNewMote = haveNewMote or not len(Motelist.motes) == len(newMotes)

        Motelist.motes = sorted(newMotes)

        Motelist.lock.release()

        return haveNewMote

    @staticmethod
    def getMotelist(update):
        if update:
            Motelist.updateMotelist(False)

        Motelist.lock.acquire()

        # return a copy of connected list
        retVal = list(Motelist.motes)

        Motelist.lock.release()

        return retVal

    @staticmethod
    def getMoteByUserData(userData):
        Motelist.lock.acquire()

        result = list()

        for mote in Motelist.motes:
            if mote.getUserData() == userData:
                result.append(mote)

        Motelist.lock.release()

        return result

    @staticmethod
    def addUpdateCallback(callback):
        Motelist.updateCallbacks.append(callback)

    @staticmethod
    def removeUpdateCallback(callback):
        try:
            Motelist.updateCallbacks.remove(callback)
        except:
            pass

    @staticmethod
    def updateMotelist(infinite):
        Motelist.infinite = infinite
        Motelist.__activateCallbacks()

        while Motelist.infinite:
            time.sleep(1)
            Motelist.__activateCallbacks()

    @staticmethod
    def startPeriodicUpdate():
        # Call new Thread
        thread = threading.Thread(target = Motelist.updateMotelist, args = (True,),
                                  name = "Motelist thread")

        # Must have, if we don't plan on joining this thread
        thread.daemon = True

        thread.start()

    @staticmethod
    def stopPeriodicUpdate():
        Motelist.infinite = False

    @staticmethod
    def __activateCallbacks(force = False):
        # If no new motes added, no need to call callbacks
        if not Motelist.recreateMoteList(comports()) and not force:
            return

        Motelist.lock.acquire()

        updateCallbackTempList = list(Motelist.updateCallbacks)

        Motelist.lock.release()

        for x in updateCallbackTempList:
            try:
                x()
            except Exception as e:
                print ("Exception while calling callback: ", e)

    @staticmethod
    def portExists(port):
        try:
            ser = serial.Serial(port, 38400, timeout = 0, parity = serial.PARITY_NONE, rtscts = 1)
            while True:
                ser.write("")
                ser.close()
                return True
        except serial.SerialException as msg:
            return False

    @staticmethod
    def getClassicFormatMotelist():

        motelist = Motelist.getMotelist(True)

        if len(motelist) == 0:
            return "No attached motes found!\n"
            
        # Prepare table column width
        lengths = [len("Reference"), len("Port"), len("Host"), len("Name")]

        for mote in motelist:
            lengths[0] = max(lengths[0], len(mote.getReference()))
            lengths[1] = max(lengths[1], len(mote.getPort()))
            lengths[2] = max(lengths[2], len(mote.getHost()))
            lengths[3] = max(lengths[3], len(mote.getName()))

        # Print header
        s = "{0}  {1}  {2}  {3}\n".format("Reference".ljust(lengths[0]),
                                       "Port".ljust(lengths[1]),
                                       "Host".ljust(lengths[2]),
                                       "Name".ljust(lengths[3]))

        # Print seperator
        s += "{0}  {1}  {2}  {3}\n".format("".ljust(lengths[0], "-"),
                                       "".ljust(lengths[1], "-"),
                                       "".ljust(lengths[2], "-"),
                                       "".ljust(lengths[3], "-"))

        # Print motelist
        for mote in motelist:
            s += "{0}  {1}  {2}  {3}\n".format(mote.getReference().ljust(lengths[0]),
                                           mote.getPort().ljust(lengths[1]),
                                           mote.getHost().ljust(lengths[2]),
                                           mote.getName().ljust(lengths[3]))
        return s

    @staticmethod
    def printMotelist():
        print(Motelist.getClassicFormatMotelist())

def main():
    for arg in sys.argv[1:]:
        if arg == "-c":
            for x in Motelist.getMotelist(True):
                print (x.getCSVData())
            sys.exit(1)
        elif arg == "-h":
            print ("Use motelist.py -c for CSV data.")
            sys.exit(1)
    
    Motelist.printMotelist()


if __name__ == '__main__':
    try:
        main()
    except SystemExit:
        raise               #let pass exit() calls
    except KeyboardInterrupt:
        if DEBUG: raise     #show full trace in debug mode
        sys.stderr.write("user abort.\n")   #short messy in user mode
        sys.exit(1)
    #except Exception as msg:
    #    sys.stderr.write("\nAn error occured:\n%s\n" % msg)
    #    sys.exit(1)

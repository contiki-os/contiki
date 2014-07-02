#!/usr/bin/env python

# Copyright (c) 2014, Jens Dede <jd@comnets.uni-bremen.de>.
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
# 1. Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.
# 3. The name of the author may not be used to endorse or promote
#    products derived from this software without specific prior
#    written permission.

# THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS
# OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
# WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
# DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
# GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
# INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
# WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
# NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
# SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

# multiRun.py - run an exported cooja simulation several times
#
# Cooja can export a complete simulation setup to a jar file. This can be
# executed independently from the contiki / cooja environment.
# This python script allows to execute one simulation file
# several times with different seeds. This may be useful for example for
# montecarlo evaluations of a simulation setup.

import argparse
import threading
import Queue
import subprocess
import datetime
import os
import sys
import shutil
import time

class executeThread(threading.Thread):
    ## \brief Worker thread
    #
    # Execute jar file in separate thread.

    commands = Queue.Queue()
    printLock = threading.Lock()

    breakSimulationLock = threading.Lock()
    breakSimulation = False

    def run(self):
        while True:
            # Check if we should break all simulation runs
            breakLoop = False
            executeThread.breakSimulationLock.acquire()
            breakLoop = executeThread.breakSimulation
            executeThread.breakSimulationLock.release()

            if breakLoop:
                break

            # Get new command from the queue, execute and mark as done
            command = executeThread.commands.get()
            self.executeCommand(command)
            executeThread.commands.task_done()

    def executeCommand(self, cmd):
        ## \brief Execute command
        #
        # Execute jar file, into stdout and stderr into files inside working
        # dir. Use command "java -jar <cmd>"
        #
        # \param cmd    Dict with command, working dir, ...

        try:
            # If breakSimulation set: Skip remaining simulation runs
            executeThread.breakSimulationLock.acquire()
            if executeThread.breakSimulation:
                executeThread.breakSimulationLock.release()
                return
            executeThread.breakSimulationLock.release()

            # Output files for stdout and stderr output of executed file
            stdoutFileName = os.path.join(cmd["path"], "stdout.log")
            stderrFileName = os.path.join(cmd["path"], "stderr.log")
            stdoutFile = open(stdoutFileName, "w")
            stderrFile = open(stderrFileName, "w")

            self.log("Starting execution of \"" + cmd['cmd'] + \
                    "\" in path \"" + cmd['path'] + "\"")

            # Start process
            proc = subprocess.Popen(['java','-Djava.awt.headless=true', '-jar', cmd['cmd']],
                    stdout=stdoutFile, stderr=stderrFile, cwd=cmd['path'])

            proc.wait()
            ret = proc.returncode

            # Remove simulation file. Copy available in the base directory
            os.remove(os.path.join(cmd['path'], cmd['cmd']))

            # Return value indicated error and user marked that this breaks
            # execution of remaining simulations
            if ret and cmd["break"]:
                executeThread.breakSimulationLock.acquire()
                executeThread.breakSimulation = True
                executeThread.breakSimulationLock.release()

            self.log("Path \"" + cmd["path"] + "\" done, return code: " + \
                    str(ret))

        except KeyboardInterrupt:
            # Inform others that a problem occured
            executeThread.breakSimulationLock.acquire()
            executeThread.breakSimulation = True
            executeThread.breakSimulationLock.release()

        finally:
            try:
                # Release lock, we could have
                executeThread.breakSimulationLock.release()
                executeThread.printLock.release()
            except threading.ThreadError:
                # Catch errors caused by not having a lock
                pass
            stdoutFile.close()
            stderrFile.close()

    def log(self, msg):
        ## \brief Log function
        #
        # Convenience function for logging
        #
        # \param msg    Log message to print

        # Ensure only one threat at time prints to screen using lock
        executeThread.printLock.acquire()
        print "(" + threading.current_thread().name + ") " + \
            datetime.datetime.now().strftime("%Y-%m-%d_%H:%M:%S") + \
            ": " + str(msg)
        executeThread.printLock.release()


##
# Main App
##

parser = argparse.ArgumentParser(
        description='Run multiple jar files in separate directories and ' +\
                'threads using the command "java -jar <progname>"')
parser.add_argument('progname', help='Name of jar file')
parser.add_argument('--max_threads', type=int, default=4, dest='max_threads',
        help='Number of maximum concurrent threads')
parser.add_argument('--runs', type=int, default=10, dest='runs', metavar="N",
        help='Run given jar file RUNS times (montecarlo simulation)')
parser.add_argument('--break', dest='break_runs', action='store_const',
        const=True, default=False,
        help='Break remaining runs in case of return value is not 0')
parser.add_argument('--execute', dest='exe', metavar="<program name>",
        default=None, help="Execute command when script has finished")

args = parser.parse_args()

print "Number of runs: ", args.runs
print "Number of threads:", args.max_threads
if args.exe:
    print "Executing \"" + args.exe + "\" when done"

if not os.path.isfile(args.progname):
    print "File not found:", args.progname
    sys.exit(1)

# Place each run in separate folder including date
simulationFolderName=datetime.datetime.now().strftime("%Y-%m-%d_%H-%M-%S")

if os.path.exists(simulationFolderName):
    print "Simulation folder exists! Aborting"
    sys.exit(1)

os.makedirs(simulationFolderName)

print "Storing results into", simulationFolderName

# Start threads
threads = [executeThread() for i in range(args.max_threads)]
for thread in threads:
    thread.setDaemon(True)
    thread.start()

cmds = []

# Copy one version of the app to the base directory. The other copies can be
# removed later
shutil.copy(args.progname, os.path.join(os.path.abspath("."), simulationFolderName))

# Generate a separate command for each simulation run. Create corresponding
# folder, execute output inside this folder
for i in range(args.runs):
    progpath = os.path.join(os.path.abspath("."), simulationFolderName, "run_" + str(i+1))
    os.makedirs(progpath)
    shutil.copy(args.progname, progpath)
    metainfo = {}
    metainfo["cmd"] = args.progname
    metainfo["path"] = progpath
    metainfo["break"] = args.break_runs
    cmds.append(metainfo)

# Enqueue commands. Worker thread will take commands out of this queue
for cmd in cmds:
    executeThread.commands.put(cmd)


while (True):
    time.sleep(1)
    # Check if we should break the simulation
    breakEverything = False
    executeThread.breakSimulationLock.acquire()
    breakEverything = executeThread.breakSimulation
    executeThread.breakSimulationLock.release()

    if executeThread.commands.empty():
        # Wait until all threads are done
        executeThread.commands.join()
        break

    if breakEverything:
        break

if args.exe != None:
    print "Executing \"" + args.exe + "\""
    os.system(args.exe)

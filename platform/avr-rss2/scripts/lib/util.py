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

import os, sys, time, threading, re
from subprocess import Popen, PIPE, STDOUT
import socket

from util import *

def isPython3():
    return sys.version_info[0] >= 3

def getUserInput(prompt):
    if isPython3():
        return input(prompt)
    else:
        return raw_input(prompt)

def toBytes(s):
    if isPython3():
        return bytes(s, 'UTF-8')
    return s

def typeIsString(s):
    if isinstance(s, str):
        return True
    if not isPython3():
        return isinstance(s, unicode)
    return False

def toArrayOfStrings(s):
    if not typeIsString(s): return s
    result = []
    parts = s.split(",")
    for p in parts:
        subparts = p.split(" ")
        for sp in subparts:
            sp = sp.strip()
            if len(sp) > 0:
                result.append(sp)
    return result

def toDictionary(s):
    result = {}
    parts = s.split(",")
    for p in parts:
        nameValue = map(lambda s: s.strip(), p.split(":")) 
        if typeIsString(nameValue):
            result[nameValue] = True    # default value
        elif len(nameValue[0]) > 0:
            if len(nameValue) == 1:
                result[nameValue[0]] = True  # default value
            else:
                result[nameValue[0]] = ":".join(nameValue[1:])
    return result

def getInt(s):
    try:
        return int(s)
    except:
        return 0

def isExecutable(filename):
    return os.path.isfile(filename) and os.access(filename, os.X_OK)

def isReadable(filename):
    return os.path.isfile(filename) and os.access(filename, os.R_OK)

def runSubprocess(args, env = None):
    # sys.stdout.write("Run subprocess: " + args + "\n")
    retcode = -1
    alldata = ''
    try:
        proc = Popen(args, stdout = PIPE, stderr = STDOUT, shell = True, env = env)
        while proc.poll() is None:
            # Be careful not to cause a deadlock: read by small portions until each newline.
            # Warning! If the child process can produce a lot of data without newlines,
            # the Python code must be modified to use limited-sized read(N) instead of readline()!
            data = proc.stdout.readline()
            if data:
                sys.stdout.write(data)
                alldata += data
            time.sleep(0.001)
        data = proc.stdout.read()
        if data:
            sys.stdout.write(data)
            alldata += data
        retcode = proc.returncode
    except OSError as e:
        print("run subprocess OSError:" + str(e))
    except CalledProcessError as e:
        print("run subprocess CalledProcessError:" + str(e))
        retcode = e.returncode
    except Exception as e:
        print("run subprocess exception:" + str(e))
    finally:
        #print("done, retcode = " + str(retcode))
        return (retcode, alldata)

def undefined(x, localVariables):
    return x not in localVariables

def createBackgroundThread(function, args):
    # make sure 'args' is a list or a tuple
    if not (type(args) is list or type(args) is tuple):
        args = (args,)
    t = threading.Thread(target=function, args=args)
    # keep it in daemon mode, in order to able to kill the app with Ctrl+C
    t.daemon = True
    t.start() 

# extract a boolean value from http query string
def qsExtractBool(qs, name, defaultValue = None):
    value = defaultValue
    if name in qs:
        arg = qs[name][0]
        try:
            value = bool(int(arg, 0))
        except:
            try:
                value = bool(arg)
            except:
                value = defaultValue
    return value

# extract a string value from http query string
def qsExtractString(qs, name, defaultValue = None):
    value = defaultValue
    if name in qs:
        try:
            value = qs[name][0]
        except:
            value = qs[name]
    return value

def qsExtractInt(qs, name, defaultValue = None):
    value = defaultValue
    if name in qs:
        try:
            value = int(qs[name][0])
        except:
            try:
                value = int(qs[name])
            except:
                value = defaultValue
    return value

def qsExtractFloat(qs, name, defaultValue = None):
    value = defaultValue
    if name in qs:
        try:
            value = float(qs[name][0])
        except:
            try:
                value = float(qs[name])
            except:
                value = defaultValue
    return value

def qsExtractList(qs, name):
    return qs.get(name, [])

def extractRimeAddress(line, prefix = '', suffix = ''):
    m = re.search(prefix + '([0-9]+)\.([0-9]+)' + suffix, line)
    if not m is None:
        try:
            return int(m.group(2)) * 256 + int(m.group(1))
        except:
            pass
    return None

# works with arrays of JSON objects
def arrayToDict(array, keyName):
    keys = map(lambda x: x["task"], array)
    return dict(zip(keys, array))

# Rime address to string
def raddrStr(address):
    return "{}.{}".format(address & 255, address / 256)


# Polynomial ^8 + ^5 + ^4 + 1
def crc8Add(acc, byte):
    acc ^= byte
    for i in range(8):
        if acc & 1:
            acc = (acc >> 1) ^ 0x8c
        else:
            acc >>= 1
    return acc

def crc8(s):
    acc = 0
    for c in s:
        acc = crc8Add(acc, ord(c))
    return acc

def getFloatSafe(s):
    try:
        return float(s)
    except:
        return 0.0

# finds a free numeric key; uses linear search
def findUnusedKey(container, hint, lower, upper):
    if hint is not None and hint < upper:
        hint += 1
        if hint not in container:
            return hint
    for i in range(lower, upper + 1):
        if i not in container:
            return i
    return None

# limit the maximum length of a list by cutting from front
def limitLength(lst, count):
    if len(lst) < count:
        return lst
    return lst[(len(lst) - count + 1):]

# regexp helper
class Matcher:
    def __init__(self, pattern, flags=0):
        self._pattern = re.compile(pattern, flags)
        self._hit = None
    def match(self, line):
        self._hit = re.match(self._pattern, line)
        return self
    def search(self, line):
        self._hit = re.search(self._pattern, line)
        return self._hit
    def matched(self):
        return self._hit != None
    def group(self, idx):
        return self._hit.group(idx)
    def as_int(self, idx):
        return int(self._hit.group(idx))

# return true if a symbols is non-binary ASCII character
def isascii(c, printable = True):
    if 0x00 <= ord(c) <= 0x7f:
        if 0x20 <= ord(c) <= 0x7e:
            return True
        if printable and (c == '\r' or c == '\n' or c == '\t'):
            return True
        return False
    else:
        return False

# return true if all symbols are non-binary ASCII chars (ord(c) < 128)
def isasciiString(s):
    for c in s:
        if not isascii(c, False):
            return False
    return True

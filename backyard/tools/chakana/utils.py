
#  Copyright (C) 2003-2007 Swedish Institute of Computer Science.
#  
# Please refer to the file named LICENSE in the same directory as this
# file for licensing information.
#  


# $Id: utils.py,v 1.1 2009/02/26 13:47:38 fros4943 Exp $

import errno
import math
import operator
import os
import popen2
import re
import string
import shutil
import sys
import tempfile
import threading
import traceback
import types

import chakana.command
import chakana.error
from chakana.debug import *

try:
  global False
  global True
  _ = False
except NameError:
  False = 0
  True = 1

class DefaultDict(dict):
  def __init__(self, defaultCreator, initialiser = (), inject = 0):
    dict.__init__(self, initialiser)
    if callable(defaultCreator):
      self._defaultCreator = defaultCreator
    else:
      self._defaultCreator = lambda k: defaultCreator
    self._inject = inject

  def __getitem__(self, key):
    try:
      return dict.__getitem__(self, key)
    except KeyError, err:
      value = self._defaultCreator(key) 
      if self._inject:
        self[key] = value
        return self[key]
      else:
        return value
        
class UnbufferedFile:
  def __init__(self, fileObj):
    self._fileObj = fileObj

  def __getattr__(self, attribute):
    return getattr(self._fileObj, attribute)

  def write(self, what):
    self._fileObj.write(what)
    self._fileObj.flush()

def copyFile(src, dest):
  debug(Event, "cp " + src + " " + dest)
  srcFile = file(src)
  destFile = file(dest, "w")
  destFile.write(srcFile.read())
  srcFile.close()
  destFile.close()

def unique(list):
  ret = []
  inList = {}
  for elem in list:
    if not elem in inList:
      ret.append(elem)
      inList[elem] = 1
  return ret

def unorderedEqual(listOne, listTwo, predicate = lambda x, y: x == y):
  if len(listOne) != len(listTwo):
    return 0
  list1 = list(listOne)
  list2 = list(listTwo)
  while list1 != []:
    found = 0
    for index in range(len(list2)):
      if predicate(list1[0], list2[index]):
        del list1[0]
        del list2[index]
        found = 1
        break
    if not found:
      return 0
  return 1

def reduceSequences(tupleSeq, oper = operator.add):
  """Return a list, where the item at position i is the reduced value
  of all items at position i."""

  if len(tupleSeq) == 0:
    return []
  return [reduce(oper, [s[i] for s in tupleSeq])
          for i in range(len(tupleSeq[0]))]

def listRemove(theList, predicate):
  """Remove all items that match predicate in theList."""
  if not callable(predicate):
    return listRemove(theList, lambda x: x == predicate)
  else:
    index = 0
    while index < len(theList):
      if predicate(theList[index]):
        del theList[index]
      else:
        index += 1
  return theList

def logn(n, x):
  return math.log(x) / math.log(n)

def longRange(* args):
  """As range, but works with long integers."""
  if len(args) == 1:
    start = 0L
    stop = args[0]
    step = 1L
  elif len(args) == 2:
    (start, stop) = args
    step = 1L
  else:
    (start, stop, step) = args
  if step == 0:
    raise ValueError("longRange() arg 3 must not be zero")
  ret = []
  while cmp(start, stop) == cmp(0, step):
    ret.append(start)
    start += step
  return ret

integerSuffixes = ("", "K", "M", "G", "T")

def abbreviateInteger(i, factor = 1024, minimise = 0):
  suf = 0
  while suf < len(integerSuffixes) - 1 and \
        ((i > 9999) or (minimise and (i != 0) and (i % factor == 0))):
    suf += 1
    i /= factor
  return str(i) + integerSuffixes[suf]

def readSuffixedInt(intString, factor = 1024):
  if intString[-1].isalpha():
    return int(intString[: -1]) * (
      factor ** list(integerSuffixes).index(intString[-1].upper()))
  else:
    return int(intString)

def normaliseIndex(index, seq):
  """Return a non-negative index relating to a sequence, or negative if
  index < - len(seq)."""
  
  if index < 0:
    return len(seq) + index
  else:
    return index

def truncateIndex(index, seq):
  """Return a non-negative index relating to a sequence, less than or equal
  to the length of the sequence."""
  ret = normaliseIndex(index, seq)
  if ret < 0:
    return 0
  length = len(seq)
  if ret > length:
    return length
  return ret

class LazySlice:
  """Representation of slice into other sequence."""
  def __init__(self, seq, start = None, stop = None, step = None):
    self._sequence = seq
    if step is None:
      self._step = 1
    else:
      self._step = step
    if self._step == 0:
      # Provoke ValueError
      [][0:1:0]

    if self._step > 0:
      startDefault = 0
      stopDefault = len(seq)
      indexShift = 0
    else:
      startDefault = len(seq) - 1
      stopDefault = -1
      indexShift = 1
      
    if start is None:
      self._start = startDefault
    else:
      self._start = truncateIndex(start + indexShift, seq) - indexShift
    if stop is None:
      self._stop = stopDefault
    else:
      self._stop = truncateIndex(stop + indexShift, seq) - indexShift

    if (self._step > 0) != (self._start < self._stop):
      self._stop = self._start

  def __getitem__(self, key):
    # debug(Debug2, "LS getitem " + str(key) + ": " + repr(self))
    if type(key) == types.SliceType:
      ret = LazySlice(self, key.start, key.stop, key.step)
      #debug(Debug2, "LS getitem " + str(key) + " = " +
      #      repr(ret) + ", LS: " + repr(self))
      return ret
    index = normaliseIndex(key, self)
    if index < 0:
      # Provoke IndexError
      [][1]
    seqIndex = self._start + index * self._step
    if self._step > 0:
      if seqIndex >= self._stop:
        [][1]
    elif seqIndex <= self._stop:
      [][1]
    ret = self._sequence[seqIndex]
    #debug(Debug2, "LS getitem " + str(key) + " (" + str(seqIndex) + ") = " +
    #      str(ret) + ", LS: " + repr(self))
    return ret          

  def __len__(self):
    return (abs(self._stop - self._start) + abs(self._step) - 1) / \
           abs(self._step)

  def __str__(self):
    return str(list(self))

  def __repr__(self):
    return "LazySlice(start = " + str(self._start) + ", stop = " + \
           str(self._stop) + ", step = " + str(self._step) + ",\n  " + \
           repr(self._sequence) + ")"

def getItem(sequence, key):
  """Obtain key in sequence.  Key may be an integer or a slice object."""
  if type(key) == types.SliceType:
    if key.step is None:
      return sequence[key.start : key.stop]
    return sequence[key.start : key.stop : key.step]
  return sequence[key]

def binarySearch(list, value, start = 0, end = None):
  "Return the position where value should be inserted in a sorted list."
  if end is None:
    end = len(list)
  if start == end:
    return end
  middle = start + (end - start) / 2
  if list[middle] < value:
    return binarySearch(list, value, middle + 1, end)
  else:
    return binarySearch(list, value, start, middle)

def binaryFind(list, value, *args, ** kwArgs):
  "Return position of value in sorted list, or None if not found."
  position = binarySearch(list, value, *args, ** kwArgs)
  if (position == len(list)) or (list[position] != value):
    return None
  else:
    return position
  
def compactIntListRepr(input, sort = 0):
  if sort:
    list = input[:]
    list.sort()
  else:
    list = input
    if __debug__:
      tmp = list[:]
      tmp.sort()
      assert(tmp == list)
  index = 0
  ret = "["
  sep = ""
  compact = lambda l, i: (i + 1 < len(l)) and (l[i] + 1 == l[i + 1])
  while index < len(list):
    ret += sep + str(list[index])
    if compact(list, index):
      ret += "-"
      while compact(list, index):
        index += 1
      ret += str(list[index])
    index += 1
    sep = ","
  return ret + "]"

def predicatedIndex(sequence, predicate):
  for i in range(len(sequence)):
    if predicate(sequence[i]):
      return i
  raise ValueError()

class ArgumentBinder:
  def __init__(self, function, argument, position = 0):
    self.__function = function
    self.__argument = argument
    self.__position = position

  def __call__(self, *args, **keywords):
    newArgs = list(args)
    newArgs[self.__position : self.__position] = [self.__argument]
    return self.__function(*newArgs, **keywords)

class ArgumentTupleBinder:
  def __init__(self, function, arguments):
    self._function = function
    self._arguments = arguments

  def __call__(self, * args, ** kwArgs):
    return self._function(* (self._arguments + args), ** kwArgs)

class KeywordArgumentBinder:
  def __init__(self, function, ** keywords):
    self._function = function
    self._keywords = keywords

  def __call__(self, * args, ** kwArgs):
    kw = self._keywords
    kw.update(kwArgs)
    return self._function(* args, ** kw)

cppKeywords = ["asm", "do", "inline", "short", "typeid", "auto",
               "double", "int", "signed", "typename", "bool",
               "dynamic_cast", "long", "sizeof", "union", "break",
               "else", "mutable", "static", "unsigned", "case",
               "enum", "namespace", "static_cast", "using", "catch",
               "explicit", "new", "struct", "virtual", "char",
               "extern", "operator", "switch", "void", "class",
               "false", "private", "template", "volatile", "const",
               "float", "protected", "this", "wchar_t", "const_cast",
               "for", "public", "throw", "while", "continue",
               "friend", "register", "true", "default", "goto",
               "reinterpret_cast", "try", "delete", "if", "return",
               "typedef"]

def isCWord(str):
  return not re.match("^[a-zA-Z_][a-zA-Z0-9_]*$", str) is None

def isCppIdentifier(str):
  return isCWord(str) and (str not in cppKeywords)

def find(seq, predicate = lambda x: x):
  for item in seq:
    if predicate(item):
      return item
  return None

def allBest(seq, comparator = cmp):
  if len(seq) < 2:
    return seq
  seq = seq[:]
  ret = [seq.pop()]
  for item in seq:
    c = comparator(item, ret[0])
    if c < 0:
      ret = [item]
    elif c == 0:
      ret.append(item)
  return ret

def readFile(fileName):
  f = open(fileName)
  try:
    ret = f.read()
    return ret
  finally:
    f.close()

def writeFile(fileName, contents):
  f = open(fileName, "w")
  try:
    f.write(contents)
  finally:
    f.close()

def writeFileAtomic(fileName, contents, suffix = ".tmp"):
  tmpFile = fileName + suffix
  writeFile(tmpFile, contents)
  try:
    os.rename(tmpFile, fileName)
  except:
    if os.path.exists(fileName) and os.path.exists(tmpFile):
      os.remove(tmpFile)
    raise


class LineIndexed:
  """Read-only file object indexed by line number.  The file object must
  support seeking."""
  
  def __init__(self, fileName, offset = 0):
    if type(fileName) == type(""):
      self._file = file(fileName)
    else:
      self._file = fileName      
    self._offset = offset
    self._length = None
    self._lineOffsets = [0]

  def close(self):
    self._file.close()

  def __del__(self):
    self.close()

  def _seekForward(self, index):
    if self._length == 0:
      raise IndexError()
    for index in range(len(self._lineOffsets), index + 1):
      if self._lineOffsets[-1] == self._length:
        raise IndexError()
      self._file.seek(self._lineOffsets[-1] + self._offset)
      line = self._file.readline()
      if line == "":
        self._length = self._lineOffsets[-1]
      else:
        self._lineOffsets.append(self._lineOffsets[-1] + len(line))

  def __getitem__(self, key):
    if type(key) == types.SliceType:
      ret = LazySlice(self, key.start, key.stop, key.step)
      return ret
    if key < 0:
      positiveLength = len(self) + key
      if positiveLength < 0:
        raise IndexError()
      return self[positiveLength]
    self._seekForward(key)
    offset = self._lineOffsets[key]
    if offset == self._length:
      raise IndexError()
    self._file.seek(offset + self._offset)
    return self._file.readline()

  def __len__(self):
    if self._length == 0:
      return 0
    while self._lineOffsets[-1] != self._length:
      self._seekForward(len(self._lineOffsets))
    return len(self._lineOffsets) - 1

  def lineOffset(self, index):
    self._seekForward(index)
    return self._lineOffsets[index]

  def __str__(self):
    return str(list(self))

  def __repr__(self):
    return "LineIndexed(offset = " + str(self._offset) + ", length = " + \
           str(self._length) + ", lineOffsets: " + \
           str(self._lineOffsets) + ")"

def getLine(fileName, count):
  fileObj = file(fileName)
  if count < 0:
    return fileObj.readlines()[count]
  for i in range(count + 1):
    ret = fileObj.readline()
    if ret == "":
      raise IndexError()
  return ret

def conditionalJoin(list, separator = " ", predicate = lambda e: e != ""):
  if len(list) == 0:
    return ""
  rest = conditionalJoin(list[1:], separator, predicate)
  if not predicate(rest):
    return list[0]
  if not predicate(list[0]):
    return rest
  else:
    return list[0] + separator + rest

def pathComponents(path):
  (head, tail) = os.path.split(path)
  if tail == "":
    if head == "":
      return []
    if head == "/":
      return ["/"]
    else:
      return pathComponents(head)
  else:
    return pathComponents(head) + [tail]

def commonFirstElements(listOfLists):
  if len(listOfLists) == 0:
    return []
  if len(listOfLists) == 1:
    return listOfLists[0]
  if (len(listOfLists[0]) == 0) or (len(listOfLists[1]) == 0) or \
     (listOfLists[0][0] != listOfLists[1][0]):
    return []
  else:
    return commonFirstElements([
      [listOfLists[0][0]] +
      commonFirstElements([listOfLists[0][1:], listOfLists[1][1:]])]
      + listOfLists[2:])

def commonPrefix(pathList):
  """Similar to os.path.commonprefix, but works on path components instead
  of individual characters."""

  assert(not isinstance(pathList, str))
  components = commonFirstElements(map(pathComponents, pathList))
  if components == []:
    return ""
  else:
    return os.path.join(* components)

def pathIsBelow(path, directory):
  """Check if path lies below directory."""
  return commonPrefix([path, directory]) == directory

def realPath(path):
  """Similar to os.path.realpath, but handles amd gracefully."""
  ret = os.path.realpath(path)
  for binDir in os.environ["PATH"].split(":") + ["/usr/sbin", "/sbin"]:
    amq = os.path.join(binDir, "amq")
    if os.path.isfile(amq):
      try:
        output = chakana.command.output(amq)
      except chakana.error.CommandFailed, err:
        debug(Debug, str(err))
        # Assume amd is not running
        return ret
      for line in output.splitlines():
        amdDir = line.split()[0]
        mountDir = line.split()[-1]
        if mountDir[0] == "/":
          match = re.match('^(' + re.escape(line) + ')(/$)', ret)
          if match:
            return amdDir + ret[len(line) :]
      return ret
  return ret
  
def removeTree(path, ignoreErrors = False):
  debug(Debug, "rm -rf " + path)
  if os.path.isdir(path):
    shutil.rmtree(path, ignore_errors = ignoreErrors)

def makeSymlink(value, dest, force = True, debugLevel = MinorEvent,
                dryRun = False):
  if os.path.islink(dest):
    if os.readlink(dest) == value:
      debug(Debug, "Link " + dest + " already points to " + value)
      return
    elif force:
      debug(debugLevel, "Removing " + dest)
      if not dryRun:
        os.remove(dest)
    else:
      raise OSError((errno.EEXIST, "Link already exists", dest))
  absValue = os.path.join(os.path.dirname(dest), value)
  if (not dryRun) and (not force) and (not os.path.isfile(absValue)):
    raise OSError((errno.ENOENT, "Link destination does not exist", absDest))
  debug(debugLevel, "Linking " + dest + " to " + value)
  if not dryRun:
    os.symlink(value, dest)

def copyTree(src, dst, symlinks = False, predicate = lambda path: 1,
             preserveTimes = False):
  """Similar to shutil.copytree, but allows existing destination and
  passes exceptions."""
  names = filter(predicate, os.listdir(src))
  if not os.path.isdir(dst):
    os.mkdir(dst)
  for name in names:
    srcname = os.path.join(src, name)
    dstname = os.path.join(dst, name)
    try:
      if symlinks and os.path.islink(srcname):
        linkto = os.readlink(srcname)
        os.symlink(linkto, dstname)
      elif os.path.isdir(srcname):
        copyTree(srcname, dstname, symlinks, predicate, preserveTimes)
      else:
        if preserveTimes:
          shutil.copy2(srcname, dstname)
        else:
          shutil.copy(srcname, dstname)
        # XXX What about devices, sockets etc.?
    except (IOError, os.error), why:
      debug(Error, "Can't copy %s to %s: %s" %
            (`srcname`, `dstname`, str(why)))
      raise

def makeDirsSafe(dir, mode = 0777):
  """Similar to os.makedirs, but does not fail if another process is making
  the directory simultaneously."""
  while not os.path.isdir(dir):
    try:
      os.makedirs(dir, mode)
    except OSError, err:
      if err.errno != errno.EEXIST:
        raise

def typeAssert(variable, type):
  assert(isinstance(variable, type))

def intervalsOverlap(range1, range2):
  if range1 > range2:
    return intervalsOverlap(range2, range1)
  if (range1[0] == range1[1]) or (range2[0] == range2[1]):
    return 0
  return range2[0] < range1[1]

class LexicalDistance:
  def __init__(self, caseChange = 1, whiteSpaceChange = 2,
               whiteSpaceRemoval = 3, ampersand = 5, other = 10,
               whiteSpace = " _-'\""):
    self.__caseChange = caseChange
    self.__whiteSpaceChange = whiteSpaceChange
    self.__whiteSpaceRemoval = whiteSpaceRemoval
    self.__ampersand = ampersand
    self.__other = other
    self.__whiteSpace = whiteSpace

  def removeWhiteSpace(self, str1, str2, limit):
    if str1[0] in self.__whiteSpace:
      if limit > self.__whiteSpaceRemoval:
        return self(str1[1:], str2, limit - self.__whiteSpaceRemoval) + \
               self.__whiteSpaceRemoval
    return limit

  def changeWhiteSpace(self, str1, str2, limit):
    if (str1[0] in self.__whiteSpace) and (str2[0] in self.__whiteSpace):
      if limit > self.__whiteSpaceChange:
        return self(str1[1:], str2[1:], limit - self.__whiteSpaceChange) + \
               self.__whiteSpaceChange
    return limit

  def changeCase(self, str1, str2, limit):
    if str1[0].upper() == str2[0].upper():
      if limit > self.__caseChange:
        return self(str1[1:], str2[1:], limit - self.__caseChange) + \
               self.__caseChange
    return limit

  def changeAmpersand(self, str1, str2, limit):
    if (str1[0] == "&") and (str2[:3].lower() == "and"):
      if limit > self.__ampersand:
        return self(str1[1:], str2[3:], limit - self.__ampersand) + \
               self.__ampersand
    return limit
  
  def removeOther(self, str1, str2, limit):
    if limit > self.__other:
      return self(str1[1:], str2, limit - self.__other) + self.__other
    return limit

  def changeOther(self, str1, str2, limit):
    if limit > self.__other:
      return self(str1[1:], str2[1:], limit - self.__other) + self.__other
    return limit
    
  def __call__(self, str1, str2, limit = 50):
    ret = limit
    if str1 == "":
      if str2 == "":
        return 0
      ret = self.removeWhiteSpace(str2, str1, ret)
      ret = self.removeOther(str2, str1, ret)
    elif str2 == "":
      ret = self(str2, str1, ret)
    else:
      if str1[0] == str2[0]:
        ret = self(str1[1:], str2[1:], ret)
      else:
        ret = self.removeWhiteSpace(str1, str2, ret)
        ret = self.removeWhiteSpace(str2, str1, ret)
        ret = self.changeWhiteSpace(str1, str2, ret)
        ret = self.changeCase(str1, str2, ret)
        ret = self.changeAmpersand(str1, str2, ret)
        ret = self.changeAmpersand(str2, str1, ret)
        ret = self.changeOther(str1, str2, ret)
        ret = self.removeOther(str1, str2, ret)
        ret = self.removeOther(str2, str1, ret)
    assert(ret <= limit)
    return ret
  
class TryOperation:
  def __init__(self, operation, description = None):
    self.__operation = operation
    if description is None:
      self.__description = str(operation)
    else:
      self.__description = description

  def __call__(self, * args, ** kwargs):
    try:
      self.__operation(* args, ** kwargs)
    except KeyboardInterrupt:
      raise
    except:
      import debug as d
      d.debug(d.Error, self.__description + " failed\n" +
              d.exceptionDump(sys.exc_info()))
  
def lineDirective():
  fileInfo = traceback.extract_stack(None, 2)[0]
  return "#line " + repr(fileInfo[1] + 1) + " \"" + fileInfo[0] + "\""

try:
  mkstemp = tempfile.mkstemp
except AttributeError:
  def mkstemp(suffix = "", prefix = None, dir = None, text = 0):
    if prefix is None:
      prefix = tempfile.gettempprefix()
    fileName = tempfile.mktemp(suffix)
    if dir is None:
      dir = os.path.dirname(fileName)
    newFileName = os.path.join(dir, prefix + os.path.basename(fileName))
    if text:
      flags = "w"
    else:
      flags = "wb"
    return (file(newFileName, flags), newFileName)


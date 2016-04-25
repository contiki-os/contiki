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
#
# Author: Atis Elsts, 2014-2016
#


functionNameToIdMapping = {
    "temperature" : 1,
    "humidity" : 2,
    "light" : 3,
    "random" : 4,
    "accelerometer" : 5,
    "battery" : 6,
    "smoke" : 7,

    "alarm" : 101,
    "heater" : 102,
    "relay" : 103,
    "print" : 104,
    "leds" : 105,

    "threshold" : 201,
    "range" : 202,
    "delay" : 203,
    "PID controller" : 204,

    "inject" : 230,
    "collect" : 231,

    "test_task" : 99,
}

def findFunctionNameById(id):
    for n in functionNameToIdMapping:
        if functionNameToIdMapping[n] == id: return n
    return None


################################################

def getTaskParameters(task):
    taskType = task.get("type", None)
    if taskType == "threshold":
        return getThresholdTaskParameters(task)
    elif taskType == "random":
        return getRandomTaskParameters(task)
    elif taskType == "range":
        return getRangeTaskParameters(task)
    elif taskType == "inject":
        return getInjectTaskParameters(task)
    elif taskType == "collect":
        return getCollectTaskParameters(task)
    elif taskType == "accelerometer":
        return getAccelTaskParameters(task)
    elif taskType == "delay":
        return getDelayTaskParameters(task)
    return None

thresholdTypeToIntMap = {
    "greater" : 0,
    "greaterEqual" : 1,
    "lower" : 2,
    "lowerEqual" : 3,
    "equal" : 4,
    "notEqual" : 5
}

def getThresholdTaskParameters(task):
    try:
        thresholdType = task.get("thresholdType", "")
    except:
        thresholdType = ""
    thresholdTypeInt = thresholdTypeToIntMap.get(thresholdType, -1)
    try:
        thresholdValue = int(task.get("threshold", 0))
    except:
        thresholdValue = 0
    return (thresholdTypeInt, thresholdValue)

def getRandomTaskParameters(task):
    outMin = task.get("outputMin", "0")
    outMax = task.get("outputMax", "0")
    return (outMin, outMax)

def getRangeTaskParameters(task):
    inMin = task.get("inputMin", "0")
    inMax = task.get("inputMax", "0")
    outMin = task.get("outputMin", "0")
    outMax = task.get("outputMax", "0")
    return (inMin, inMax, outMin, outMax)

outputTypeToIntMap = {
    "random" : 0,
    "increasing" : 1,
    "decreasing" : 2
}

def getInjectTaskParameters(task):
    outMin = task.get("outputMin", "0")
    outMax = task.get("outputMax", "0")
    outTimes = task.get("outputTimes", "0")
    outType = outputTypeToIntMap.get(task.get("outputType", "random"), 0)
    return (outMin, outMax, outTimes, outType)

def getCollectTaskParameters(task):
    retransmissions = task.get("retransmissions", "15")
    return (retransmissions)

axisToIntMap = {
    "x" : 0,
    "y" : 2,
    "z" : 4
}

def getAccelTaskParameters(task):
    axis = task.get("axis", "x")
    try:
        axisInt = axisToIntMap[axis]
    except:
        axisInt = 0
    return (axisInt)


def getUnitMultiplier(units):
    if units == "milliseconds" or units == "":
        return 1
    if units == "seconds":
        return 1000
    if units == "minutes":
        return 1000 * 60
    if units == "hours":
        return 1000 * 60 * 60
    if units == "days":
        return 1000 * 60 * 60 * 24
    print("Delay parameters: invalid time units: " + units)
    return 1

def convertToMs(time, units):
    try:
        time = int(time)
    except:
        print("Delay parameters: invalid time format: " + str(time))
        return time

    return time * getUnitMultiplier(units)

DELAY_TYPE_DELAY      = 0x0
DELAY_TYPE_RATELIMIT  = 0x1
DELAY_TYPE_RANDOM     = 0x2

def getDelayTaskParameters(task):
    pauseType = task.get("pauseType")

    if pauseType == "delay":
        typeId = DELAY_TYPE_DELAY
        pause = convertToMs(task.get("timeout"), task.get("timeoutUnits", ""))

    elif pauseType == "rate":
        typeId = DELAY_TYPE_RATELIMIT
        try:
            frequency = int(task.get("rate"))
            period = 1. / frequency
            pause = int(period * getUnitMultiplier(task.get("rateUnits", "")))
        except:
            print("Delay parameters: invalid time format: " + str(time))
            pause = 0

    elif pauseType == "random":
        typeId = DELAY_TYPE_RANDOM
        pause = convertToMs(task.get("random"), task.get("randomUnits", ""))

    else:
        print("Invalid delay format: " + str(time))
        typeId = 0xffff
        pause = 0

    return (typeId, 0, pause >> 16, pause & 0xffff)


#
# Copyright (c) 2015 NXP B.V.
# All rights reserved.
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
# 1. Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.
# 3. Neither the name of NXP B.V. nor the names of its contributors
#    may be used to endorse or promote products derived from this software
#    without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY NXP B.V. AND CONTRIBUTORS ``AS IS'' AND
# ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED.  IN NO EVENT SHALL NXP B.V. OR CONTRIBUTORS BE LIABLE
# FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
# OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
# HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
# LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
# OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
# SUCH DAMAGE.
#
# This file is part of the Contiki operating system.
#
# Author: Theo van Daele <theo.van.daele@nxp.com>
#
#

import sys
import time
import socket
import thread
import numpy as np
import Gnuplot

# Plot buffer length for each node buffer
SIZE_NODE_DATA = 600

# Repeat time for connect watchdog and ping
TICK = 5
# Connect watchdog. Unit: [TICK]
CONNECT_WATCHDOG = 30/TICK

TX_PORT = 8185
RX_PORT = 8186

last_sample_value = 0
x_range = np.arange(SIZE_NODE_DATA)
dummy_data = np.zeros(SIZE_NODE_DATA, dtype=np.int)
plot = Gnuplot.Data(x_range,dummy_data)
g = Gnuplot.Gnuplot()

# Attention: Skip leading zero's in IPv6 address list, otherwise address comparison with received UDP packet address goes wrong
NODE_ALIAS = 0            # node name in plot
NODE_DATA = 1             # Time line with data to plot
NODE_CONNECTED = 2        # If 0, not connected, else connect_watchdog value
NODE_LAST_DATA = 3        # Storage for received sample
NODE_PLOT = 4             # Plot instance
node_data = {"bbbb::215:8d00:36:180"  : ["NODE-001",     np.zeros(SIZE_NODE_DATA, dtype=np.int), 0, last_sample_value, plot],
             "bbbb::215:8d00:36:892"  : ["NODE-196",     np.zeros(SIZE_NODE_DATA, dtype=np.int), 0, last_sample_value, plot],
             "bbbb::215:8d00:36:8b1"  : ["NODE-193",     np.zeros(SIZE_NODE_DATA, dtype=np.int), 0, last_sample_value, plot],
             "bbbb::215:8d00:36:8b3"  : ["NODE-198",     np.zeros(SIZE_NODE_DATA, dtype=np.int), 0, last_sample_value, plot]}

# List of all nodes derived from node_data list
node_list = node_data.keys()

def initPlots():
    for node in range(len(node_list)):
        node_data_key = node_data[node_list[node]]
        g.title('Sensor Network Output')
        g.ylabel('Value')
        g.xlabel('Time-Span[s]')
        g("set yrange [-150:150]")
        g("set xrange [0:"+str(SIZE_NODE_DATA)+"]")
        node_data_key[NODE_PLOT] = Gnuplot.Data(x_range,node_data_key[NODE_DATA], title=node_data_key[NODE_ALIAS] , with_='lines lw 2')
    nodes = [ node_data[node_list[i]][NODE_PLOT] for i in xrange(len(node_list)) ]
    g.plot(*nodes)



def udpReceive():
    """RUNS ON SEPARATE THREAD """
    while True:
        data, addr = s_rx.recvfrom(128)
        data = data.replace("\"","").strip("\0") # strip termination byte and possible
        node_data_key = node_data[addr[0]]
        # Indicate node is connected
        node_data_key[NODE_CONNECTED] = CONNECT_WATCHDOG
        print addr[0] + ' (' + node_data_key[NODE_ALIAS] + ') : ' + data
        # Write new data at index in data buffer. Data buffer is view of plot
        data_lock.acquire()
        node_data_key[NODE_LAST_DATA] = int(data)
        data_lock.release()

def plotGraphs():
    while True:
        data_lock.acquire()
        for node in range(len(node_list)):
            node_data_key = node_data[node_list[node]]
            for k in range(1,SIZE_NODE_DATA,1):
                node_data_key[NODE_DATA][k-1] = node_data_key[NODE_DATA][k]
            node_data_key[NODE_DATA][SIZE_NODE_DATA-1] = node_data_key[NODE_LAST_DATA]
            if node_data_key[NODE_CONNECTED] == 0:
                node_data_key[NODE_PLOT] = Gnuplot.Data(x_range,node_data_key[NODE_DATA], title=node_data_key[NODE_ALIAS], with_='dots')
            else:
                node_data_key[NODE_PLOT] = Gnuplot.Data(x_range,node_data_key[NODE_DATA], title=node_data_key[NODE_ALIAS], with_='lines lw 2')
        nodes = [ node_data[node_list[i]][NODE_PLOT] for i in xrange(len(node_list)) ]
        g.plot(*nodes)
        data_lock.release()
        time.sleep(1)


##### MAIN #####
s_tx = socket.socket(socket.AF_INET6, socket.SOCK_DGRAM)
s_rx = socket.socket(socket.AF_INET6, socket.SOCK_DGRAM)
s_rx.bind(('', RX_PORT))
initPlots()
data_lock = thread.allocate_lock()
thread.start_new_thread(udpReceive, ())
thread.start_new_thread(plotGraphs, ())
ping_node_index = 0
ping_msg = "ping"
while True:
    # Every 5 secs, one node of the list in pinged
    if (ping_node_index >=len(node_list)):
        ping_node_index = 0;
    try:
        print "ping " + node_data[node_list[ping_node_index]][NODE_ALIAS]
        s_tx.sendto(ping_msg, (node_list[ping_node_index], TX_PORT))
    except:
        print 'Failed to send to ' + node_list[ping_node_index]
    ping_node_index += 1
    # Update connect watchdog
    for node in range(len(node_list)):
        node_data_key = node_data[node_list[node]]
        if (node_data_key[NODE_CONNECTED] > 0):
            node_data_key[NODE_CONNECTED] -= 1
    time.sleep(TICK)






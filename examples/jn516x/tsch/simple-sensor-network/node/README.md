Node is part of Simple-Sensor-Network
When node is connected to border-router (TSCH), it will wait to be addressed by a host
on port 8185. 
When addressed, IPv6 address of host is acquired. Node will start to transmit samples of
a waveform ("simulated" sensor data) to the host on port 8186 at a rate of once per 
10 seconds. Type of waveform and phase depend on mac address of node in order to better 
distinguish the waveforms at the host side.
LED is flashed every time a sample is transmitted.
Nodes are tested with configuration JN516x_WITH_DONGLE=1


The tx-power-control example can be used to check the link between a node (tx-power-control-node) and a
border-router (tx-power-control-router).

After a link is established between tx-power-control-node and tx-power-control-router, the TX power 
control is exposed as a PUT/POST coap resource named Set-TX-Power. 
The GET coap resource Get-TX-Power will show the actual TX power level that has been established with 
the TX power setpoint.

The tx-power-control-router shows 2 GET resources:
Get-RSSI: This shows the detected energy level
Get-Last-RSSI: This shows the RSSI of the last received packet. When there is only 1 node, Get-Last-RSSI will
show the effect of controlling the TX power level.

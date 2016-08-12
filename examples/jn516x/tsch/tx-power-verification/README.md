The tx-power-control example can be used to check the link between a node and an rpl-border-router.
After a link is established, the following coap rresources are exposed:

node:
Set-Tx-Power: a PUT/POST resource to set the TX power of the device. The firmware in the device will set the
              TX power to the nearest available value.
Get-Tx-Power: a GET resource to retrieve the actual Tx power that is set. 


rpl-border-router:
Get-RSSI:     a GET resource that shows the detected energy level
Get-Last-RSSI:a GET resource that shows the RSSI of the last received packet. When there is only 1 node, Get-Last-RSSI will
              show the effect of changing the TX power level on that node.

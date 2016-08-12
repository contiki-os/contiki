dr1175-node is an example of a RICH node containing a JN516x microcontroller on a DR1174 baseboard. 
A DR1175 shield is mounted on the baseboard. 
The boards are part of the NXP JN516x Evaluation Kit (see http://www.nxp.com/documents/leaflet/75017368.pdf) 
The dr1175-node connects to the network in the same way as described in `examples\jn5168/rpl/README.md`

The resources on the DR1175 are available as CoAP resources. They can be accessed via a CoAP client at the IPv6 interface 
of the border router (e.g. via Copper plug-in on Firefox).
The following list gives an overview of the resources:

URI                        Description
---                        -----------
DR1175\AllSensors          This is an observable resource that shows the status of the humidity, light and temperature sensor.
                           The resource is automatically updated when a change in the value of the sensor data occurs.
DR1175\ColorLED\RGBValue   With this resource, the level and color of the RGB LED can be set.
                           The output is set as 3 numbers (R, G and B) from 0..255, separated with a space 
DR1175\Humidity\Unit       This resource will return the unit of the humidity sensor
DR1175\Humidity\Value      This resource will return the value of the humidity sensor
DR1175\LightSensor\Unit    This resource will return the unit of the light sensor
DR1175\LightSensor\Value   This resource will return the value of the light sensor
DR1175\Temperature\Unit    This resource will return the unit of the temperature sensor
DR1175\Temperature\Value   This resource will return the value of the temperature sensor
DR1175\WhiteLED            This resource will set the level of 3 white power LEDs. Level is from 0..255
DR1174\D3On1174            This resource control LED D3 on the base board 
DR1174\D6On1174            This resource control LED D6 on the base board 
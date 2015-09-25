dr1199-node is an example of a RICH node containing a JN516x microcontroller on a DR1174 baseboard. 
A DR1199 shield is mounted on the baseboard. 
The boards are part of the NXP JN516x Evaluation Kit (see http://www.nxp.com/documents/leaflet/75017368.pdf) 
The dr1199-node connects to the network in the same way as described in `examples\jn5168/rpl/README.md`

The resources on the DR1199 are available as CoAP resources. They can be accessed via a CoAP client at the IPv6 interface 
of the border router (e.g. via Copper plug-in on Firefox).
The following list gives an overview of the resources:

URI                     Description
---                     -----------
DR1199\AllSensors       This is an observable resource that shows the status of all switches and the potentiometer on the board
                        The resource is automatically updated when a change of the status/value of the switches and potentiometer
                        takes place.
DR1199\LED\All          When writing a '1', LEDs D1..D3 will switch ON
                        When writing a '0', LEDs D1..D3 will switch OFF
DR1199\LED\D1           When writing a '1', LED D1 will switch ON                                                             
                        When writing a '0', LED D1 will switch OFF
DR1199\LED\D2           When writing a '1', LED D2 will switch ON                                                             
                        When writing a '0', LED D2 will switch OFF
DR1199\LED\D3           When writing a '1', LED D3 will switch ON                                                             
                        When writing a '0', LED D3 will switch OFF
DR1199\D3On1174         This resource control LED D3 on the base board 
DR1199\D6On1174         This resource control LED D6 on the base board 
DR1199\Potentiometer    The resource will show the value of the potentiometer
DR1199\Switch\DIO8      This resource shows the status of the DIO8 switch (on DR1174)
DR1199\Switch\SW1       This resource shows the status of the SW1 switch
DR1199\Switch\SW2       This resource shows the status of the SW2 switch
DR1199\Switch\SW3       This resource shows the status of the SW3 switch
DR1199\Switch\SW4       This resource shows the status of the SW4 switch




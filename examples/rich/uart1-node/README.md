This example is a simple node to test uart1 functionality of a JN516x on a RICH node.

The TX output of uart1 is represented as a PUT/POST resource for a Coap client. 
A string entered on a Coap plug-in of a browser will be forwarded over the wireless network to the
the TX output of UART1.  
 
The RX input of uart1 is represented as an OBSERVABLE resource for a Coap client.
If a string is entered on uart1 that is terminated with a Line Feed or Carriage Return, 
a Coap event will notify Coap clients to issue a GET event. 
This will make string available at the Coap client side.



 
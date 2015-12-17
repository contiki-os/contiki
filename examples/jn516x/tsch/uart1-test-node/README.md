This example is a simple TSCH node used to test uart1 functionality of a JN516x on a node.
Note: uart0 is usually configured as serial port for flashing the JN516x and for logging and debug output. 

The TX output of uart1 is represented as a PUT/POST resource for a Coap client. 
A string entered on a Coap plug-in of a browser will be forwarded over the wireless TSCH network to the
the TX output of UART1.  
 
The RX input of uart1 is represented as an OBSERVABLE resource for a Coap client.
If a string is entered on uart1 that is terminated with a Line Feed or Carriage Return, 
a Coap event will notify Coap clients to issue a GET event. 
This will make string available at the Coap client side.

The example has been verified on a DR1174 evaluation board


 
The code is 'xy-test-setup' is used to do position dependent measurements on packet reception rate at UDP level.
A TSCH network is made that consists of 3 components:
- coordinator-client: 
              Creates the PAN and controls the other components in the network for the measurement execution.
              It keeps track of the measurement results and output the results to a serial port.
- sink-server: 
              Is the target for the measurement. Coordinator transmits UDP packets to the sink. The will ACK the packets 
              at MAC level. The coordinator keeps track of the number of retries that are needed to receive the ACK.
              The Sink is mounted on a XY device, so that the measurements are executed on different position in the XY plane.
              This way the impact of XY dependent effects such as fading on the packet reception rate can be measured
- xy-server:
              The XY device is a modified XY plotter (Makeblock XY Plotter V2.0 - www.makeblock.cc). 
              The sink device is mounted on the movable part of the XY plotter.
              The plotter is controlled with GCODE command strings over a serial UART channel. In this test setup this ÜART
              channel is connected to UART1 of a DR1174 board. This makes it possible to let the XY device be part of the 
              TSCH network. The coordinator can send location setpoints over the TSCH channel to the XY device.
All 3 network components ran on the DR1174 board
             
The measurement and XY device configuration settings are stored in common-conf-xy.h                 

The examples are build with the option that TSCH logging is on.
In order to have only experiment output on the serial output of the coordinator, add the following lines 
in the project-conf.h files:
/* Suppress log messages from TSCH */
#undef TSCH_LOG_CONF_LEVEL
#define TSCH_LOG_CONF_LEVEL 0


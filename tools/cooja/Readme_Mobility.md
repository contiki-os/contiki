<!---Author: Manish Kausik H-->

# Cooja Mobility
	This version of cooja has the option of moving motes via a contiki process. 
	
## How to use this verion of Cooja?
	1. To compile and launch cooja, the steps are the same as the older cooja versions:
		ant jar // to compile only
		ant run // to compile and launch cooja
		
	2. To setup mote tracking :
		a. Go to File ---> New Simulation
		b. In the New Simulation Dialog box, enable "Track Positions of Node" checkbox
		c. Fill in all other details as per your needs
		d. Press "Create".
	
	Now you can operate Cooja just as before!
	
## What is expected from the contiki code?
	The contiki code that you upload to the virtual motes in cooja MUST define 2 Global variables called "pos_x" and "pos_y".
	They must be declared like this:
		static float pos_x=<value>;
		static float pos_y=<value>;
		
	These are the variables that you MUST update, so that cooja can update positions in the GUI.
	Note: If these variables are not declared as Global Variables or are not declared at all, a NULL_POINTER_EXCEPTION will be thrown by Cooja.
	These Variables are relevant and compulsory only if the Track Position checkbox was enabled when creating the simulation.
	
## Some other important details
	Cooja updates the positions of the motes every 100ms. Any changes made to pos_x and pos_y before 100ms time will not be reflected in Cooja.
	
## Examples
	the example folder in contiki parent folder contains an example of a contiki process that makes the mote moving in a circle. The folder is named "circular-motion"
	
## Potential Application
	The dynamic mobility feature has been introduced so that motes can make decisions on how to move on the run, while communicating with other motes.
	This gives a platform for research on various path planning methods in tight sync with various communication stacks already available in contiki. 

	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	


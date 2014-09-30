Advanced Sky GUI Cooja Plugin
=============================
&nbsp;
-----------------------------
About
-----------------------------
&nbsp;

The Advanced Sky GUI Plug-in for Cooja Simulator is an interactive graphic interface, which adds to the standard sky mote interface, the capability to stimulate the ADC ports, and to move the mote in a simulated environment. It allows the following operations:
* Simulate the PAR and TRS sensors
* Apply the simulated analog signals on 8 Sky ADC ports
* Move the mote around
* Use the serial port, reset button, user button and LEDs
>

The Advanced Sky GUI Plug-in for Cooja Simulator is composed by the following parts:
* Analog sliders for 8 analog ports
* Joystick to move mote around
* Reset button
* User button
* Three working LEDs
* Serial Port

-----------------------------
Installation
-----------------------------
&nbsp;

The following steps outline how to install the Advanced Sky GUI Plug-in:
1. Download the Advanced Sky GUI Plug-in code
2. After Downloading it, you have to place it on the following path (extract the folder in case it is compressed):

```sh
YOUR_CONTIKI_FOLDER/tools/cooja/apps
```
3. Modify the build.xml file in the Cooja folder:

```sh
YOUR_CONTIKI_FOLDER/tools/cooja
```

and add:

```sh
<ant antfile="build.xml" dir="apps/sky_gui" target="jar" inheritAll="false"/>
```

to the following code:

```sh
<target name="jar" depends="jar_cooja">
<ant antfile="build.xml" dir="apps/mrm" target="jar" inheritAll="false"/>
<ant antfile="build.xml" dir="apps/mspsim" target="jar" inheritAll="false"/>
<ant antfile="build.xml" dir="apps/avrora" target="jar" inheritAll="false"/>
<ant antfile="build.xml" dir="apps/serial_socket" target="jar" inheritAll="false"/>
<ant antfile="build.xml" dir="apps/collect-view" target="jar" inheritAll="false"/>
<ant antfile="build.xml" dir="apps/powertracker" target="jar" inheritAll="false"/>
<ant antfile="build.xml" dir="apps/apps/sky_gui" target="jar" inheritAll="false"/>
</target>
```
> 

and also add this line:

```sh
<ant antfile="build.xml" dir="apps/apps/sky_gui" target="clean" inheritAll="false"/>
```

to the following code:

```sh
<target name="clean" depends="init">
<delete dir="${build}"/>
<delete dir="${dist}"/>
<ant antfile="build.xml" dir="apps/mrm" target="clean" inheritAll="false"/>
<ant antfile="build.xml" dir="apps/mspsim" target="clean" inheritAll="false"/>
<ant antfile="build.xml" dir="apps/avrora" target="clean" inheritAll="false"/>
<ant antfile="build.xml" dir="apps/serial_socket" target="clean" inheritAll="false"/>
<ant antfile="build.xml" dir="apps/collect-view" target="clean" inheritAll="false"/>
<ant antfile="build.xml" dir="apps/powertracker" target="clean" inheritAll="false"/>
<ant antfile="build.xml" dir="apps/apps/sky_gui" target="clean" inheritAll="false"/>
</target>
```

-----------------------------
How to use
-----------------------------
&nbsp;

### Activating the Advanced Sky GUI Plug-in
&nbsp;

In order to interact directly with motes in COOJA, you need to use the Advanced Sky GUI plug-in which you have previously downloaded from here and added to your COOJA folder (this has been explained in the installation procedure). In order to use this plug-in, when running COOJA for the first time, you need to select and activate the plug-in in the COOJA extension list which you can select in "Setting -> COOJA extensions".
> 

### Executing the Advanced Sky GUI Plug-in
&nbsp;

For each wireless node that interfaces with the Advanced Sky GUI, you have to right click on that mote and to select "Mote tools Sky -> Sky Graphic User Interface". This plugin allows the communication between the mote and the advanced graphic user interface.
> 

### Testing the Advanced Sky GUI Plug-in
&nbsp;

1. Run Collect-View example with two or more Sky nodes
2. Open the Collect-view GUI, choose a node in the list "Nodes" and click on the "Sensors -> Light 1" tab
2. In Cooja Simlator "Network view" right click on the chosen mote you want to interact with, and select "Mote tools Sky -> Sky Graphic User Interface"
4. Check the "PAR ADC 4" checkbox, move the slider and see, in the "Ligth 1" graph, the value of the light to change
5. Move the Joystick and see what happens when the node goes out of the radio transmission range
6. Send "help" data in the serial port and choose the command you want to execute


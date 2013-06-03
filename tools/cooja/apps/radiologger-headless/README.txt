To install this plugin in COOJA:

1) (optional, recommended) Copy the 'cooja-radiologger-headless' folder to your CONTIKI_DIR/tools/cooja/apps folder.
2) (optional, recommended) Rename it to 'radiologger-headless'
3) Build the plugin
  - goto the plugin folder in a console
  - type 'ant jar'
4) Add that folder to the DEFAULT_PROJECTDIRS variable, in one of the following ways:
  a) Via the GUI.
    - Open COOJA in graphical mode
    - Go to Settings > Cooja Extensions
    - In the file browser in the left panel, browse to the CONTIKI_DIR/tools/cooja/apps folder
    - Check the box next to radiologger-headless (it should appear green)
    - 'Save' the window, click OK to permanently apply the change
  b) By hand:
    - Open ~/.cooja.user.properties
    - Append ';[APPS_DIR]/radiologger-headless' at the end of the DEFAULT_PROJECTDIRS line
    - Note: this file is created the very first time you use COOJA in GUI mode, and is user-specific.
5) To enable this in your simulation, do it in one of 2 ways:
  a) Via the GUI:
    - Open your simulation in COOJA
    - Select Tools > Headless radio logger...
    - You will see an empty window titled Radio logger headless. Do not close it.
    - Save your simulation and close COOJA.
  b) By hand
    - Added the following snippet to the obvious place in the .csc file:

  <plugin>
    be.cetic.cooja.plugins.RadioLoggerHeadless
    <width>150</width>
    <z>0</z>
    <height>300</height>
    <location_x>1</location_x>
    <location_y>403</location_y>
  </plugin> 

Feel free to report bugs or errors in this how-to to: 6lbr@cetic.be

Enjoy.

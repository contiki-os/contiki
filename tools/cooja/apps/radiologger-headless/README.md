This is a standalone plugin for COOJA to log the radio packets
in headless mode. It is a stripped-down version of COOJA's 
graphical Radio Logger. It allows to generate PCAPs while 
executing COOJA from the command line with the -nogui option.

To install this plugin in COOJA:

1. (optional, recommended) Copy the 'cooja-radiologger-headless' folder to your COOJA apps folder.
2. (optional, recommended) Rename it to 'radiologger-headless'
3. Build the plugin:
    * Open a console and Navigate into the radiologger-headless folder
    * type `ant jar`
    * You may need to adjust the cooja property in the build.xml file for this step
4. Add that folder to the DEFAULT_PROJECTDIRS variable, in one of the following ways:
    * Via the GUI.
        * Open COOJA in graphical mode
        * Go to Tools > Cooja Extensions
        * In the file browser in the left panel, browse to the cooja/apps folder
        * Check the box next to radiologger-headless (it should appear green)
        * 'Save' the window, click OK to permanently apply the change
    * By hand:
        * Open ~/.cooja.user.properties
        * Append ';[APPS_DIR]/radiologger-headless' at the end of the DEFAULT_PROJECTDIRS line
        * Note: this file is created the very first time you use COOJA in GUI mode, and is user-specific.
5. To enable this in your simulation, do it in one of 2 ways:
    * Via the GUI:
        * Open your simulation in COOJA
        * Select Tools > Headless radio logger...
        * You will see an empty window titled Radio logger headless. Do not close it.
        * Save your simulation and close COOJA.
    * By hand
        * Add [**the following snippet**](https://github.com/cetic/cooja-radiologger-headless/blob/master/CSC_PLUGIN.txt) to the obvious place in the .csc file.

Feel free to report suggestions and bugs to: 6lbr@cetic.be

Enjoy.

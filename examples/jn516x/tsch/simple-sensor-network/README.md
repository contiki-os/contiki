This example shows a simple sensor network consisting of node(s),an rpl-border-router and a host connected to
the IPv6 network. The nodes (see also node/README.md) regularly transmit simulated sensor data to the host.
The host visualises the received data. 

The Python script tools/Output-Visualisation.py visualises the received data from the nodes.
The script runs on Python27 and uses gnuplot for visualisation. 
The following installations are needed (tested with Win32 versions:
- Python 27 (https://www.python.org/download/releases/2.7/)
- gnuplot (http://sourceforge.net/projects/gnuplot/files/gnuplot/)
- gnuplot.py (http://gnuplot-py.sourceforge.net/)
- numpy.py (e.g. http://www.lfd.uci.edu/~gohlke/pythonlibs/#numpy)
Manual modification:
Python27\Lib\site-packages\Gnuplot\gp_win32.py
Modify gnuplot_command as in example below
    gnuplot_command = r'"C:\Program Files (x86)\gnuplot\bin\gnuplot.exe"'

- The IPv6 addresses of nodes need to be entered in the dictionary "node_data" of tools/Output-Visualisation.py
  A user name for the node can be entered in this table as well.
- The script will send a ping message to the next node in the list every 5 seconds. The nodes will obtain
  the IPv6 address from this message and start sending "sensor" data to the host.
- Received sensor data is plotted. When no data has been received for more than 30seconds, the plot line
  will be dashed. 
- Plots are updated once per second.
 
  
/*
 * Copyright (c) 2006, Swedish Institute of Computer Science.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * $Id: UAODVControl.java,v 1.2 2007/01/09 10:09:59 fros4943 Exp $
 */

import java.awt.*;
import java.awt.event.*;
import java.util.*;
import javax.swing.*;
import org.apache.log4j.Logger;

import se.sics.cooja.*;
import se.sics.cooja.contikimote.interfaces.ContikiRS232;
import se.sics.cooja.interfaces.*;

/**
 * @author Fredrik Osterlind
 */
@ClassDescription("uAODV Control")
@PluginType(PluginType.SIM_PLUGIN)
public class UAODVControl extends VisPlugin {
	private static final long serialVersionUID = 1L;
	private static Logger logger = Logger.getLogger(UAODVControl.class);
	private Simulation mySimulation;
	private JComboBox sourceComboBox;
	private JComboBox destComboBox;

	/**
	 * @param simulationToVisualize Current simulation
	 */
	public UAODVControl(Simulation simulationToVisualize, GUI gui) {
    super("uAODV Control (uses RS232)", gui);
    mySimulation = simulationToVisualize;

    Container mainPane = this.getContentPane();
    mainPane.setLayout(new BoxLayout(mainPane, BoxLayout.Y_AXIS));
    JLabel label;
    JPanel smallPane;
    
    // Create available nodes list
    Vector<String> nodeDescs = new Vector<String>();
    for (int i=0; i < mySimulation.getMotesCount(); i++) {
      Mote currentMote = mySimulation.getMote(i);
      nodeDescs.add("ID=" 
          + currentMote.getInterfaces().getMoteID().getMoteID()
          + ", IP=" 
          + currentMote.getInterfaces().getIPAddress().getIPString());
    }
    
    // Create source combo box
    label = new JLabel("Select RREQ source");

    sourceComboBox = new JComboBox(nodeDescs);
    if (sourceComboBox.getItemCount() < 1) {
      logger.warn("No nodes available");
    } else
      sourceComboBox.setSelectedIndex(0);
    label.setLabelFor(sourceComboBox);

    smallPane = new JPanel();
    smallPane.add(label);
    smallPane.add(Box.createHorizontalStrut(10));
    smallPane.add(sourceComboBox);
    
    mainPane.add(smallPane);

    // Create destination combo box
    label = new JLabel("Select RREQ destination");

    destComboBox = new JComboBox(nodeDescs);
    if (destComboBox.getItemCount() < 1) {
      logger.warn("No nodes available");
    } else
      destComboBox.setSelectedIndex(0);
    label.setLabelFor(destComboBox);

    smallPane = new JPanel();
    smallPane.add(label);
    smallPane.add(Box.createHorizontalStrut(10));
    smallPane.add(destComboBox);
    
    mainPane.add(smallPane);
    
    // Add set button
    smallPane = new JPanel(new BorderLayout());
    JButton setDestinationButton = new JButton("Set IP and send RREQ");
    setDestinationButton.addActionListener(new ActionListener() {
      public void actionPerformed(ActionEvent e) {
        logger.debug("Sending RS232 command now");
        Mote sourceMote = mySimulation.getMote(sourceComboBox.getSelectedIndex());
        Mote destMote = mySimulation.getMote(destComboBox.getSelectedIndex());
        if (sourceMote == null || destMote == null) {
          logger.error("Error in mote selection");
          return;
        }

        // Get destination IP
        IPAddress destIP = destMote.getInterfaces().getIPAddress();
        if (destIP == null) {
          logger.error("Error when fetching destination IP");
          return;
        }
        
        // Set destination and start sending by using RS232
        ContikiRS232 rs232 = sourceMote.getInterfaces().getInterfaceOfType(ContikiRS232.class);
        if (rs232 == null) {
          logger.error("RS232 interface is null!");
          return;
        }

        rs232.sendSerialMessage("SENDTO>" + destIP.getIPString());
      }
    });
    smallPane.add(BorderLayout.EAST, setDestinationButton);
    mainPane.add(smallPane);
    
    
    pack();
    
    // Tries to select this plugin
    try {
      setSelected(true);
    } catch (java.beans.PropertyVetoException e) {
      // Could not select
    }
    
  }

  public void closePlugin() {
  }

}

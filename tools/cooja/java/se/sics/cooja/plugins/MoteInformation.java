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
 * $Id: MoteInformation.java,v 1.6 2008/10/28 13:59:35 fros4943 Exp $
 */

package se.sics.cooja.plugins;

import java.awt.*;
import java.awt.event.*;
import java.util.*;
import javax.swing.*;
import org.apache.log4j.Logger;

import se.sics.cooja.*;
import se.sics.cooja.Mote.State;

/**
 * Mote information displays information about a given mote.
 *
 * @author Fredrik Osterlind
 */
@ClassDescription("Mote Information")
@PluginType(PluginType.MOTE_PLUGIN)
public class MoteInformation extends VisPlugin {
  private static Logger logger = Logger.getLogger(MoteInformation.class);

  private static final long serialVersionUID = 1L;

  private Mote mote;

  private final static int LABEL_WIDTH = 170;
  private final static int LABEL_HEIGHT = 15;

  private final JLabel stateLabel;

  private Observer stateObserver;
  private Vector<JPanel> visibleMoteInterfaces = new Vector<JPanel>();

  private Simulation mySimulation;

  /**
   * Create a new mote information window.
   *
   * @param moteToView Mote to view
   */
  public MoteInformation(Mote moteToView, Simulation simulation, GUI gui) {
    super("Mote Information (" + moteToView + ")", gui);

    mote = moteToView;
    mySimulation = simulation;

    JLabel label;
    JPanel mainPane = new JPanel();
    mainPane.setBorder(BorderFactory.createEmptyBorder(10, 10, 10, 10));
    mainPane.setLayout(new BoxLayout(mainPane, BoxLayout.Y_AXIS));
    JPanel smallPane;

    /* Remove button */
    smallPane = new JPanel(new BorderLayout());
    label = new JLabel("Remove mote");
    label.setPreferredSize(new Dimension(LABEL_WIDTH,LABEL_HEIGHT));
    smallPane.add(BorderLayout.WEST, label);

    JButton button = new JButton("Remove");
    button.setActionCommand("removeMote");
    button.addActionListener(new ActionListener() {
      public void actionPerformed(ActionEvent e) {
        mySimulation.removeMote(mote);
        dispose();
      }
    });

    smallPane.add(BorderLayout.EAST, button);
    mainPane.add(smallPane);
    mainPane.add(Box.createRigidArea(new Dimension(0,25)));

    /* State */
    smallPane = new JPanel(new BorderLayout());
    label = new JLabel("-- STATE --");
    label.setPreferredSize(new Dimension(LABEL_WIDTH,LABEL_HEIGHT));
    smallPane.add(BorderLayout.WEST, label);
    if (mote.getState() == Mote.State.ACTIVE) {
      label = new JLabel("active");
    } else if (mote.getState() == State.LPM) {
      label = new JLabel("low power mode");
    } else {
      label = new JLabel("dead");
    }

    label.setPreferredSize(new Dimension(LABEL_WIDTH,LABEL_HEIGHT));
    stateLabel = label;

    smallPane.add(BorderLayout.EAST, label);

    mainPane.add(smallPane);
    mainPane.add(Box.createRigidArea(new Dimension(0,25)));

    /* Mote type */
    smallPane = new JPanel(new BorderLayout());
    label = new JLabel("-- MOTE TYPE --");
    label.setPreferredSize(new Dimension(LABEL_WIDTH,LABEL_HEIGHT));
    smallPane.add(BorderLayout.WEST, label);
    label = new JLabel(moteToView.getType().getIdentifier() + ": \"" + moteToView.getType().getDescription() + "\"");
    label.setPreferredSize(new Dimension(LABEL_WIDTH,LABEL_HEIGHT));
    smallPane.add(BorderLayout.EAST, label);

    mainPane.add(smallPane);
    mainPane.add(Box.createRigidArea(new Dimension(0,25)));

    /* Mote interfaces */
    smallPane = new JPanel(new BorderLayout());
    label = new JLabel("-- MOTE INTERFACES --");
    label.setPreferredSize(new Dimension(LABEL_WIDTH,LABEL_HEIGHT));
    smallPane.add(BorderLayout.NORTH, label);

    mainPane.add(smallPane);
    mainPane.add(Box.createRigidArea(new Dimension(0,10)));

    for (MoteInterface intf : mote.getInterfaces().getInterfaces()) {
      smallPane = new JPanel();
      smallPane.setLayout(new BorderLayout());

      String interfaceDescription = GUI.getDescriptionOf(intf);
      JPanel interfaceVisualizer = intf.getInterfaceVisualizer();
      label = new JLabel(interfaceDescription);
      label.setAlignmentX(JLabel.CENTER_ALIGNMENT);
      smallPane.add(BorderLayout.NORTH, label);

      if (interfaceVisualizer != null) {
        interfaceVisualizer.setBorder(BorderFactory.createEtchedBorder());
        smallPane.add(BorderLayout.CENTER, interfaceVisualizer);

				// Tag each visualized interface
				interfaceVisualizer.putClientProperty("my_interface", intf);
        visibleMoteInterfaces.add(interfaceVisualizer);
      }

      mainPane.add(smallPane);
      mainPane.add(Box.createRigidArea(new Dimension(0,5)));
    }

    this.getContentPane().add(BorderLayout.CENTER,
        new JScrollPane(mainPane,
            JScrollPane.VERTICAL_SCROLLBAR_AS_NEEDED,
            JScrollPane.HORIZONTAL_SCROLLBAR_AS_NEEDED));

    pack();
    setPreferredSize(new Dimension(getWidth()+15, 250));
    setSize(new Dimension(getWidth()+15, 250));

    try {
      setSelected(true);
    } catch (java.beans.PropertyVetoException e) {
      // Could not select
    }

    /* Listen to mote state changes */
    mote.addStateObserver(stateObserver = new Observer() {
      public void update(Observable obs, Object obj) {
        if (mote.getState() == State.ACTIVE) {
          stateLabel.setText("active");
        } else if (mote.getState() == Mote.State.LPM) {
          stateLabel.setText("low power mode");
        } else {
          stateLabel.setText("dead");
        }
      }
    });
  }

  public void closePlugin() {
	  // Remove state observer
    mote.deleteStateObserver(stateObserver);

    // Release all interface visualizations
    for (JPanel interfaceVisualization: visibleMoteInterfaces) {
      MoteInterface moteInterface = (MoteInterface) interfaceVisualization.getClientProperty("my_interface");
      if (moteInterface != null) {
        moteInterface.releaseInterfaceVisualizer(interfaceVisualization);
      } else {
        logger.warn("Could not release panel");
      }
    }
  }

}

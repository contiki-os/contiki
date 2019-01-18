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
 * $Id: MoteTypeInformation.java,v 1.2 2007/01/09 09:49:24 fros4943 Exp $
 */

package se.sics.cooja.plugins;

import java.awt.*;
import javax.swing.*;
import org.apache.log4j.Logger;

import se.sics.cooja.*;

/**
 * Shows a summary of all created mote types.
 *
 * @author Fredrik Osterlind
 */
@ClassDescription("Mote Type Information")
@PluginType(PluginType.SIM_PLUGIN)
public class MoteTypeInformation extends VisPlugin {
  private static Logger logger = Logger.getLogger(MoteTypeInformation.class);

  private static final long serialVersionUID = 1L;

  private Simulation mySimulation;

  private final static int LABEL_WIDTH = 170;
  private final static int LABEL_HEIGHT = 15;

  /**
   * Create a new mote type information window.
   *
   * @param simulation Simulation
   */
  public MoteTypeInformation(Simulation simulation, GUI gui) {
    super("Mote Type Information *frozen*", gui);

    mySimulation = simulation;

    JLabel label;
    JPanel mainPane = new JPanel();
    mainPane.setBorder(BorderFactory.createEmptyBorder(10, 10, 10, 10));
    mainPane.setLayout(new BoxLayout(mainPane, BoxLayout.Y_AXIS));
    JPanel smallPane;

    // Visualize mote types
    smallPane = new JPanel(new BorderLayout());
    label = new JLabel("-- MOTE TYPES AT TIME " + simulation.getSimulationTime() + " --");
    label.setPreferredSize(new Dimension(LABEL_WIDTH,LABEL_HEIGHT));
    smallPane.add(BorderLayout.NORTH, label);

    mainPane.add(smallPane);
    mainPane.add(Box.createRigidArea(new Dimension(0,10)));
    
    for (MoteType moteType: mySimulation.getMoteTypes()) {
      smallPane = new JPanel();
      smallPane.setLayout(new BorderLayout());

      label = new JLabel(GUI.getDescriptionOf(moteType) +": " +
          "ID=" + moteType.getIdentifier() + 
          ", \"" + moteType.getDescription() + "\"");
      label.setAlignmentX(JLabel.CENTER_ALIGNMENT);
      smallPane.add(BorderLayout.NORTH, label);

      JPanel moteTypeVisualizer = moteType.getTypeVisualizer();
      if (moteTypeVisualizer != null) {
        moteTypeVisualizer.setBorder(BorderFactory.createEtchedBorder());
        smallPane.add(BorderLayout.CENTER, moteTypeVisualizer);
      } else
        smallPane.add(BorderLayout.CENTER, Box.createVerticalStrut(25));
        
      
      mainPane.add(smallPane);
      mainPane.add(Box.createRigidArea(new Dimension(0,25)));
    }
    
    
    this.setContentPane(new JScrollPane(mainPane,
        JScrollPane.VERTICAL_SCROLLBAR_AS_NEEDED,
        JScrollPane.HORIZONTAL_SCROLLBAR_AS_NEEDED));
    pack();
    setPreferredSize(new Dimension(350,500));
    setSize(new Dimension(350,500));
    
    try {
      setSelected(true);
    } catch (java.beans.PropertyVetoException e) {
      // Could not select
    }
    
  }
  
  public void closePlugin() {
  }

}

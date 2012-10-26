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
 */

package se.sics.cooja.plugins;

import java.awt.BorderLayout;
import java.awt.Dimension;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;

import javax.swing.BorderFactory;
import javax.swing.BoxLayout;
import javax.swing.JButton;
import javax.swing.JLabel;
import javax.swing.JPanel;
import javax.swing.JScrollPane;

import org.apache.log4j.Logger;

import se.sics.cooja.ClassDescription;
import se.sics.cooja.GUI;
import se.sics.cooja.Mote;
import se.sics.cooja.MotePlugin;
import se.sics.cooja.PluginType;
import se.sics.cooja.Simulation;
import se.sics.cooja.VisPlugin;
import se.sics.cooja.motes.AbstractEmulatedMote;

/**
 * Mote information displays information about a given mote.
 *
 * @author Fredrik Osterlind
 */
@ClassDescription("Mote Information")
@PluginType(PluginType.MOTE_PLUGIN)
public class MoteInformation extends VisPlugin implements MotePlugin {
  private static final long serialVersionUID = 2359676837283723500L;
  private static Logger logger = Logger.getLogger(MoteInformation.class);

  private Mote mote;

  private final static int LABEL_WIDTH = 170;
  private final static int LABEL_HEIGHT = 20;
  private final static Dimension size = new Dimension(LABEL_WIDTH,LABEL_HEIGHT);
  
  private Simulation simulation;

  /**
   * Create a new mote information window.
   *
   * @param m Mote
   * @param s Simulation
   * @param gui Simulator
   */
  public MoteInformation(Mote m, Simulation s, GUI gui) {
    super("Mote Information (" + m + ")", gui);
    this.mote = m;
    this.simulation = s;

    JLabel label;
    JButton button;
    JPanel smallPane;

    JPanel mainPane = new JPanel();
    mainPane.setBorder(BorderFactory.createEmptyBorder(5, 5, 5, 5));
    mainPane.setLayout(new BoxLayout(mainPane, BoxLayout.Y_AXIS));

    /* Mote type */
    smallPane = new JPanel(new BorderLayout());
    label = new JLabel("Mote type");
    label.setPreferredSize(size);
    smallPane.add(BorderLayout.WEST, label);
    label = new JLabel(mote.getType().getDescription());
    label.setPreferredSize(size);
    smallPane.add(BorderLayout.EAST, label);
    mainPane.add(smallPane);

    smallPane = new JPanel(new BorderLayout());
    label = new JLabel(mote.getType().getIdentifier());
    label.setPreferredSize(size);
    smallPane.add(BorderLayout.EAST, label);
    mainPane.add(smallPane);

    smallPane = new JPanel(new BorderLayout());
    button = new JButton("Mote type information");
    button.setPreferredSize(size);
    button.addActionListener(new ActionListener() {
      public void actionPerformed(ActionEvent e) {
        simulation.getGUI().tryStartPlugin(MoteTypeInformation.class, simulation.getGUI(), simulation, mote);
      }
    });
    smallPane.add(BorderLayout.EAST, button);
    mainPane.add(smallPane);

    /* Mote interfaces */
    smallPane = new JPanel(new BorderLayout());
    label = new JLabel("Mote interfaces");
    label.setPreferredSize(size);
    smallPane.add(BorderLayout.WEST, label);
    label = new JLabel(mote.getInterfaces().getInterfaces().size() + " interfaces");
    label.setPreferredSize(size);
    smallPane.add(BorderLayout.EAST, label);
    mainPane.add(smallPane);

    smallPane = new JPanel(new BorderLayout());
    button = new JButton("Mote interface viewer");
    button.setPreferredSize(size);
    button.addActionListener(new ActionListener() {
      public void actionPerformed(ActionEvent e) {
        simulation.getGUI().tryStartPlugin(MoteInterfaceViewer.class, simulation.getGUI(), simulation, mote);
      }
    });
    smallPane.add(BorderLayout.EAST, button);
    mainPane.add(smallPane);
    
    /* CPU frequency */
    if (mote instanceof AbstractEmulatedMote) {
      AbstractEmulatedMote emulatedMote = (AbstractEmulatedMote) mote;
      smallPane = new JPanel(new BorderLayout());
      label = new JLabel("CPU frequency");
      label.setPreferredSize(size);
      smallPane.add(BorderLayout.WEST, label);
      if (emulatedMote.getCPUFrequency() < 0) {
        label = new JLabel("[unknown]");
      } else {
        label = new JLabel(emulatedMote.getCPUFrequency() + " Hz");
      }
      label.setPreferredSize(size);
      smallPane.add(BorderLayout.EAST, label);
      mainPane.add(smallPane);
    }
    
    /* Remove button */
    smallPane = new JPanel(new BorderLayout());
    label = new JLabel("Remove mote");
    label.setPreferredSize(size);
    smallPane.add(BorderLayout.WEST, label);

    button = new JButton("Remove");
    button.setPreferredSize(size);
    button.addActionListener(new ActionListener() {
      public void actionPerformed(ActionEvent e) {
        /* TODO In simulation event (if running) */
        simulation.removeMote(MoteInformation.this.mote);
      }
    });
    smallPane.add(BorderLayout.EAST, button);
    mainPane.add(smallPane);

    this.getContentPane().add(BorderLayout.CENTER,
        new JScrollPane(mainPane,
            JScrollPane.VERTICAL_SCROLLBAR_AS_NEEDED,
            JScrollPane.HORIZONTAL_SCROLLBAR_AS_NEEDED));

    pack();
    setSize(new Dimension(getWidth()+15, getHeight()+15));
  }

  public void closePlugin() {
  }

  public Mote getMote() {
    return mote;
  }

}

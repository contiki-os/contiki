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

import java.awt.*;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.util.*;
import javax.swing.*;
import javax.swing.Timer;

import se.sics.cooja.*;

/**
 * SimInformation is a simple information window for simulations.
 *
 * @author Fredrik Osterlind
 */
@ClassDescription("Simulation Information")
@PluginType(PluginType.SIM_PLUGIN)
public class SimInformation extends VisPlugin {
  private Simulation simulation;

  private static final int LABEL_UPDATE_INTERVAL = 100;

  private final static int LABEL_WIDTH = 170;
  private final static int LABEL_HEIGHT = 15;

  private JLabel labelStatus;
  private JLabel labelSimTime;
  private JLabel labelNrMotes;
  private JLabel labelNrMoteTypes;

  private Observer simObserver;

  /**
   * Create a new simulation information window.
   *
   * @param simulationToView Simulation to view
   */
  public SimInformation(Simulation simulationToView, GUI gui) {
    super("Simulation Information", gui);

    simulation = simulationToView;

    JLabel label;
    JPanel mainPane = new JPanel();
    mainPane.setBorder(BorderFactory.createEmptyBorder(10, 10, 10, 10));
    mainPane.setLayout(new BoxLayout(mainPane, BoxLayout.Y_AXIS));
    JPanel smallPane;

    // Status information
    smallPane = new JPanel();
    smallPane.setAlignmentX(Component.LEFT_ALIGNMENT);
    smallPane.setLayout(new BoxLayout(smallPane, BoxLayout.X_AXIS));
    label = new JLabel("Status");
    label.setPreferredSize(new Dimension(LABEL_WIDTH,LABEL_HEIGHT));
    smallPane.add(label);

    smallPane.add(Box.createHorizontalStrut(10));
    smallPane.add(Box.createHorizontalGlue());

    label = new JLabel();
    if (simulation.isRunning()) {
      label.setText("RUNNING");
    } else {
      label.setText("STOPPED");
    }

    labelStatus = label;
    smallPane.add(label);

    mainPane.add(smallPane);

    mainPane.add(Box.createRigidArea(new Dimension(0,5)));

    // Current simulation time
    smallPane = new JPanel();
    smallPane.setAlignmentX(Component.LEFT_ALIGNMENT);
    smallPane.setLayout(new BoxLayout(smallPane, BoxLayout.X_AXIS));
    label = new JLabel("Simulation time");
    label.setPreferredSize(new Dimension(LABEL_WIDTH,LABEL_HEIGHT));
    smallPane.add(label);

    smallPane.add(Box.createHorizontalStrut(10));
    smallPane.add(Box.createHorizontalGlue());

    label = new JLabel();
    label.setText(""  + simulation.getSimulationTimeMillis());

    labelSimTime = label;
    smallPane.add(label);

    mainPane.add(smallPane);

    mainPane.add(Box.createRigidArea(new Dimension(0,5)));

    // Number of motes
    smallPane = new JPanel();
    smallPane.setAlignmentX(Component.LEFT_ALIGNMENT);
    smallPane.setLayout(new BoxLayout(smallPane, BoxLayout.X_AXIS));
    label = new JLabel("Number of motes");
    label.setPreferredSize(new Dimension(LABEL_WIDTH,LABEL_HEIGHT));
    smallPane.add(label);

    smallPane.add(Box.createHorizontalStrut(10));
    smallPane.add(Box.createHorizontalGlue());

    label = new JLabel();
    label.setText(""  + simulation.getMotesCount());

    labelNrMotes = label;
    smallPane.add(label);

    mainPane.add(smallPane);

    mainPane.add(Box.createRigidArea(new Dimension(0,5)));


    // Number of mote types
    smallPane = new JPanel();
    smallPane.setAlignmentX(Component.LEFT_ALIGNMENT);
    smallPane.setLayout(new BoxLayout(smallPane, BoxLayout.X_AXIS));
    label = new JLabel("Number of mote types");
    label.setPreferredSize(new Dimension(LABEL_WIDTH,LABEL_HEIGHT));
    smallPane.add(label);

    smallPane.add(Box.createHorizontalStrut(10));
    smallPane.add(Box.createHorizontalGlue());

    label = new JLabel();
    label.setText(""  + simulation.getMoteTypes().length);

    labelNrMoteTypes = label;
    smallPane.add(label);

    mainPane.add(smallPane);

    mainPane.add(Box.createRigidArea(new Dimension(0,5)));



    // Radio Medium type
    smallPane = new JPanel();
    smallPane.setAlignmentX(Component.LEFT_ALIGNMENT);
    smallPane.setLayout(new BoxLayout(smallPane, BoxLayout.X_AXIS));
    label = new JLabel("Radio medium");
    label.setPreferredSize(new Dimension(LABEL_WIDTH,LABEL_HEIGHT));
    smallPane.add(label);

    smallPane.add(Box.createHorizontalStrut(10));
    smallPane.add(Box.createHorizontalGlue());

    Class<? extends RadioMedium> radioMediumClass = simulation.getRadioMedium().getClass();
    String description = GUI.getDescriptionOf(radioMediumClass);
    label = new JLabel(description);

    smallPane.add(label);

    mainPane.add(smallPane);

    mainPane.add(Box.createRigidArea(new Dimension(0,5)));


    this.setContentPane(mainPane);
    pack();

    // Register as simulation observer
    simulation.addObserver(simObserver = new Observer() {
      public void update(Observable obs, Object obj) {
        if (simulation == null) {
          return;
        }
        if (simulation.isRunning()) {
          labelStatus.setText("RUNNING");
          updateLabelTimer.start();
        } else {
          labelStatus.setText("STOPPED");
        }
        labelNrMotes.setText(""  + simulation.getMotesCount());
        labelNrMoteTypes.setText(""  + simulation.getMoteTypes().length);
      }
    });

    /* Update current time label when simulation is running */
    if (simulation.isRunning()) {
      updateLabelTimer.start(); 
    }
    
    try {
      setSelected(true);
    } catch (java.beans.PropertyVetoException e) {
      // Could not select
    }

  }

  public void closePlugin() {
    // Remove log observer from all log interfaces
    if (simObserver != null) {
      simulation.deleteObserver(simObserver);
    }

    /* Remove label update timer */
    updateLabelTimer.stop();
  }

  private Timer updateLabelTimer = new Timer(LABEL_UPDATE_INTERVAL, new ActionListener() {
    public void actionPerformed(ActionEvent e) {
      labelSimTime.setText("" + simulation.getSimulationTimeMillis());

      /* Automatically stop if simulation is no longer running */
      if (!simulation.isRunning()) {
        updateLabelTimer.stop();
      }
    }
  });

}

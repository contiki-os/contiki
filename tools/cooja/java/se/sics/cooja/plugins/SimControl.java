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
 * $Id: SimControl.java,v 1.3 2007/01/09 09:49:24 fros4943 Exp $
 */

package se.sics.cooja.plugins;

import java.awt.*;
import java.awt.event.*;
import java.beans.PropertyChangeEvent;
import java.beans.PropertyChangeListener;
import java.text.NumberFormat;
import java.util.*;
import javax.swing.*;
import javax.swing.event.*;
import org.apache.log4j.Logger;

import se.sics.cooja.*;

/**
 * The Control Panel is a simple control panel for simulations.
 *
 * @author Fredrik Osterlind
 */
@ClassDescription("Control Panel")
@PluginType(PluginType.SIM_STANDARD_PLUGIN)
public class SimControl extends VisPlugin {
  private static final long serialVersionUID = 1L;
  private static Logger logger = Logger.getLogger(SimControl.class);

  private static final int MIN_DELAY_TIME = 0;
  private static final int MAX_DELAY_TIME = 100;

  private Simulation simulation;

  private JSlider sliderDelay;
  private JLabel simulationTime;
  private JButton startButton, stopButton;
  private JFormattedTextField stopTimeTextField;
  private int simulationStopTime = -1;

  private Observer simObserver;
  private Observer tickObserver;
  
  private long lastTextUpdateTime = -1;
  
  /**
   * Create a new simulation control panel.
   *
   * @param simulationToControl Simulation to control
   */
  public SimControl(Simulation simulationToControl, GUI gui) {
    super("Control Panel - " + simulationToControl.getTitle(), gui);

    simulation = simulationToControl;

    JButton button;
    JPanel smallPanel;

    // Register as tickobserver
    simulation.addTickObserver(tickObserver = new Observer() {
      public void update(Observable obs, Object obj) {
        if (simulation ==  null || simulationTime == null)
          return;
        
        // During simulation running, only update text 10 times each second
        if (lastTextUpdateTime < System.currentTimeMillis() - 100) {
          lastTextUpdateTime = System.currentTimeMillis();
          simulationTime.setText("Current simulation time: " + simulation.getSimulationTime());
        }

        if (simulationStopTime > 0 && simulationStopTime <= simulation.getSimulationTime() && simulation.isRunning()) {
          // Time to stop simulation now
          simulation.stopSimulation();
          simulationStopTime = -1;
        }
      }
    });

    // Register as simulation observer
    simulation.addObserver(simObserver = new Observer() {
      public void update(Observable obs, Object obj) {
        if (simulation.isRunning()) {
          startButton.setEnabled(false);
          stopButton.setEnabled(true);
        } else {
          startButton.setEnabled(true);
          stopButton.setEnabled(false);
          simulationStopTime = -1;
        }
        sliderDelay.setValue((int) simulation.getDelayTime());
        simulationTime.setText("Current simulation time: " + simulation.getSimulationTime());
      }
    });


    // Main panel
    JPanel controlPanel = new JPanel();
    controlPanel.setLayout(new BoxLayout(controlPanel, BoxLayout.Y_AXIS));
    
    setContentPane(controlPanel);

    // Add control buttons
    smallPanel = new JPanel();
    smallPanel.setLayout(new BoxLayout(smallPanel, BoxLayout.X_AXIS));
    smallPanel.setBorder(BorderFactory.createEmptyBorder(10, 5, 10, 5));

    button = new JButton("Start");
    button.setActionCommand("start");
    button.addActionListener(myEventHandler);
    startButton = button;
    smallPanel.add(button);

    button = new JButton("Stop");
    button.setActionCommand("stop");
    button.addActionListener(myEventHandler);
    stopButton = button;
    smallPanel.add(button);

    button = new JButton("Tick all motes once");
    button.setActionCommand("tickall");
    button.addActionListener(myEventHandler);
    smallPanel.add(button);

    smallPanel.setAlignmentX(Component.LEFT_ALIGNMENT);
    controlPanel.add(smallPanel);

    smallPanel = new JPanel();
    smallPanel.setLayout(new BoxLayout(smallPanel, BoxLayout.X_AXIS));
    smallPanel.setBorder(BorderFactory.createEmptyBorder(0, 5, 10, 5));

    button = new JButton("Run until");
    button.setActionCommand("rununtil");
    button.addActionListener(myEventHandler);
    smallPanel.add(button);

    smallPanel.add(Box.createHorizontalStrut(10));
    
    NumberFormat integerFormat = NumberFormat.getIntegerInstance();
    stopTimeTextField = new JFormattedTextField(integerFormat);
    stopTimeTextField.setValue(simulation.getSimulationTime());
    stopTimeTextField.addPropertyChangeListener("value", new PropertyChangeListener() {
      public void propertyChange(PropertyChangeEvent e) {
        JFormattedTextField numberTextField = (JFormattedTextField) e.getSource();
        int untilTime = ((Number) numberTextField.getValue()).intValue();
        if (untilTime < simulation.getSimulationTime()) {
          numberTextField.setValue(new Integer(simulation.getSimulationTime() + simulation.getTickTime()));
        }
      }
    });
    smallPanel.add(stopTimeTextField);

    smallPanel.setAlignmentX(Component.LEFT_ALIGNMENT);
    controlPanel.add(smallPanel);

    
    // Add delay slider
    smallPanel = new JPanel();
    smallPanel.setLayout(new BoxLayout(smallPanel, BoxLayout.Y_AXIS));
    smallPanel.setBorder(BorderFactory.createEmptyBorder(0, 10, 10, 10));

    simulationTime = new JLabel();
    simulationTime.setText("Current simulation time: " + simulation.getSimulationTime());

    smallPanel.add(simulationTime);
    smallPanel.add(Box.createRigidArea(new Dimension(0, 10)));
    
    smallPanel.add(new JLabel("Delay (ms) between each tick"));
    
    JSlider slider;
    if (simulation.getDelayTime() > MAX_DELAY_TIME)
      slider = new JSlider(JSlider.HORIZONTAL, MIN_DELAY_TIME, simulation.getDelayTime(), simulation.getDelayTime());
    else
      slider = new JSlider(JSlider.HORIZONTAL, MIN_DELAY_TIME, MAX_DELAY_TIME, simulation.getDelayTime());
    
    slider.addChangeListener(myEventHandler);
    slider.setMajorTickSpacing(20);
    slider.setPaintTicks(true);
    
    slider.setPaintLabels(true);

    sliderDelay = slider;
    smallPanel.add(slider);

    controlPanel.add(smallPanel);

    pack();

    try {
      setSelected(true);
    } catch (java.beans.PropertyVetoException e) {
      // Could not select
    }
    
  }
  
  private class MyEventHandler implements ActionListener, ChangeListener {
    public void stateChanged(ChangeEvent e) {
      if (e.getSource() == sliderDelay) {
        simulation.setDelayTime(sliderDelay.getValue());
      } else
        logger.debug("Unhandled state change: " + e);
    }
    public void actionPerformed(ActionEvent e) {
      if (e.getActionCommand().equals("start")) {
        simulationStopTime = -1; // Reset until time
        simulation.startSimulation();
      } else if (e.getActionCommand().equals("stop")) {
        simulationStopTime = -1; // Reset until time
        if (simulation.isRunning())
          simulation.stopSimulation();
      } else if (e.getActionCommand().equals("tickall")) {
        simulationStopTime = -1; // Reset until time
        simulation.tickSimulation();
      } else if (e.getActionCommand().equals("rununtil")) {
        // Set new stop time
        simulationStopTime = ((Number) stopTimeTextField.getValue()).intValue();
        if (simulationStopTime > simulation.getSimulationTime() && !simulation.isRunning()) {
          simulation.startSimulation();
        } else {
          if (simulation.isRunning())
            simulation.stopSimulation();
          simulationStopTime = -1;
        }
      } else
        logger.debug("Unhandled action: " + e.getActionCommand());
    }
  } MyEventHandler myEventHandler = new MyEventHandler();
  
  public void closePlugin() {
    // Remove log observer from all log interfaces
    if (simObserver != null)
      simulation.deleteObserver(simObserver);
    
    if (tickObserver != null)
      simulation.deleteTickObserver(tickObserver);
  }
  
}

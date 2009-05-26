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
 * $Id: SimControl.java,v 1.14 2009/05/26 14:27:00 fros4943 Exp $
 */

package se.sics.cooja.plugins;

import java.awt.*;
import java.awt.event.*;
import java.beans.PropertyChangeEvent;
import java.beans.PropertyChangeListener;
import java.text.NumberFormat;
import java.util.*;
import javax.swing.*;
import javax.swing.Timer;
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

  private Simulation simulation;

  private static final int SLIDE_MAX = 921; // e^9.21 => ~10000
  private static final int TIME_MAX = 10000;

  private static final int LABEL_UPDATE_INTERVAL = 100;

  private JSlider sliderDelay;
  private JLabel simulationTime, delayLabel;
  private JButton startButton, stopButton;
  private JFormattedTextField stopTimeTextField;

  private Observer simObserver;

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

    /* Update current time label when simulation is running */
    if (simulation.isRunning()) {
      updateLabelTimer.start(); 
    }

    /* Observe current simulation */
    simulation.addObserver(simObserver = new Observer() {
      public void update(Observable obs, Object obj) {
        if (simulation.isRunning()) {
          startButton.setEnabled(false);
          stopButton.setEnabled(true);
          
          /* Start label update timer */
          if (!updateLabelTimer.isRunning()) {
            updateLabelTimer.start();
          }
        } else {
          startButton.setEnabled(true);
          stopButton.setEnabled(false);

          /* Update simulation stop text field */
          if (!stopEvent.isScheduled()) {
            stopTimeTextField.setValue(simulation.getSimulationTimeMillis());
          }
        }

        SwingUtilities.invokeLater(new Runnable() {
          public void run() {
            sliderDelay.setValue(convertTimeToSlide(simulation.getDelayTime()));
            simulationTime.setText("Current simulation time: " + simulation.getSimulationTimeMillis());
            simulationTime.setToolTipText("Simulation time in microseconds: " + simulation.getSimulationTime());
          }
        });
      }
    });
        
    // Main panel
    JPanel controlPanel = new JPanel();
    controlPanel.setLayout(new BoxLayout(controlPanel, BoxLayout.Y_AXIS));

    getContentPane().add(controlPanel, BorderLayout.NORTH);

    /* Control buttons */
    smallPanel = new JPanel();
    smallPanel.setLayout(new BoxLayout(smallPanel, BoxLayout.X_AXIS));
    smallPanel.setBorder(BorderFactory.createEmptyBorder(10, 5, 10, 5));

    button = new JButton("Start");
    button.setActionCommand("start");
    button.addActionListener(myEventHandler);
    startButton = button;
    smallPanel.add(button);

    button = new JButton("Pause");
    button.setActionCommand("stop");
    button.addActionListener(myEventHandler);
    stopButton = button;
    smallPanel.add(button);

    button = new JButton("Step millisecond");
    button.setActionCommand("single_ms");
    button.addActionListener(myEventHandler);
    smallPanel.add(button);

    smallPanel.setAlignmentX(Component.LEFT_ALIGNMENT);
    controlPanel.add(smallPanel);

    /* Run until */
    smallPanel = new JPanel();
    smallPanel.setLayout(new BoxLayout(smallPanel, BoxLayout.X_AXIS));
    smallPanel.setBorder(BorderFactory.createEmptyBorder(5, 5, 0, 5));

    JLabel label = new JLabel("Run until:");
    smallPanel.add(label);

    smallPanel.add(Box.createHorizontalStrut(10));

    NumberFormat integerFormat = NumberFormat.getIntegerInstance();
    stopTimeTextField = new JFormattedTextField(integerFormat);
    stopTimeTextField.addPropertyChangeListener("value", new PropertyChangeListener() {
      public void propertyChange(PropertyChangeEvent e) {
        /* Remove already scheduled stop event */
        if (stopEvent.isScheduled()) {
          stopEvent.remove();
        }

        JFormattedTextField numberTextField = (JFormattedTextField) e.getSource();
        long stopTime = ((Number) numberTextField.getValue()).intValue()*Simulation.MILLISECOND;
        if (stopTime <= simulation.getSimulationTime()) {
          /* No simulation stop scheduled */
          numberTextField.setBackground(Color.LIGHT_GRAY);
          numberTextField.setToolTipText("Enter future simulation time");
        } else {
          /* Schedule simulation stop */
          numberTextField.setBackground(Color.WHITE);
          numberTextField.setToolTipText("Simulation will stop at time (us): " + stopTime);
          simulation.scheduleEvent(stopEvent, stopTime);
        }
      }
    });
    stopTimeTextField.setValue(simulation.getSimulationTimeMillis());
    stopTimeTextField.setSize(100, stopTimeTextField.getHeight());

    smallPanel.add(stopTimeTextField);
    smallPanel.add(Box.createHorizontalGlue());

    smallPanel.setAlignmentX(Component.LEFT_ALIGNMENT);
    controlPanel.add(smallPanel);

    /* Time label */
    smallPanel = new JPanel();
    smallPanel.setLayout(new BoxLayout(smallPanel, BoxLayout.X_AXIS));
    smallPanel.setBorder(BorderFactory.createEmptyBorder(5, 5, 0, 5));

    label = new JLabel("Current simulation time: " + simulation.getSimulationTimeMillis());
    smallPanel.add(label);
    simulationTime = label;

    smallPanel.setAlignmentX(Component.LEFT_ALIGNMENT);
    controlPanel.add(smallPanel);

    /* Delay label */
    smallPanel = new JPanel();
    smallPanel.setLayout(new BoxLayout(smallPanel, BoxLayout.X_AXIS));
    smallPanel.setBorder(BorderFactory.createEmptyBorder(5, 5, 0, 5));

    if (simulation.getDelayTime() > 0) {
      label = new JLabel("Delay: " + simulation.getDelayTime() + " ms");
    } else {
      label = new JLabel("No simulation delay");
    }
    smallPanel.add(label);
    delayLabel = label;

    smallPanel.setAlignmentX(Component.LEFT_ALIGNMENT);
    controlPanel.add(smallPanel);

    /* Delay slider */
    smallPanel = new JPanel();
    smallPanel.setLayout(new BoxLayout(smallPanel, BoxLayout.X_AXIS));
    smallPanel.setBorder(BorderFactory.createEmptyBorder(5, 5, 10, 5));

    JSlider slider = new JSlider(JSlider.HORIZONTAL, 0, SLIDE_MAX, convertTimeToSlide(simulation.getDelayTime()));
    slider.addChangeListener(myEventHandler);

    Hashtable labelTable = new Hashtable();
    for (int i=0; i < 100; i += 10) {
      labelTable.put(new Integer(convertTimeToSlide(i)), new JLabel("."));
    }
    for (int i=200; i < 10000; i += 500) {
      labelTable.put(new Integer(convertTimeToSlide(i)), new JLabel(":"));
    }
    slider.setLabelTable(labelTable);
    slider.setPaintLabels(true);

    smallPanel.add(slider);
    sliderDelay = slider;

    smallPanel.setAlignmentX(Component.LEFT_ALIGNMENT);
    controlPanel.add(smallPanel);

    pack();

    try {
      setSelected(true);
    } catch (java.beans.PropertyVetoException e) {
      // Could not select
    }
  }

  private int convertSlideToTime(int slide) {
    if (slide == SLIDE_MAX) {
      return TIME_MAX;
    }
    return (int) Math.round(Math.exp(slide/100.0) - 1.0);
  }

  private int convertTimeToSlide(int time) {
    if (time == TIME_MAX) {
      return SLIDE_MAX;
    }

    return (int) Math.round((Math.log(time + 1)*100.0));
  }

  private class MyEventHandler implements ActionListener, ChangeListener {
    public void stateChanged(ChangeEvent e) {
      if (e.getSource() == sliderDelay) {
        simulation.setDelayTime(convertSlideToTime(sliderDelay.getValue()));
        if (simulation.getDelayTime() > 0) {
          delayLabel.setText("Delay: " + simulation.getDelayTime() + " ms");
        } else {
          delayLabel.setText("No simulation delay");
        }
      } else {
        logger.debug("Unhandled state change: " + e);
      }
    }
    public void actionPerformed(ActionEvent e) {
      if (e.getActionCommand().equals("start")) {
        simulation.startSimulation();
      } else if (e.getActionCommand().equals("stop")) {
        if (simulation.isRunning()) {
          simulation.stopSimulation();
        }
      } else if (e.getActionCommand().equals("single_ms")) {
        simulation.stepMillisecondSimulation();
      } else {
        logger.debug("Unhandled action: " + e.getActionCommand());
      }
    }
  } MyEventHandler myEventHandler = new MyEventHandler();

  public void closePlugin() {
    /* Remove simulation observer */
    if (simObserver != null) {
      simulation.deleteObserver(simObserver);
    }

    /* Remove stop event */
    if (stopEvent.isScheduled()) {
      stopEvent.remove();
    }

    /* Remove label update timer */
    updateLabelTimer.stop();
  }

  private TimeEvent stopEvent = new TimeEvent(0) {
    public void execute(long t) {
      /* Stop simulation */
      simulation.stopSimulation();
    }
  };

  private Timer updateLabelTimer = new Timer(LABEL_UPDATE_INTERVAL, new ActionListener() {
    public void actionPerformed(ActionEvent e) {
      simulationTime.setText("Current simulation time: " + simulation.getSimulationTimeMillis());

      /* Automatically stop if simulation is no longer running */
      if (!simulation.isRunning()) {
        updateLabelTimer.stop();
      }
    }
  });
}

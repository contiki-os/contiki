/*
 * Copyright (c) 2009, Swedish Institute of Computer Science.
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
 * $Id: SimControl.java,v 1.16 2009/10/27 10:12:00 fros4943 Exp $
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
 * Control panel for starting and pausing the current simulation.
 * Allows for configuring the simulation delay.
 *
 * @author Fredrik Osterlind
 */
@ClassDescription("Control Panel")
@PluginType(PluginType.SIM_STANDARD_PLUGIN)
public class SimControl extends VisPlugin {
  private static final long serialVersionUID = 8452253637624664192L;
  private static Logger logger = Logger.getLogger(SimControl.class);

  private Simulation simulation;
  private static final int SLIDE_MIN = -100;
  private static final int SLIDE_MAX = 1000;

  private static final int LABEL_UPDATE_INTERVAL = 100;

  private JButton startButton, stopButton;
  private JSlider sliderDelay;
  private JLabel simulationTime, delayLabel;
  private JFormattedTextField stopTimeTextField;

  private Observer simObserver;

  /**
   * Create a new simulation control panel.
   *
   * @param simulation Simulation to control
   */
  public SimControl(Simulation simulation, GUI gui) {
    super("Control Panel", gui);
    this.simulation = simulation;

    /* Update current time label when simulation is running */
    if (simulation.isRunning()) {
      updateLabelTimer.start(); 
    }

    /* Container */
    JPanel smallPanel;
    JPanel controlPanel = new JPanel();
    controlPanel.setLayout(new BoxLayout(controlPanel, BoxLayout.Y_AXIS));

    getContentPane().add(controlPanel, BorderLayout.NORTH);

    /* Control buttons */
    smallPanel = new JPanel();
    smallPanel.setLayout(new BoxLayout(smallPanel, BoxLayout.X_AXIS));
    smallPanel.setBorder(BorderFactory.createEmptyBorder(10, 5, 10, 5));

    smallPanel.add(startButton = new JButton(startAction));
    smallPanel.add(stopButton = new JButton(stopAction));
    smallPanel.add(new JButton(stepAction));

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

        final long t = ((Number)e.getNewValue()).intValue()*Simulation.MILLISECOND;
        if (t <= SimControl.this.simulation.getSimulationTime()) {
          /* No simulation stop scheduled */
          stopTimeTextField.setBackground(Color.LIGHT_GRAY);
          stopTimeTextField.setToolTipText("Enter simulation time when to automatically pause");
        } else {
          /* Schedule simulation stop */
          stopTimeTextField.setBackground(Color.WHITE);
          stopTimeTextField.setToolTipText("Simulation will stop at time (us): " + t);
          SimControl.this.simulation.invokeSimulationThread(new Runnable() {
            public void run() {
              SimControl.this.simulation.scheduleEvent(stopEvent, t);
            }
          });
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

    label = new JLabel("?");
    smallPanel.add(label);
    simulationTime = label;

    smallPanel.setAlignmentX(Component.LEFT_ALIGNMENT);
    controlPanel.add(smallPanel);

    /* Delay label */
    smallPanel = new JPanel();
    smallPanel.setLayout(new BoxLayout(smallPanel, BoxLayout.X_AXIS));
    smallPanel.setBorder(BorderFactory.createEmptyBorder(5, 5, 0, 5));

    label = new JLabel("?");
    smallPanel.add(label);
    delayLabel = label;

    smallPanel.setAlignmentX(Component.LEFT_ALIGNMENT);
    controlPanel.add(smallPanel);

    /* Delay slider */
    smallPanel = new JPanel();
    smallPanel.setLayout(new BoxLayout(smallPanel, BoxLayout.X_AXIS));
    smallPanel.setBorder(BorderFactory.createEmptyBorder(5, 5, 10, 5));

    sliderDelay = new JSlider(
        JSlider.HORIZONTAL, 
        SLIDE_MIN, 
        SLIDE_MAX, 
        convertTimeToSlide(simulation.getDelayTime()));
    sliderDelay.addChangeListener(new ChangeListener() {
      public void stateChanged(ChangeEvent e) {
        SimControl.this.simulation.setDelayTime(
            convertSlideToTime(sliderDelay.getValue()));
        updateValues();
      }
    });

    smallPanel.add(sliderDelay);

    smallPanel.setAlignmentX(Component.LEFT_ALIGNMENT);
    controlPanel.add(smallPanel);

    /* Observe current simulation */
    simulation.addObserver(simObserver = new Observer() {
      public void update(Observable obs, Object obj) {
        SwingUtilities.invokeLater(new Runnable() {
          public void run() {
            updateValues();
          }
        });
      }
    });
    /* Set initial values */
    updateValues();

    pack();
  }

  private void updateValues() {
    /* Update simulation delay */
    sliderDelay.setValue(convertTimeToSlide(simulation.getDelayTime()));
    if (simulation.getDelayTime() == 0) {
      delayLabel.setText("No simulation delay");
    } else if (simulation.getDelayTime() == Integer.MIN_VALUE) {
      delayLabel.setText("Real time");
    } else if (simulation.getDelayTime() > 0) {
      delayLabel.setText("Delay: " + simulation.getDelayTime() + " ms");
    } else {
      delayLabel.setText("Delay: 1/" + (-simulation.getDelayTime()) + " ms");
    }

    /* Update current time */
    simulationTime.setText("Current simulation time: "
        + simulation.getSimulationTimeMillis()
        + " ms");
    if (simulation.isRunning() && !updateLabelTimer.isRunning()) {
      updateLabelTimer.start();
    }
    if (!simulation.isRunning()) {
      simulationTime.setToolTipText("Simulation time in microseconds: "
          + simulation.getSimulationTime());
    }

    /* Update control buttons */
    if (simulation.isRunning()) {
      startAction.setEnabled(false);
      stopAction.setEnabled(true);
      stepAction.setEnabled(false);
    } else {
      startAction.setEnabled(true);
      stopAction.setEnabled(false);
      stepAction.setEnabled(true);

      if (!stopEvent.isScheduled()) {
        stopTimeTextField.setValue(simulation.getSimulationTimeMillis());
      }
    }
  }

  private int convertSlideToTime(int slide) {
    if (slide == SLIDE_MIN) {
      /* Special case: no delay */
      return 0;
    }
    if (slide == SLIDE_MIN+1) {
      /* Special case: real time */
      return Integer.MIN_VALUE;
    }
    if (slide <= 0) {
      return slide-2; /* Ignore special cases */ 
    }
    return slide; 
  }

  private int convertTimeToSlide(int time) {
    if (time == 0) {
      /* Special case: no delay */
      return SLIDE_MIN;
    }
    if (time == Integer.MIN_VALUE) {
      /* Special case: real time */
      return SLIDE_MIN+1;
    }
    if (time < 0) {
      return time+2; /* Ignore special cases */
    }
    return time;
  }

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
      SwingUtilities.invokeLater(new Runnable() {
        public void run() {
          stopTimeTextField.setBackground(Color.LIGHT_GRAY);
          stopTimeTextField.setToolTipText("Enter simulation time when to automatically pause");
          stopTimeTextField.requestFocus();
        }
      });
    }
  };

  private Timer updateLabelTimer = new Timer(LABEL_UPDATE_INTERVAL, new ActionListener() {
    public void actionPerformed(ActionEvent e) {
      simulationTime.setText("Current simulation time: "
          + simulation.getSimulationTimeMillis()
          + " ms");

      /* Automatically stop if simulation is no longer running */
      if (!simulation.isRunning()) {
        updateLabelTimer.stop();
      }
    }
  });

  private Action startAction = new AbstractAction("Start") {
    public void actionPerformed(ActionEvent e) {
      simulation.startSimulation();
      stopButton.requestFocus();
    }
  };
  private Action stopAction = new AbstractAction("Pause") {
    public void actionPerformed(ActionEvent e) {
      simulation.stopSimulation();
      startButton.requestFocus();
    }
  };
  private Action stepAction = new AbstractAction("Step millisecond") {
    public void actionPerformed(ActionEvent e) {
      simulation.stepMillisecondSimulation();
    }
  };
}

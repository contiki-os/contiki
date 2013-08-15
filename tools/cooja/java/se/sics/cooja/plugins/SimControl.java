/*
 * Copyright (c) 2012, Swedish Institute of Computer Science.
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
 */

package se.sics.cooja.plugins;

import java.awt.BorderLayout;
import java.awt.Component;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.util.Observable;
import java.util.Observer;

import javax.swing.AbstractAction;
import javax.swing.Action;
import javax.swing.BorderFactory;
import javax.swing.BoxLayout;
import javax.swing.ButtonGroup;
import javax.swing.JButton;
import javax.swing.JLabel;
import javax.swing.JMenu;
import javax.swing.JMenuBar;
import javax.swing.JMenuItem;
import javax.swing.JPanel;
import javax.swing.JRadioButtonMenuItem;
import javax.swing.SwingUtilities;
import javax.swing.Timer;

import se.sics.cooja.ClassDescription;
import se.sics.cooja.GUI;
import se.sics.cooja.HasQuickHelp;
import se.sics.cooja.PluginType;
import se.sics.cooja.Simulation;
import se.sics.cooja.VisPlugin;

/**
 * Control panel for starting and pausing the current simulation.
 *
 * @author Fredrik Osterlind
 */
@ClassDescription("Simulation control")
@PluginType(PluginType.SIM_STANDARD_PLUGIN)
public class SimControl extends VisPlugin implements HasQuickHelp {
  private static final int LABEL_UPDATE_INTERVAL = 150;

  private Simulation simulation;

  private JButton startButton, stopButton;
  private JLabel simulationTime, simulationSpeedup;

  private Observer simObserver;

  private long lastSimulationTimeTimestamp;
  private long lastSystemTimeTimestamp;

  /**
   * Create a new simulation control panel.
   *
   * @param simulation Simulation to control
   */
  public SimControl(Simulation simulation, GUI gui) {
    super("Simulation control", gui);
    this.simulation = simulation;

    /* Update current time label when simulation is running */
    if (simulation.isRunning()) {
      updateLabelTimer.start();
    }

    /* Menus */
    JMenuBar menuBar = new JMenuBar();
    JMenu runMenu = new JMenu("Run");
    JMenu speedMenu = new JMenu("Speed limit");

    menuBar.add(runMenu);
    menuBar.add(speedMenu);
    this.setJMenuBar(menuBar);

    runMenu.add(new JMenuItem(startAction));
    runMenu.add(new JMenuItem(stopAction));
    runMenu.add(new JMenuItem(stepAction));
    runMenu.add(new JMenuItem(reloadAction));

    ButtonGroup speedlimitButtonGroup = new ButtonGroup();
    JRadioButtonMenuItem limitMenuItemNo = new JRadioButtonMenuItem(
        new ChangeMaxSpeedLimitAction("No speed limit", null));
    speedlimitButtonGroup.add(limitMenuItemNo);
    speedMenu.add(limitMenuItemNo);
    JRadioButtonMenuItem limitMenuItem1 = new JRadioButtonMenuItem(
        new ChangeMaxSpeedLimitAction("1%", 0.01));
    speedlimitButtonGroup.add(limitMenuItem1);
    speedMenu.add(limitMenuItem1);
    JRadioButtonMenuItem limitMenuItem2 = new JRadioButtonMenuItem(
        new ChangeMaxSpeedLimitAction("10%", 0.10));
    speedlimitButtonGroup.add(limitMenuItem2);
    speedMenu.add(limitMenuItem2);
    JRadioButtonMenuItem limitMenuItem3 = new JRadioButtonMenuItem(
            new ChangeMaxSpeedLimitAction("100%", 1.0));
        speedlimitButtonGroup.add(limitMenuItem3);
        speedMenu.add(limitMenuItem3);
        JRadioButtonMenuItem limitMenuItem200 = new JRadioButtonMenuItem(
                new ChangeMaxSpeedLimitAction("200%", 2.0));
            speedlimitButtonGroup.add(limitMenuItem200);
            speedMenu.add(limitMenuItem200);
    JRadioButtonMenuItem limitMenuItem4 = new JRadioButtonMenuItem(
        new ChangeMaxSpeedLimitAction("1000%", 10.0));
    speedlimitButtonGroup.add(limitMenuItem4);
    speedMenu.add(limitMenuItem4);

    if (simulation.getSpeedLimit() == null) {
      limitMenuItemNo.setSelected(true);
    } else if (simulation.getSpeedLimit().doubleValue() == 0.01) {
      limitMenuItem1.setSelected(true);
    } else if (simulation.getSpeedLimit().doubleValue() == 0.10) {
      limitMenuItem2.setSelected(true);
    } else if (simulation.getSpeedLimit().doubleValue() == 1.0) {
        limitMenuItem3.setSelected(true);
    } else if (simulation.getSpeedLimit().doubleValue() == 2.0) {
        limitMenuItem200.setSelected(true);
    } else if (simulation.getSpeedLimit().doubleValue() == 10) {
      limitMenuItem4.setSelected(true);
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
    smallPanel.add(new JButton(reloadAction));

    smallPanel.setAlignmentX(Component.LEFT_ALIGNMENT);
    controlPanel.add(smallPanel);

    smallPanel.setAlignmentX(Component.LEFT_ALIGNMENT);
    controlPanel.add(smallPanel);

    /* Time label */
    smallPanel = new JPanel();
    smallPanel.setLayout(new BoxLayout(smallPanel, BoxLayout.X_AXIS));
    smallPanel.setBorder(BorderFactory.createEmptyBorder(5, 5, 0, 5));

    JLabel label = new JLabel("?");
    smallPanel.add(label);
    simulationTime = label;

    smallPanel.setAlignmentX(Component.LEFT_ALIGNMENT);
    controlPanel.add(smallPanel);

    /* Simulation speed label */
    smallPanel = new JPanel();
    smallPanel.setLayout(new BoxLayout(smallPanel, BoxLayout.X_AXIS));
    smallPanel.setBorder(BorderFactory.createEmptyBorder(5, 5, 0, 5));

    label = new JLabel("?");
    smallPanel.add(label);
    simulationSpeedup = label;

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

    this.lastSystemTimeTimestamp = System.currentTimeMillis();
    this.lastSimulationTimeTimestamp = 0;

    /* XXX HACK: here we set the position and size of the window when it
     * appears on a blank simulation screen. */
    this.setLocation(400, 0);
    this.setSize(280, 160);
  }

  private class ChangeMaxSpeedLimitAction extends AbstractAction {
    private Double maxSpeed;
    public ChangeMaxSpeedLimitAction(String name, Double maxSpeed) {
      super(name);
      this.maxSpeed = maxSpeed;
    }
    public void actionPerformed(ActionEvent e) {
      simulation.setSpeedLimit(maxSpeed);
    }
  }

  private void updateValues() {
    /* Update current time */
    simulationTime.setText(getTimeString());
    simulationSpeedup.setText("Speed: ---");
    if (simulation.isRunning() && !updateLabelTimer.isRunning()) {
      updateLabelTimer.start();
    }

    /* Update control buttons */
    if (simulation.isRunning()) {
      startAction.setEnabled(false);
      stopAction.setEnabled(true);
      stepAction.setEnabled(false);
    } else {
      if(simulation.isRunnable()) {
        startAction.setEnabled(true);
        stepAction.setEnabled(true);
      } else {
        startAction.setEnabled(false);
        stepAction.setEnabled(false);
      }
      stopAction.setEnabled(false);
    }
  }

  private static final long TIME_SECOND = 1000*Simulation.MILLISECOND;
  private static final long TIME_MINUTE = 60*TIME_SECOND;
  private static final long TIME_HOUR = 60*TIME_MINUTE;
  public String getTimeString() {
    long t = simulation.getSimulationTime();
    long h = (t / TIME_HOUR);
    t -= (t / TIME_HOUR)*TIME_HOUR;
    long m = (t / TIME_MINUTE);
    t -= (t / TIME_MINUTE)*TIME_MINUTE;
    long s = (t / TIME_SECOND);
    t -= (t / TIME_SECOND)*TIME_SECOND;
    long ms = t / Simulation.MILLISECOND;
    if (h > 0) {
      return String.format("Time: %d:%02d:%02d.%03d", h,m,s,ms);
    } else {
      return String.format("Time: %02d:%02d.%03d", m,s,ms);
    }
  }

  public void closePlugin() {
    /* Remove simulation observer */
    if (simObserver != null) {
      simulation.deleteObserver(simObserver);
    }

    /* Remove label update timer */
    updateLabelTimer.stop();
  }

  private Timer updateLabelTimer = new Timer(LABEL_UPDATE_INTERVAL, new ActionListener() {
    public void actionPerformed(ActionEvent e) {
      simulationTime.setText(getTimeString());

      long systemTimeDiff = System.currentTimeMillis() - lastSystemTimeTimestamp;

      if (systemTimeDiff > 1000) {
        long simulationTimeDiff = simulation.getSimulationTimeMillis() - lastSimulationTimeTimestamp;
        lastSimulationTimeTimestamp = simulation.getSimulationTimeMillis();
        lastSystemTimeTimestamp = System.currentTimeMillis();

        double speedup = (double)simulationTimeDiff / (double)systemTimeDiff;
        simulationSpeedup.setText(String.format("Speed: %2.2f%%", 100 * speedup));
      }

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
  private Action stepAction = new AbstractAction("Step") {
    public void actionPerformed(ActionEvent e) {
      simulation.stepMillisecondSimulation();
    }
  };
  private Action reloadAction = new AbstractAction("Reload") {
    public void actionPerformed(ActionEvent e) {
      simulation.getGUI().reloadCurrentSimulation(simulation.isRunning());
    }
  };

  public String getQuickHelp() {
    return "<b>Control Panel</b>" +
        "<p>The control panel controls the simulation. " +
        "<p><i>Start</i> starts the simulation. " +
        "<p><i>Pause</i> stops the simulation. " +
        "<p>The keyboard shortcut for starting and pausing the simulation is <i>Ctrl+S</i>. " +
        "<p><i>Step</i> runs the simulation for one millisecond. " +
        "<p><i>Reload</i> reloads and restarts the simulation. " +
        "<p>Simulation speed is controlled via the Speed limit menu.";
  }
}

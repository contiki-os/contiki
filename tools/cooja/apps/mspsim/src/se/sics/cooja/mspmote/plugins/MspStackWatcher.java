/*
 * Copyright (c) 2007, Swedish Institute of Computer Science.
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

package se.sics.cooja.mspmote.plugins;

import java.awt.BorderLayout;
import java.awt.GridLayout;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.util.Observable;
import java.util.Observer;

import javax.swing.JButton;
import javax.swing.JOptionPane;
import javax.swing.JPanel;

import org.apache.log4j.Logger;

import se.sics.cooja.ClassDescription;
import se.sics.cooja.GUI;
import se.sics.cooja.Mote;
import se.sics.cooja.MotePlugin;
import se.sics.cooja.PluginType;
import se.sics.cooja.Simulation;
import se.sics.cooja.SupportedArguments;
import se.sics.cooja.VisPlugin;
import se.sics.cooja.mspmote.MspMote;
import se.sics.mspsim.core.MSP430;
import se.sics.mspsim.ui.StackUI;
import se.sics.mspsim.util.Utils;

@ClassDescription("Msp Stack Watcher")
@PluginType(PluginType.MOTE_PLUGIN)
@SupportedArguments(motes = {MspMote.class})
public class MspStackWatcher extends VisPlugin implements MotePlugin {
  private static Logger logger = Logger.getLogger(MspStackWatcher.class);

  private MspMote mspMote;
  private MSP430 cpu;
  private StackUI stackUI;

  private Simulation simulation;
  private Observer stackObserver = null;
  private JButton startButton;
  private JButton stopButton;

  public MspStackWatcher(Mote mote, Simulation simulationToVisualize, GUI gui) {
    super("Msp Stack Watcher", gui);
    this.mspMote = (MspMote) mote;
    cpu = mspMote.getCPU();
    simulation = simulationToVisualize;

    getContentPane().setLayout(new BorderLayout());

    // Register as stack observable
    if (stackObserver == null) {
      mspMote.getStackOverflowObservable().addObserver(stackObserver = new Observer() {
        public void update(Observable obs, Object obj) {
          simulation.stopSimulation();
          JOptionPane.showMessageDialog(
              MspStackWatcher.this,
              "Bad memory access!\nSimulation stopped.\n" +
              "\nCurrent stack pointer = 0x" + Utils.hex16(cpu.reg[MSP430.SP]) +
              "\nStart of heap = 0x" + Utils.hex16(cpu.getDisAsm().getMap().heapStartAddress),
              "Stack overflow", JOptionPane.ERROR_MESSAGE
          );
        }
      });
    }

    // Create stack overflow controls
    startButton = new JButton("Stop simulation on stack overflow");
    startButton.addActionListener(new ActionListener() {
      public void actionPerformed(ActionEvent e) {
        startButton.setEnabled(false);
        stopButton.setEnabled(true);

        mspMote.monitorStack(true);
      }
    });

    stopButton = new JButton("Cancel");
    stopButton.setEnabled(false);
    stopButton.addActionListener(new ActionListener() {
      public void actionPerformed(ActionEvent e) {
        startButton.setEnabled(true);
        stopButton.setEnabled(false);

        mspMote.monitorStack(false);
      }
    });

    // Create nfi's stack viewer
    stackUI = new StackUI(cpu);
    stackUI.init("MSPSim stack", mspMote.registry);
    stackUI.start();

    // Register as log listener
    /*if (logObserver == null && mspMote.getInterfaces().getLog() != null) {
      mspMote.getInterfaces().getLog().addObserver(logObserver = new Observer() {
        public void update(Observable obs, Object obj) {
          stackUI.addNote(mspMote.getInterfaces().getLog().getLastLogMessage());
        }
      });
    }*/

    JPanel controlPanel = new JPanel(new GridLayout(2,1));
    controlPanel.add(startButton);
    controlPanel.add(stopButton);

    add(BorderLayout.CENTER, stackUI);
    add(BorderLayout.SOUTH, controlPanel);

    setSize(240, 300);

    // Tries to select this plugin
    try {
      setSelected(true);
    } catch (java.beans.PropertyVetoException e) {
      // Could not select
    }
  }

  public void closePlugin() {
    mspMote.getStackOverflowObservable().deleteObserver(stackObserver);
    stackUI.stop();
  }

  public Mote getMote() {
    return mspMote;
  }

}

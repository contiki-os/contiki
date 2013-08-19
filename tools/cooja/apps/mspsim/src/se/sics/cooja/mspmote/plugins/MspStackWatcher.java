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
import java.awt.Color;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.io.IOException;
import java.util.ArrayList;
import java.util.Collection;

import javax.swing.JLabel;
import javax.swing.JOptionPane;
import javax.swing.JToggleButton;
import javax.swing.SwingUtilities;

import org.apache.log4j.Logger;
import org.jdom.Element;

import se.sics.cooja.ClassDescription;
import se.sics.cooja.GUI;
import se.sics.cooja.Mote;
import se.sics.cooja.MotePlugin;
import se.sics.cooja.MoteTimeEvent;
import se.sics.cooja.PluginType;
import se.sics.cooja.Simulation;
import se.sics.cooja.SupportedArguments;
import se.sics.cooja.VisPlugin;
import se.sics.cooja.mspmote.MspMote;
import se.sics.cooja.mspmote.MspMoteType;
import se.sics.mspsim.core.MSP430;
import se.sics.mspsim.core.Memory.AccessMode;
import se.sics.mspsim.core.RegisterMonitor;
import se.sics.mspsim.ui.StackUI;

@ClassDescription("Msp Stack Watcher")
@PluginType(PluginType.MOTE_PLUGIN)
@SupportedArguments(motes = { MspMote.class })
public class MspStackWatcher extends VisPlugin implements MotePlugin {
  private static Logger logger = Logger.getLogger(MspStackWatcher.class);

  private Simulation simulation;
  private MSP430 cpu;
  private MspMote mspMote;

  private StackUI stackUI;
  private RegisterMonitor.Adapter registerMonitor = null;

  private JToggleButton toggleButton;

  private MoteTimeEvent increasePosTimeEvent;

  private Integer userOverriddenStack = null;

  private JLabel memLabel = new JLabel("");
  
  public MspStackWatcher(Mote mote, Simulation simulationToVisualize, GUI gui) {
    super("Msp Stack Watcher: " + mote, gui);
    this.mspMote = (MspMote) mote;
    cpu = mspMote.getCPU();
    simulation = simulationToVisualize;

    getContentPane().setLayout(new BorderLayout());

    toggleButton = new JToggleButton("Click to monitor for stack overflows");
    toggleButton.addActionListener(new ActionListener() {
      public void actionPerformed(ActionEvent e) {
        if (toggleButton.isSelected()) {
          toggleButton.setText("Monitoring for stack overflows");
          if (!activate(true)) {
            toggleButton.setBackground(Color.RED);
            toggleButton.setText("Monitoring for stack overflows - FAILED!");
            toggleButton.setSelected(false);
          }
          toggleButton.setBackground(null);
        } else {
          toggleButton.setBackground(null);
          toggleButton.setText("Click to monitor for stack overflows");
          deactivate();
        }
      }
    });

    /* Create Mspsim stack viewer */
    stackUI = new StackUI(cpu, -1); /* Needs manual updates */
    stackUI.init("Stack usage", mspMote.registry);
    stackUI.start();
    increasePosTimeEvent = new MoteTimeEvent(mspMote, 0) {
      public void execute(long t) {
        stackUI.requestIncreasePos();
        simulation.scheduleEvent(this, t + Simulation.MILLISECOND);
      }
    };
    simulation.scheduleEvent(increasePosTimeEvent, simulation.getSimulationTime());

    add(BorderLayout.NORTH, memLabel);
    add(BorderLayout.CENTER, stackUI);
    add(BorderLayout.SOUTH, toggleButton);

    setSize(400, 300);
  }

  private boolean activate(boolean gui) {
    try {
      int stack = ((MspMoteType) mspMote.getType()).getELF().getMap().stackStartAddress; 

      if (gui) {
    	  String s = (String)JOptionPane.showInputDialog(
    			  GUI.getTopParentContainer(),
    			  "With default linker scripts the stack starts at 0x" + Integer.toHexString(stack) + ".\n" +
    					  "If you are using a modified linker script, you may here correct the stack start address.",
    					  "Enter stack start address",
    					  JOptionPane.PLAIN_MESSAGE,
    					  null,
    					  null,
    					  "0x" + Integer.toHexString(userOverriddenStack!=null?userOverriddenStack:stack));
    	  userOverriddenStack = null;
    	  if (s != null) {
    		  try {
    			  int newStack = Integer.decode(s); 
    			  if (newStack != stack) {
    				  userOverriddenStack = newStack;
    			  }
    		  } catch (Exception e) {
    			  logger.error("Error parsing provided stack address: " + s, e);
    			  return false;
    		  }
    	  }
      }
	  if (userOverriddenStack != null) {
		  stack = userOverriddenStack; 
	  }
      
      int heap = ((MspMoteType) mspMote.getType()).getELF().getMap().heapStartAddress;
      if (stack < 0) {
        stack = cpu.config.ramStart + cpu.config.ramSize;
      }
      logger.debug("SP starts at: 0x" + Integer.toHexString(stack));
      logger.debug("Heap starts at: 0x" + Integer.toHexString(heap));
      logger.debug("Available stack: " + (stack-heap) + " bytes");
      memLabel.setText(String.format("Stack 0x%x, heap 0x%x", stack, heap));
      
      if (stack < 0 || heap < 0) {
        return false;
      }

      /*final int stackStartAddress = stack;*/
      final int heapStartAddress = heap;
      registerMonitor = new RegisterMonitor.Adapter() {
    	int min = Integer.MAX_VALUE;
        public void notifyWriteBefore(int register, final int sp, AccessMode mode) {
          /*logger.debug("SP is now: 0x" + Integer.toHexString(sp));*/
          final int available = sp - heapStartAddress;

          if (available < min) {
        	  min = available;
              String details = mspMote.getExecutionDetails();
              if (details != null) {
            	  logger.info(String.format(mspMote + ": Maximum stack usage: 0x%x, available stack 0x%x", sp, available));
            	  logger.info(details);
              }
          }
          
          if (available <= 0) {
            SwingUtilities.invokeLater(new Runnable() {
              public void run() {
                JOptionPane.showMessageDialog(GUI.getTopParentContainer(),
                    String.format("Stack overflow!\n\n" +
                    		"\tSP = 0x%05x\n" +
                    		"\tHeap start = 0x%05x\n\n" +
                    		"\tAvailable = %d\n", sp, heapStartAddress, available),
                    		"Stack overflow on " + mspMote,
                    JOptionPane.ERROR_MESSAGE);
              }
            });
            simulation.stopSimulation();
          }
        }
      };
      cpu.addRegisterWriteMonitor(MSP430.SP, registerMonitor);
    } catch (IOException e) {
      logger.warn("Stack monitoring failed: " + e.getMessage(), e);
      registerMonitor = null;
      return false;
    }
    return true;
  }

  private void deactivate() {
	  userOverriddenStack = null;
    if (registerMonitor != null) {
      cpu.removeRegisterWriteMonitor(MSP430.SP, registerMonitor);
      registerMonitor = null;
    }
  }

  public Collection<Element> getConfigXML() {
    ArrayList<Element> config = new ArrayList<Element>();
    Element element;

    if (userOverriddenStack != null) {
        element = new Element("stack");
        element.setText("0x" + Integer.toHexString(userOverriddenStack));
        config.add(element);
    }

    element = new Element("monitoring");
    element.setText("" + toggleButton.isSelected());
    config.add(element);

    return config;
  }

  public boolean setConfigXML(Collection<Element> configXML,
      boolean visAvailable) {
    for (Element element : configXML) {
        if (element.getName().equals("monitoring")) {
            boolean monitor = Boolean.parseBoolean(element.getText());
            if (monitor) {
              if (activate(false)) {
                toggleButton.setSelected(true);
              }
            }
        } else if (element.getName().equals("stack")) {
        	userOverriddenStack = Integer.decode(element.getText()); 
        }
    }
    return true;
  }

  public void closePlugin() {
    increasePosTimeEvent.remove();
    stackUI.stop();
    deactivate();
  }

  public Mote getMote() {
    return mspMote;
  }

}

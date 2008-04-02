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
 * $Id: LogListener.java,v 1.9 2008/04/02 16:42:39 fros4943 Exp $
 */

package se.sics.cooja.plugins;

import java.awt.BorderLayout;
import java.awt.Insets;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.io.*;
import java.util.*;
import javax.swing.*;
import org.apache.log4j.Logger;
import org.jdom.Element;

import se.sics.cooja.*;
import se.sics.cooja.interfaces.Log;

/**
 * A simple mote log listener.
 * When instantiated, is registers as a listener on all currently existing
 * motes' log interfaces. (Observe that if new motes are added to a simulation,
 * a new log listener must be created to listen to those motes also).
 *
 * @author Fredrik Osterlind
 */
@ClassDescription("Log Listener")
@PluginType(PluginType.SIM_PLUGIN)
public class LogListener extends VisPlugin {
  private static final long serialVersionUID = 1L;
  private static Logger logger = Logger.getLogger(LogListener.class);

  private JTextArea logTextArea;
  private Observer logObserver;
  private Simulation simulation;

  private String filterText = null;
  private JTextField filterTextField = null;

  /**
   * Create a new simulation control panel.
   *
   * @param simulationToControl Simulation to control
   */
  public LogListener(final Simulation simulationToControl, final GUI gui) {
    super("Log Listener - Listening on ?? mote logs", gui);
    simulation = simulationToControl;
    int nrLogs = 0;

    // Log observer
    logObserver = new Observer() {
      public void update(Observable obs, Object obj) {
        if (logTextArea == null) {
          return;
        }

        Mote mote = (Mote) obj;
        Log moteLogInterface = (Log) obs;
        String outputString = "TIME:" + simulation.getSimulationTime() + "\t";
        if (mote != null && mote.getInterfaces().getMoteID() != null) {
          outputString = outputString.concat("ID:" + mote.getInterfaces().getMoteID().getMoteID() + "\t");
        }
        // Match against filter (if any)
        if (filterText != null && !filterText.equals("") &&
            !moteLogInterface.getLastLogMessages().contains(filterText)) {
          return;
        }

        outputString = outputString.concat(moteLogInterface.getLastLogMessages());


        final String str = outputString;
        SwingUtilities.invokeLater(new Runnable() {
          public void run() {
            logTextArea.append("\n");
            logTextArea.append(str);
            logTextArea.setCaretPosition(logTextArea.getDocument().getLength());
          }
        });
      }
    };

    // Register as loglistener on all currently active motes
    for (int i=0; i < simulation.getMotesCount(); i++) {
      if (simulation.getMote(i).getInterfaces().getLog() != null) {
        simulation.getMote(i).getInterfaces().getLog().addObserver(logObserver);
        nrLogs++;
      }
    }

    simulation.addObserver(new Observer() {
      public void update(Observable obs, Object obj) {
        /* Reregister as log listener */
        int nrLogs = 0;
        for (int i=0; i < simulation.getMotesCount(); i++) {
          if (simulation.getMote(i).getInterfaces().getLog() != null) {
            simulation.getMote(i).getInterfaces().getLog().deleteObserver(logObserver);
            simulation.getMote(i).getInterfaces().getLog().addObserver(logObserver);
            nrLogs++;
          }
        }
        setTitle("Log Listener - Listening on " + nrLogs + " mote logs");
      }
    });

    // Main panel
    logTextArea = new JTextArea(8,50);
    logTextArea.setMargin(new Insets(5,5,5,5));
    logTextArea.setEditable(false);
    logTextArea.setCursor(null);

    JPanel filterPanel = new JPanel();
    filterPanel.setLayout(new BoxLayout(filterPanel, BoxLayout.X_AXIS));
    filterTextField = new JTextField("");
    filterPanel.add(Box.createVerticalStrut(2));
    filterPanel.add(new JLabel("Filter on string: "));
    filterPanel.add(filterTextField);
    filterTextField.addActionListener(new ActionListener() {
      public void actionPerformed(ActionEvent e) {
        filterText = filterTextField.getText();
      }
    });
    filterPanel.add(Box.createVerticalStrut(2));
    JButton saveButton;
    filterPanel.add(saveButton = new JButton("Save log"));
    saveButton.addActionListener(new ActionListener() {
      public void actionPerformed(ActionEvent ev) {
        JFileChooser fc = new JFileChooser();

        int returnVal = fc.showSaveDialog(GUI.getTopParentContainer());
        if (returnVal == JFileChooser.APPROVE_OPTION) {
          File saveFile = fc.getSelectedFile();

          if (saveFile.exists()) {
            String s1 = "Overwrite";
            String s2 = "Cancel";
            Object[] options = { s1, s2 };
            int n = JOptionPane.showOptionDialog(
                GUI.getTopParentContainer(),
                "A file with the same name already exists.\nDo you want to remove it?",
                "Overwrite existing file?", JOptionPane.YES_NO_OPTION,
                JOptionPane.QUESTION_MESSAGE, null, options, s1);
            if (n != JOptionPane.YES_OPTION) {
              return;
            }
          }

          if (!saveFile.exists() || saveFile.canWrite()) {
            try {
              BufferedWriter outStream = new BufferedWriter(
                  new OutputStreamWriter(
                      new FileOutputStream(
                          saveFile)));
              outStream.write(logTextArea.getText());
              outStream.close();
            } catch (Exception ex) {
              logger.fatal("Could not write to file: " + saveFile);
              return;
            }

          } else {
            logger.fatal("No write access to file");
          }
        }
      }
    });

    if (gui.isVisualizedInApplet()) {
      saveButton.setToolTipText("Not available in applet");
      saveButton.setEnabled(false);
    }

    getContentPane().add(BorderLayout.CENTER, new JScrollPane(logTextArea));
    getContentPane().add(BorderLayout.SOUTH, new JScrollPane(filterPanel));

    setTitle("Log Listener - Listening on " + nrLogs + " mote logs");
    pack();

    try {
      setSelected(true);
    } catch (java.beans.PropertyVetoException e) {
      // Could not select
    }

  }

  public void closePlugin() {
    // Remove log observer from all log interfaces
    for (int i=0; i < simulation.getMotesCount(); i++) {
      if (simulation.getMote(i).getInterfaces().getLog() != null) {
        simulation.getMote(i).getInterfaces().getLog().deleteObserver(logObserver);
      }
    }
  }

  public Collection<Element> getConfigXML() {
    Vector<Element> config = new Vector<Element>();

    Element element;

    // Selected variable name
    element = new Element("filter");
    element.setText(filterText);
    config.add(element);

    return config;
  }

  public boolean setConfigXML(Collection<Element> configXML, boolean visAvailable) {

    for (Element element : configXML) {
      if (element.getName().equals("filter")) {
        filterText = element.getText();
        filterTextField.setText(filterText);
      }
    }

    return true;
  }


}

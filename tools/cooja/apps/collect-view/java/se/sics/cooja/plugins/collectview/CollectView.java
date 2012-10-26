/*
 * Copyright (c) 2010, Swedish Institute of Computer Science.
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
package se.sics.cooja.plugins.collectview;
import java.awt.BorderLayout;
import java.awt.EventQueue;
import java.awt.GridBagConstraints;
import java.awt.GridBagLayout;
import java.io.BufferedReader;
import java.io.DataOutputStream;
import java.io.File;
import java.io.IOException;
import java.io.InputStreamReader;
import java.util.Collection;
import java.util.Observable;
import java.util.Observer;

import javax.swing.JLabel;
import javax.swing.JOptionPane;
import javax.swing.JPanel;

import org.apache.log4j.Logger;
import org.jdom.Element;

import se.sics.cooja.ClassDescription;
import se.sics.cooja.GUI;
import se.sics.cooja.HasQuickHelp;
import se.sics.cooja.Mote;
import se.sics.cooja.MotePlugin;
import se.sics.cooja.PluginType;
import se.sics.cooja.Simulation;
import se.sics.cooja.VisPlugin;
import se.sics.cooja.interfaces.SerialPort;

/**
 * Plugin to connect CollectView to COOJA.
 *
 * @author Niclas Finne
 */
@ClassDescription("Collect View")
@PluginType(PluginType.MOTE_PLUGIN)
public class CollectView extends VisPlugin implements MotePlugin, HasQuickHelp {
  private static final long serialVersionUID = 1L;

  private static Logger logger = Logger.getLogger(CollectView.class);

  private final Mote mote;
  private final SerialPort serialPort;
  private Observer serialDataObserver;

  private JLabel inLabel, outLabel;
  private int inBytes = 0, outBytes = 0;

  private Process commandProcess;
  private DataOutputStream out;
  private boolean isRunning;

  public CollectView(Mote mote, Simulation simulation, final GUI gui) {
    super("Collect View (" + mote + ")", gui, false);
    this.mote = mote;

    /* Mote serial port */
    serialPort = (SerialPort) mote.getInterfaces().getLog();
    if (serialPort == null) {
      throw new RuntimeException("No mote serial port");
    }

    /* GUI components */
    if (GUI.isVisualized()) {
      inLabel =  new JLabel("      0 bytes", JLabel.RIGHT);
      outLabel = new JLabel("      0 bytes", JLabel.RIGHT);

      JPanel panel = new JPanel(new GridBagLayout());
      GridBagConstraints c = new GridBagConstraints();
      c.fill = GridBagConstraints.HORIZONTAL;
      c.insets.left = c.insets.right = c.insets.top = 6;
      c.gridy = 0;

      panel.add(new JLabel("CollectView -> Mote:"), c);
      panel.add(inLabel, c);

      c.insets.bottom = 6;
      c.gridy++;
      panel.add(new JLabel("Mote -> CollectView:"), c);
      panel.add(outLabel, c);

      getContentPane().add(BorderLayout.NORTH, panel);
      pack();
    }

    String contikiPath = GUI.getExternalToolsSetting("PATH_CONTIKI", "../../..");
    String jarFile = contikiPath + "/tools/collect-view/dist/collect-view.jar";
    if (!new File(jarFile).canRead()) {
      logger.fatal("Could not find the CollectView application:" + jarFile);
      if (GUI.isVisualized()) {
        JOptionPane.showMessageDialog(GUI.getTopParentContainer(),
            "Could not find the CollectView application:\n" +
            jarFile + "\n\nPlease try to recompile it!",
            "CollectView application not found", JOptionPane.ERROR_MESSAGE);
      }
      // Could not find the CollectView application
      cleanup();
      return;
    }

    try {
      String[] cmd = new String[] {
          "java", "-jar", jarFile,
          "-n", "-f"
      };

      isRunning = true;
      commandProcess = Runtime.getRuntime().exec(cmd);
      final BufferedReader input = new BufferedReader(new InputStreamReader(commandProcess.getInputStream()));
      final BufferedReader err = new BufferedReader(new InputStreamReader(commandProcess.getErrorStream()));
      out = new DataOutputStream(commandProcess.getOutputStream());

      /* Start thread listening on standard out */
      Thread readInput = new Thread(new Runnable() {
        public void run() {
          String line;
          try {
            while ((line = input.readLine()) != null) {
              if (line.length() > 0) {
                System.err.println("CollectView Serial> " + line);
                for (int i = 0, n = line.length(); i < n; i++) {
                  serialPort.writeByte((byte) line.charAt(i));
                }
                serialPort.writeByte((byte) '\n');
                inBytes += line.length() + 1;
                if (GUI.isVisualized()) {
                  inLabel.setText(inBytes + " bytes");
                }
              }
            }
            input.close();
          } catch (IOException e) {
            if (isRunning) {
              logger.error("The CollectView application died!", e);
            }
          } finally {
            cleanup();
          }
        }
      }, "read input stream thread");

      /* Start thread listening on standard err */
      Thread readError = new Thread(new Runnable() {
        public void run() {
          String line;
          try {
            while ((line = err.readLine()) != null) {
              System.err.println("CollectView> " + line);
            }
            err.close();
          } catch (IOException e) {
            if (isRunning) {
              logger.error("The CollectView application died!", e);
            }
          }
        }
      }, "read error stream thread");

      readInput.start();
      readError.start();
    } catch (Exception e) {
      throw (RuntimeException) new RuntimeException(
          "CollectView error: " + e.getMessage()).initCause(e);
    }

    /* Observe serial port for outgoing data */
    serialPort.addSerialDataObserver(serialDataObserver = new Observer() {
      public void update(Observable obs, Object obj) {
        DataOutputStream out = CollectView.this.out;
        if (out != null) {
          try {
            byte b = serialPort.getLastSerialData();
            out.write(b);
            outBytes++;
            if (b == '\n') {
              out.flush();
              if (GUI.isVisualized()) {
                outLabel.setText(outBytes + " bytes");
              }
            }
          } catch (IOException e) {
            if (isRunning) {
              logger.warn("CollectView output error", e);
            }
          }
        }
      }
    });
  }

  public boolean setConfigXML(Collection<Element> configXML, boolean visAvailable) {
    return true;
  }

  public Collection<Element> getConfigXML() {
    return null;
  }

  private void cleanup() {
    if (serialDataObserver != null) {
      serialPort.deleteSerialDataObserver(serialDataObserver);
      serialDataObserver = null;
    }

    if (isRunning) {
      logger.fatal("The CollectView application died!");
      if (GUI.isVisualized()) {
        JOptionPane.showMessageDialog(this, "The CollectView application died!",
            "CollectView died!", JOptionPane.ERROR_MESSAGE);
      }
    }
    isRunning = false;
    if (commandProcess != null) {
      commandProcess.destroy();
      commandProcess = null;
    }
    if (out != null) {
      try {
        out.close();
        out = null;
      } catch (IOException e) {
      }
    }

    if (GUI.isVisualized()) {
      EventQueue.invokeLater(new Runnable() {
        public void run() {
          setTitle(getTitle() + " *DISCONNECTED*");
          inLabel.setEnabled(false);
          outLabel.setEnabled(false);
        }
      });
    }
  }

  public void closePlugin() {
    isRunning = false;
    cleanup();
  }

  public Mote getMote() {
    return mote;
  }

  public String getQuickHelp() {
    return "<b>CollectView</b><p>" +
    "The CollectView plugin starts an instance of the CollectView application and " +
    "connects it to the serial port of a specific mote. " +
    "The CollectView application can then interact with the simulated network in " +
    "exactly the same way as it would interact with a network of real motes.";
  }

}

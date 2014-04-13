package org.contikios.cooja.serialsocket;

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

import java.awt.BorderLayout;
import java.awt.Color;
import java.awt.Dimension;
import java.awt.GridBagConstraints;
import java.awt.GridBagLayout;
import java.awt.GridLayout;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.io.DataInputStream;
import java.io.DataOutputStream;
import java.io.IOException;
import java.net.ServerSocket;
import java.net.Socket;
import java.text.NumberFormat;
import java.util.Collection;
import java.util.Observable;
import java.util.Observer;

import javax.swing.BorderFactory;
import javax.swing.BoxLayout;
import javax.swing.JButton;
import javax.swing.JFormattedTextField;
import javax.swing.JLabel;
import javax.swing.JPanel;
import javax.swing.JSeparator;
import javax.swing.SwingUtilities;
import javax.swing.Timer;
import javax.swing.border.EtchedBorder;
import javax.swing.text.NumberFormatter;

import org.apache.log4j.Logger;
import org.jdom.Element;

import org.contikios.cooja.ClassDescription;
import org.contikios.cooja.Cooja;
import org.contikios.cooja.Mote;
import org.contikios.cooja.MotePlugin;
import org.contikios.cooja.PluginType;
import org.contikios.cooja.Simulation;
import org.contikios.cooja.VisPlugin;
import org.contikios.cooja.interfaces.SerialPort;

/**
 * Socket to simulated serial port forwarder. Server version.
 * 
 * @author Fredrik Osterlind
 */
@ClassDescription("Serial Socket (SERVER)")
@PluginType(PluginType.MOTE_PLUGIN)
public class SerialSocketServer extends VisPlugin implements MotePlugin {
  private static final long serialVersionUID = 1L;
  private static Logger logger = Logger.getLogger(SerialSocketServer.class);

  private final static int STATUSBAR_WIDTH = 350;

  public final int LISTEN_PORT;

  private SerialPort serialPort;
  private Observer serialDataObserver;

  private JLabel statusLabel, inLabel, outLabel;
  private JButton serverStartButton;
  private int inBytes = 0, outBytes = 0;

  private ServerSocket server;
  private Socket client;
  private DataInputStream in;
  private DataOutputStream out;

  private Mote mote;

  public SerialSocketServer(Mote mote, Simulation simulation, final Cooja gui) {
    super("Serial Socket (SERVER) (" + mote + ")", gui, false);
    this.mote = mote;

    updateTimer.start();

    LISTEN_PORT = 60000 + mote.getID();

    /* GUI components */
    if (Cooja.isVisualized()) {

      setResizable(false);
      setLayout(new BorderLayout());

      // --- Server Port setup
      
      GridBagConstraints c = new GridBagConstraints();
      JPanel socketPanel = new JPanel(new GridBagLayout());
      socketPanel.setBorder(BorderFactory.createEmptyBorder(2, 2, 2, 2));
      
      JLabel label = new JLabel("Listen port: ");
      c.gridx = 0;
      c.gridy = 0;
      socketPanel.add(label, c);
      
      NumberFormat nf = NumberFormat.getIntegerInstance();
      nf.setGroupingUsed(false);
      final JFormattedTextField serverPortField = new JFormattedTextField(new NumberFormatter(nf));
      serverPortField.setColumns(5);
      serverPortField.setText(String.valueOf(LISTEN_PORT));
      c.gridx++;
      socketPanel.add(serverPortField, c);

      serverStartButton = new JButton("Start");
      c.gridx++;
      c.weightx = 0.1;
      c.anchor = GridBagConstraints.EAST;
      socketPanel.add(serverStartButton, c);

      c.gridx = 0;
      c.gridy++;
      c.gridwidth = GridBagConstraints.REMAINDER;
      c.fill = GridBagConstraints.HORIZONTAL;
      socketPanel.add(new JSeparator(JSeparator.HORIZONTAL), c);
      
      add(BorderLayout.NORTH, socketPanel);
      
      // --- Incoming / outgoing info

      JPanel connectionInfoPanel = new JPanel(new GridLayout(0, 2));
      connectionInfoPanel.setBorder(BorderFactory.createEmptyBorder(5, 5, 5, 5));
      c = new GridBagConstraints();

      label = new JLabel("socket -> mote: ");
      label.setHorizontalAlignment(JLabel.RIGHT);
      c.gridx = 0;
      c.gridy = 0;
      c.anchor = GridBagConstraints.EAST;
      connectionInfoPanel.add(label);

      inLabel = new JLabel("0 bytes");
      c.gridx++;
      c.anchor = GridBagConstraints.WEST;
      connectionInfoPanel.add(inLabel);

      label = new JLabel("mote -> socket: ");
      label.setHorizontalAlignment(JLabel.RIGHT);
      c.gridx = 0;
      c.gridy++;
      c.anchor = GridBagConstraints.EAST;
      connectionInfoPanel.add(label);

      outLabel = new JLabel("0 bytes");
      c.gridx++;
      c.anchor = GridBagConstraints.WEST;
      connectionInfoPanel.add(outLabel);

      add(BorderLayout.CENTER, connectionInfoPanel);

      // --- Status bar
      
      JPanel statusBarPanel = new JPanel(new BorderLayout()) {
        @Override
        public Dimension getPreferredSize() {
          Dimension d = super.getPreferredSize();
          return new Dimension(STATUSBAR_WIDTH, d.height);
        }
      };
      statusBarPanel.setLayout(new BoxLayout(statusBarPanel, BoxLayout.LINE_AXIS));
      statusBarPanel.setBorder(BorderFactory.createEtchedBorder(EtchedBorder.RAISED));
      label = new JLabel("Status: ");
      statusBarPanel.add(label);
      
      statusLabel = new JLabel("Not started");
      statusLabel.setForeground(Color.DARK_GRAY);
      statusBarPanel.add(statusLabel);
      
      add(BorderLayout.SOUTH, statusBarPanel);

      serverStartButton.addActionListener(new ActionListener() {

        @Override
        public void actionPerformed(ActionEvent e) {
          // XXX
        }
      });
      
      pack();
    }

    /* Mote serial port */
    serialPort = (SerialPort) mote.getInterfaces().getLog();
    if (serialPort == null) {
      throw new RuntimeException("No mote serial port");
    }

    try {
      logger.info("Listening on port: " + LISTEN_PORT);
      if (Cooja.isVisualized()) {
        statusLabel.setText("Listening on port: " + LISTEN_PORT);
      }
      server = new ServerSocket(LISTEN_PORT);
      new Thread() {
        @Override
        public void run() {
          while (server != null) {
            try {
              client = server.accept();
              in = new DataInputStream(client.getInputStream());
              out = new DataOutputStream(client.getOutputStream());
              out.flush();

              startSocketReadThread(in);
              if (Cooja.isVisualized()) {
                statusLabel.setText("Client connected: " + client.getInetAddress());
              }
            } catch (IOException e) {
              logger.fatal("Listening thread shut down: " + e.getMessage());
              server = null;
              cleanupClient();
              break;
            }
          }
        }
      }.start();
    } catch (Exception e) {
      throw (RuntimeException) new RuntimeException(
          "Connection error: " + e.getMessage()).initCause(e);
    }

    /* Observe serial port for outgoing data */
    serialPort.addSerialDataObserver(serialDataObserver = new Observer() {
      @Override
      public void update(Observable obs, Object obj) {
        try {
          if (out == null) {
            /*logger.debug("out is null");*/
            return;
          }
          
          out.write(serialPort.getLastSerialData());
          out.flush();
          
          outBytes++;
        } catch (IOException e) {
          cleanupClient();
        }
      }
    });
  }

  private void startSocketReadThread(final DataInputStream in) {
    /* Forward data: virtual port -> mote */
    Thread incomingDataThread = new Thread(new Runnable() {
      @Override
      public void run() {
        int numRead = 0;
        byte[] data = new byte[1024];
        logger.info("Forwarder: socket -> serial port");
        while (true) {
          numRead = -1;
          try {
            numRead = in.read(data);
          } catch (IOException e) {
            numRead = -1;
          }

          if (numRead >= 0) {
            for (int i=0; i < numRead; i++) {
              serialPort.writeByte(data[i]);
            }

            inBytes += numRead;
          } else {
            cleanupClient();
            break;
          }
        }
      }
    });
    incomingDataThread.start();
  }

  @Override
  public boolean setConfigXML(Collection<Element> configXML, boolean visAvailable) {
    return true;
  }

  @Override
  public Collection<Element> getConfigXML() {
    return null;
  }

  private void cleanupClient() {
    try {
      if (client != null) {
        client.close();
        client = null;
      }
    } catch (IOException e1) {
    }
    try {
      if (in != null) {
        in.close();
        in = null;
      }
    } catch (IOException e) {
    }
    try {
      if (out != null) {
        out.close();
        out = null;
      }
    } catch (IOException e) {
    }

    if (Cooja.isVisualized()) {
      SwingUtilities.invokeLater(new Runnable() {
        @Override
        public void run() {
          statusLabel.setText("Listening on port: " + LISTEN_PORT);
        }
      });
    }
  }

  private boolean closed = false;
  @Override
  public void closePlugin() {
	  closed = true;
    cleanupClient();
    serialPort.deleteSerialDataObserver(serialDataObserver);
    try {
      server.close();
    } catch (IOException e) {
    }
  }

  @Override
  public Mote getMote() {
    return mote;
  }

  private static final int UPDATE_INTERVAL = 150;
  private Timer updateTimer = new Timer(UPDATE_INTERVAL, new ActionListener() {
    @Override
	  public void actionPerformed(ActionEvent e) {
		  if (closed) {
			  updateTimer.stop();
			  return;
		  }
		  
		  inLabel.setText(inBytes + " bytes");
		  outLabel.setText(outBytes + " bytes");
	  }
  });
}


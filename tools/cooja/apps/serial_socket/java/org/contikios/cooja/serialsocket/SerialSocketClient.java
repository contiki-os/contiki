package org.contikios.cooja.serialsocket;

/*
 * Copyright (c) 2014, TU Braunschweig.
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
import java.net.Socket;
import java.text.NumberFormat;
import java.text.ParseException;
import java.util.ArrayList;
import java.util.Collection;
import java.util.LinkedList;
import java.util.List;
import java.util.Observable;
import java.util.Observer;
import java.util.logging.Level;

import javax.swing.BorderFactory;
import javax.swing.BoxLayout;
import javax.swing.JButton;
import javax.swing.JFormattedTextField;
import javax.swing.JLabel;
import javax.swing.JPanel;
import javax.swing.JSeparator;
import javax.swing.JTextField;
import javax.swing.SwingUtilities;
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
 * Socket to simulated serial port forwarder. Client version.
 * 
 * @author Fredrik Osterlind
 * @author Enrico Jorns
 */
@ClassDescription("Serial Socket (CLIENT)")
@PluginType(PluginType.MOTE_PLUGIN)
public class SerialSocketClient extends VisPlugin implements MotePlugin {
  private static final long serialVersionUID = 1L;
  private static final Logger logger = Logger.getLogger(SerialSocketClient.class);

  private static final String SERVER_DEFAULT_HOST = "localhost";
  private static final int SERVER_DEFAULT_PORT = 1234;
  
  private final static int STATUSBAR_WIDTH = 350;
  
  private static final Color ST_COLOR_UNCONNECTED = Color.DARK_GRAY;
  private static final Color ST_COLOR_CONNECTED = new Color(0, 161, 83);
  private static final Color ST_COLOR_FAILED = Color.RED;
  
  private SerialPort serialPort;
  private Observer serialDataObserver;

  private JLabel socketToMoteLabel;
  private JLabel moteToSocketLabel;
  private JLabel socketStatusLabel;
  private JTextField serverHostField;
  private JFormattedTextField serverPortField;
  private JButton serverSelectButton;
  
  private int inBytes = 0, outBytes = 0;

  private Socket socket;
  private DataInputStream in;
  private DataOutputStream out;

  private final Mote mote;
  private final Simulation simulation;

  public SerialSocketClient(Mote mote, Simulation simulation, final Cooja gui) {
    super("Serial Socket (CLIENT) (" + mote + ")", gui, false);
    this.mote = mote;
    this.simulation = simulation;

    /* GUI components */
    if (Cooja.isVisualized()) {

      setResizable(false);
      setLayout(new BorderLayout());
      
      // --- Server setup
      
      GridBagConstraints c = new GridBagConstraints();
      JPanel serverSelectPanel = new JPanel(new GridBagLayout());
      serverSelectPanel.setBorder(BorderFactory.createEmptyBorder(2, 2, 2, 2));

      JLabel label = new JLabel("Host:");
      c.gridx = 0;
      c.gridy = 0;
      c.gridx++;
      serverSelectPanel.add(label, c);
      
      serverHostField = new JTextField(SERVER_DEFAULT_HOST);
      serverHostField.setColumns(10);
      c.gridx++;
      c.weightx = 1.0;
      serverSelectPanel.add(serverHostField, c);
      
      label = new JLabel("Port:");
      c.gridx++;
      c.weightx = 0.0;
      serverSelectPanel.add(label, c);

      NumberFormat nf = NumberFormat.getIntegerInstance();
      nf.setGroupingUsed(false);
      serverPortField = new JFormattedTextField(new NumberFormatter(nf));
      serverPortField.setColumns(5);
      serverPortField.setText(String.valueOf(SERVER_DEFAULT_PORT));
      c.gridx++;
      serverSelectPanel.add(serverPortField, c);

      serverSelectButton = new JButton("Connect") { // Button for label toggeling
        private final String altString = "Disconnect";
        
        @Override
        public Dimension getPreferredSize() {
          String origText = getText();
          Dimension origDim = super.getPreferredSize();
          setText(altString);
          Dimension altDim = super.getPreferredSize();
          setText(origText);
          return new Dimension(Math.max(origDim.width, altDim.width), origDim.height);
        }
      };
      c.gridx++;
      c.weightx = 0.1;
      c.anchor = GridBagConstraints.EAST;
      serverSelectPanel.add(serverSelectButton, c);

      c.gridx = 0;
      c.gridy++;
      c.gridwidth = GridBagConstraints.REMAINDER;
      c.fill = GridBagConstraints.HORIZONTAL;
      serverSelectPanel.add(new JSeparator(JSeparator.HORIZONTAL), c);
      
      add(BorderLayout.NORTH, serverSelectPanel);
      
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

      socketToMoteLabel = new JLabel("0 bytes");
      c.gridx++;
      c.anchor = GridBagConstraints.WEST;
      connectionInfoPanel.add(socketToMoteLabel);

      label = new JLabel("mote -> socket: ");
      label.setHorizontalAlignment(JLabel.RIGHT);
      c.gridx = 0;
      c.gridy++;
      c.anchor = GridBagConstraints.EAST;
      connectionInfoPanel.add(label);

      moteToSocketLabel = new JLabel("0 bytes");
      c.gridx++;
      c.anchor = GridBagConstraints.WEST;
      connectionInfoPanel.add(moteToSocketLabel);

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
      
      socketStatusLabel = new JLabel("Disconnected");
      socketStatusLabel.setForeground(Color.DARK_GRAY);
      statusBarPanel.add(socketStatusLabel);
      
      add(BorderLayout.SOUTH, statusBarPanel);

      /* Mote serial port */
      serialPort = (SerialPort) mote.getInterfaces().getLog();
      if (serialPort == null) {
        throw new RuntimeException("No mote serial port");
      }

      serverSelectButton.addActionListener(new ActionListener() {

        @Override
        public void actionPerformed(ActionEvent e) {
          if (e.getActionCommand().equals("Connect")) {
            try {
              serverPortField.commitEdit();
              startClient(serverHostField.getText(), ((Long) serverPortField.getValue()).intValue());
            } catch (ParseException ex) {
              logger.error(ex);
            }
          } else {
            // close socket
            cleanup();
          }
        }
      });
      

      /* Observe serial port for outgoing data and write to socket */
      serialPort.addSerialDataObserver(serialDataObserver = new Observer() {
        @Override
        public void update(Observable obs, Object obj) {
          try {
            if (out == null) {
              return;
            }
            out.write(serialPort.getLastSerialData());
            out.flush();
            outBytes++;
            if (Cooja.isVisualized()) {
              moteToSocketLabel.setText(outBytes + " bytes");
            }
          } catch (IOException ex) {
            logger.error(ex.getMessage());
            socketStatusLabel.setText("failed");
            socketStatusLabel.setForeground(ST_COLOR_FAILED);
          }
        }
      });
    }

    if (Cooja.isVisualized()) {
      addClientListener(new ClientListener() {

        @Override
        public void onError(final String msg) {
          SwingUtilities.invokeLater(new Runnable() {
            @Override
            public void run() {
              socketStatusLabel.setForeground(ST_COLOR_FAILED);
              socketStatusLabel.setText(msg);
            }
          });
        }

        @Override
        public void onConnected() {
          SwingUtilities.invokeLater(new Runnable() {
            @Override
            public void run() {
              socketStatusLabel.setText("Connected");
              socketStatusLabel.setForeground(ST_COLOR_CONNECTED);
              serverHostField.setEnabled(false);
              serverPortField.setEnabled(false);
              serverSelectButton.setText("Disconnect");
            }
          });
        }

        @Override
        public void onDisconnected() {
          SwingUtilities.invokeLater(new Runnable() {
            @Override
            public void run() {
              socketStatusLabel.setForeground(ST_COLOR_UNCONNECTED);
              socketStatusLabel.setText("Disconnected");
              serverHostField.setEnabled(true);
              serverPortField.setEnabled(true);
              serverSelectButton.setText("Connect");
            }
          });
        }
      });
    }
    pack();
  }
  
  private List<ClientListener> listeners = new LinkedList<>();
  
  public interface ClientListener {
    void onError(String msg);
    void onConnected();
    void onDisconnected();
  }
  
  public void addClientListener(ClientListener listener) {
    if (!listeners.contains(listener)) {
      listeners.add(listener);
    }
  }
  
  private void notifyClientError(String msg) {
    for (ClientListener l : listeners) {
      l.onError(msg);
    }
  }
  
  private void notifyClientConnected() {
    for (ClientListener l : listeners) {
      l.onConnected();
    }
  }
  
  private void notifyClientDisconnected() {
    for (ClientListener l : listeners) {
      l.onDisconnected();
    }
  }
  
  
  public void startClient(String host, int port) {
    if (socket == null) {
      // connect to serer
      try {
        logger.info("Connecting: " + host + ":" + port);
        socket = new Socket(host, port);
        in = new DataInputStream(socket.getInputStream());
        out = new DataOutputStream(socket.getOutputStream());
        out.flush();
        startSocketReadThread(in);
        notifyClientConnected();
      } catch (IOException ex) {
        logger.error(ex.getMessage());
        notifyClientError(ex.getMessage());
      }
    } else {
      // disconnect from server
      try {
        logger.info("Closing connection to serer...");
        socket.close();
        notifyClientDisconnected();
      } catch (IOException ex) {
        logger.error(ex);
        notifyClientError(ex.getMessage());
      } finally {
        socket = null;
      }
    }

  }

  private void startSocketReadThread(final DataInputStream in) {
    /* Forward data: virtual port -> mote */
    Thread incomingDataThread = new Thread(new Runnable() {
      @Override
      public void run() {
        int numRead = 0;
        byte[] data = new byte[1024];
        logger.info("Start forwarding: socket -> serial port");
        while (numRead >= 0) {
          try {
            numRead = in.read(data);
          } catch (IOException e) {
            logger.info(e.getMessage());
            numRead = -1;
            continue;
          }

          if (numRead >= 0) {
            final int finalNumRead = numRead;
            final byte[] finalData = data;
            /* We are not on the simulation thread */
            simulation.invokeSimulationThread(new Runnable() {

              @Override
              public void run() {
                for (int i = 0; i < finalNumRead; i++) {
                  serialPort.writeByte(finalData[i]);
                }
              }
            });

            inBytes += numRead;
            if (Cooja.isVisualized()) {
              socketToMoteLabel.setText(inBytes + " bytes");
            }
          }
        }

        logger.info("Incoming data thread shut down");
        cleanup();
        notifyClientDisconnected();
      }
    });
    incomingDataThread.start();
  }
  
  @Override
  public Collection<Element> getConfigXML() {
    List<Element> config = new ArrayList<>();
    Element element;
    
    // XXX isVisualized guards?
    element = new Element("host");
    if (socket == null || !socket.isBound()) {
      element.setText(serverHostField.getText());
    } else {
      element.setText(socket.getInetAddress().getHostName());
    }
    config.add(element);

    element = new Element("port");
    if (socket == null || !socket.isBound()) {
      try {
        serverPortField.commitEdit();
        element.setText(String.valueOf((Long) serverPortField.getValue()));
      } catch (ParseException ex) {
        logger.error(ex.getMessage());
        serverPortField.setText("null");
      }
    } else {
      element.setText(String.valueOf(socket.getPort()));
    }
    config.add(element);

    element = new Element("bound");
    if (socket == null) {
      element.setText(String.valueOf(false));
    } else {
      element.setText(String.valueOf(socket.isBound()));
    }
    config.add(element);

    return config;
  }

  @Override
  public boolean setConfigXML(Collection<Element> configXML, boolean visAvailable) {
    String host = null;
    Integer port = null;
    boolean bound = false;
    
    for (Element element : configXML) {
      switch (element.getName()) {
        case "host":
          host = element.getText();
          break;
        case "port":
          port = Integer.parseInt(element.getText());
          break;
        case "bound":
          bound = Boolean.parseBoolean(element.getText());
          break;
        default:
          logger.warn("Unknwon config element: " + element.getName());
          break;
      }
    }
    
    // XXX binding might fail if server not configured yet
    if (Cooja.isVisualized()) {
      if (host != null) {
        serverHostField.setText(host);
      }
      if (port != null) {
        serverPortField.setText(String.valueOf(port));
      }
      if (bound) {
        serverSelectButton.doClick();
      }
    } else {
      // if bound and all set up, start client
      if (host != null && port != null) {
        startClient(host, port);
      } else {
        logger.error("Client not started due to incomplete configuration");
      }
    }

    return true;
  }


  private void cleanup() {
    serialPort.deleteSerialDataObserver(serialDataObserver);

    try {
      if (socket != null) {
        socket.close();
        socket = null;
      }
    } catch (IOException e1) {
      logger.warn(e1.getMessage());
    }
    try {
      if (in != null) {
        in.close();
        in = null;
      }
    } catch (IOException e) {
      logger.warn(e.getMessage());
    }
    try {
      if (out != null) {
        out.close();
        out = null;
      }
    } catch (IOException e) {
      logger.warn(e.getMessage());
    }
  }

  @Override
  public void closePlugin() {
    cleanup();
  }

  @Override
  public Mote getMote() {
    return mote;
  }
}


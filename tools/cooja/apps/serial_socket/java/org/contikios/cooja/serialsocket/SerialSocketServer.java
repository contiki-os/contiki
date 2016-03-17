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
import java.net.ServerSocket;
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
 * @author Enrico Jorns
 */
@ClassDescription("Serial Socket (SERVER)")
@PluginType(PluginType.MOTE_PLUGIN)
public class SerialSocketServer extends VisPlugin implements MotePlugin {
  private static final long serialVersionUID = 1L;
  private static final Logger logger = Logger.getLogger(SerialSocketServer.class);

  private final static int STATUSBAR_WIDTH = 350;

  private static final Color COLOR_NEUTRAL = Color.DARK_GRAY;
  private static final Color COLOR_POSITIVE = new Color(0, 161, 83);
  private static final Color COLOR_NEGATIVE = Color.RED;
  
  private final int SERVER_DEFAULT_PORT;

  private final SerialPort serialPort;
  private Observer serialDataObserver;

  private JLabel socketToMoteLabel;
  private JLabel moteToSocketLabel;
  private JLabel socketStatusLabel;
  private JFormattedTextField listenPortField;
  private JButton serverStartButton;

  private int inBytes = 0, outBytes = 0;

  private ServerSocket serverSocket;
  private Socket clientSocket;

  private Mote mote;
  private Simulation simulation;

  public SerialSocketServer(Mote mote, Simulation simulation, final Cooja gui) {
    super("Serial Socket (SERVER) (" + mote + ")", gui, false);
    this.mote = mote;
    this.simulation = simulation;

    updateTimer.start();

    SERVER_DEFAULT_PORT = 60000 + mote.getID();

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
      c.weightx = 0.1;
      c.anchor = GridBagConstraints.EAST;
      socketPanel.add(label, c);
      
      NumberFormat nf = NumberFormat.getIntegerInstance();
      nf.setGroupingUsed(false);
      listenPortField = new JFormattedTextField(new NumberFormatter(nf));
      listenPortField.setColumns(5);
      listenPortField.setText(String.valueOf(SERVER_DEFAULT_PORT));
      c.gridx++;
      c.weightx = 0.0;
      socketPanel.add(listenPortField, c);

      serverStartButton = new JButton("Start") { // Button for label toggeling
        private final String altString = "Stop";
        
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
      
      socketStatusLabel = new JLabel("Idle");
      socketStatusLabel.setForeground(Color.DARK_GRAY);
      statusBarPanel.add(socketStatusLabel);
      
      add(BorderLayout.SOUTH, statusBarPanel);

      serverStartButton.addActionListener(new ActionListener() {

        @Override
        public void actionPerformed(ActionEvent e) {
          if (e.getActionCommand().equals("Start")) {
            try {
              listenPortField.commitEdit();
            } catch (ParseException ex) {
              java.util.logging.Logger.getLogger(SerialSocketClient.class.getName()).log(Level.SEVERE, null, ex);
            }
            startServer(((Long) listenPortField.getValue()).intValue());
          } else {
            stopServer();
          }
        }
      });
      
      pack();
    }

    /* Mote serial port */
    serialPort = (SerialPort) mote.getInterfaces().getLog();
    if (serialPort == null) {
      throw new RuntimeException("No mote serial port");
    }

    if (Cooja.isVisualized()) {
      // gui updates for server status updates
      addServerListener(new ServerListener() {

        @Override
        public void onServerStarted(final int port) {
          SwingUtilities.invokeLater(new Runnable() {

            @Override
            public void run() {
              System.out.println("onServerStarted");
              socketStatusLabel.setForeground(COLOR_NEUTRAL);
              socketStatusLabel.setText("Listening on port " + String.valueOf(port));
              listenPortField.setEnabled(false);
              serverStartButton.setText("Stop");
            }
          });
        }

        @Override
        public void onClientConnected(final Socket client) {
          SwingUtilities.invokeLater(new Runnable() {

            @Override
            public void run() {
              socketStatusLabel.setForeground(COLOR_POSITIVE);
              socketStatusLabel.setText("Client "
                      + client.getInetAddress() + ":" + client.getPort()
                      + " connected.");
            }
          });
        }

        @Override
        public void onClientDisconnected() {
          SwingUtilities.invokeLater(new Runnable() {

            @Override
            public void run() {
              // XXX check why needed
              if (serverSocket != null) {
                socketStatusLabel.setForeground(COLOR_NEUTRAL);
                socketStatusLabel.setText("Listening on port " + String.valueOf(serverSocket.getLocalPort()));
              }
            }
          });
        }

        @Override
        public void onServerStopped() {
          SwingUtilities.invokeLater(new Runnable() {

            @Override
            public void run() {
              listenPortField.setEnabled(true);
              serverStartButton.setText("Start");
              socketStatusLabel.setForeground(COLOR_NEUTRAL);
              socketStatusLabel.setText("Idle");
            }
          });
        }

        @Override
        public void onServerError(final String msg) {
          SwingUtilities.invokeLater(new Runnable() {

            @Override
            public void run() {
              socketStatusLabel.setForeground(COLOR_NEGATIVE);
              socketStatusLabel.setText(msg);
            }
          });
        }

      });
    }

  }

  private List<ServerListener> listeners = new LinkedList<>();
  
  public interface ServerListener {
    void onServerStarted(int port);
    void onClientConnected(Socket client);
    void onClientDisconnected();
    void onServerStopped();
    void onServerError(String msg);
  }
  
  private void addServerListener(ServerListener listener) {
    listeners.add(listener);
  }
  
  public void notifyServerStarted(int port) {
    for (ServerListener listener : listeners) {
      listener.onServerStarted(port);
    }
  }
  
  public void notifyClientConnected(Socket client) {
    for (ServerListener listener : listeners) {
      listener.onClientConnected(client);
    }
  }

  public void notifyClientDisconnected() {
    for (ServerListener listener : listeners) {
      listener.onClientDisconnected();
    }
  }
  
  public void notifyServerStopped() {
    for (ServerListener listener : listeners) {
      listener.onServerStopped();
    }
  }
  
  public void notifyServerError(String msg) {
    for (ServerListener listener : listeners) {
      listener.onServerError(msg);
    }
  }
  
  /**
   * Start server ..
   * @param port 
   */
  public void startServer(int port) {
    try {
      serverSocket = new ServerSocket(port);
      logger.info("Listening on port: " + port);
      notifyServerStarted(port);
    } catch (IOException ex) {
      logger.error(ex.getMessage());
      notifyServerError(ex.getMessage());
      return;
    }

    new Thread() {
      private Thread incomingDataHandler;
      @Override
      public void run() {
        while (!serverSocket.isClosed()) {
          try {
            // wait for next client
            Socket candidateSocket = serverSocket.accept();

            // reject connection if already one client connected
            if (clientSocket != null && !clientSocket.isClosed()) {
              logger.info("Refused connection of client " + candidateSocket.getInetAddress());
              candidateSocket.close();
              continue;
            }

            clientSocket = candidateSocket;

            /* Start handler for data input from socket */
            incomingDataHandler = new Thread(new IncomingDataHandler());
            incomingDataHandler.start();

            /* Observe serial port for outgoing data */
            serialDataObserver = new SerialDataObserver();
            serialPort.addSerialDataObserver(serialDataObserver);

            inBytes = outBytes = 0;

            logger.info("Client connected: " + clientSocket.getInetAddress());
            notifyClientConnected(clientSocket);

          } catch (IOException e) {
            logger.info("Listening thread shut down: " + e.getMessage());
            try {
              serverSocket.close();
            } catch (IOException ex) {
              logger.error(ex);
            }
          }
        }
        cleanupClient();
        if (incomingDataHandler != null) {
          // Wait for reader thread to terminate
          try {
            incomingDataHandler.join(500);
          } catch (InterruptedException ex) {
            logger.warn(ex);
          }
        }
        notifyServerStopped();
      }
    }.start();
  }

  /**
   * Stops server by closing server listen socket.
   */
  public void stopServer() {
    try {
      serverSocket.close();
    } catch (IOException ex) {
      logger.error(ex);
    }
  }

  /* Forward data: virtual port -> mote */
  private class IncomingDataHandler implements Runnable {

    DataInputStream in;

    @Override
    public void run() {
      int numRead = 0;
      byte[] data = new byte[1024];
      try {
        in = new DataInputStream(clientSocket.getInputStream());
      } catch (IOException ex) {
        logger.error(ex);
        return;
      }

      logger.info("Forwarder: socket -> serial port");
      while (numRead >= 0) {
        final int finalNumRead = numRead;
        final byte[] finalData = data;
        /* We are not on the simulation thread */
        simulation.invokeSimulationThread(new Runnable() {

          @Override
          public void run() {
            for (int i = 0; i < finalNumRead; i++) {
              serialPort.writeByte(finalData[i]);
            }
            inBytes += finalNumRead;
          }
        });

        try {
          numRead = in.read(data);
        } catch (IOException e) {
          logger.info(e.getMessage());
          numRead = -1;
        }
      }
      logger.info("End of Stream");
      cleanupClient();
    }
  }

  private class SerialDataObserver implements Observer {
    
    DataOutputStream out;

    public SerialDataObserver() {
      try {
        out = new DataOutputStream(clientSocket.getOutputStream());
      } catch (IOException ex) {
        logger.error(ex);
        out = null;
      }
    }

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
      } catch (IOException ex) {
        logger.error(ex);
        cleanupClient();
      }
    }

  }

  @Override
  public Collection<Element> getConfigXML() {
    List<Element> config = new ArrayList<>();
    Element element;
    
    // XXX isVisualized guards?

    element = new Element("port");
    if (serverSocket == null || !serverSocket.isBound()) {
      try {
        listenPortField.commitEdit();
        element.setText(String.valueOf((Long) listenPortField.getValue()));
      } catch (ParseException ex) {
        logger.error(ex.getMessage());
        listenPortField.setText("null");
      }
    } else {
      element.setText(String.valueOf(serverSocket.getLocalPort()));
    }
    config.add(element);

    element = new Element("bound");
    if (serverSocket == null) {
      element.setText(String.valueOf(false));
    } else {
      element.setText(String.valueOf(!serverSocket.isClosed()));
    }
    config.add(element);

    return config;
  }

  @Override
  public boolean setConfigXML(Collection<Element> configXML, boolean visAvailable) {
    Integer port = null;
    boolean bound = false;
    
    for (Element element : configXML) {
      switch (element.getName()) {
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
    if (Cooja.isVisualized()) {
      if (port != null) {
        listenPortField.setText(String.valueOf(port));
      }
      if (bound) {
        serverStartButton.doClick();
      }
    } else {
      // if bound and all set up, start client
      if (port != null) {
        startServer(port);
      } else {
        logger.error("Server not started due to incomplete configuration");
      }
    }

    return true;
  }

  private void cleanupClient() {
    try {
      if (clientSocket != null) {
        clientSocket.close();
        clientSocket = null;
      }
    } catch (IOException e1) {
      logger.error(e1.getMessage());
    }

    serialPort.deleteSerialDataObserver(serialDataObserver);

    notifyClientDisconnected();
  }

  private boolean closed = false;

  @Override
  public void closePlugin() {
    closed = true;
    cleanupClient();
    try {
      if (serverSocket != null) {
        serverSocket.close();
      }
    } catch (IOException ex) {
      logger.error(ex);
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
	  	if (Cooja.isVisualized()) {
		  if (closed) {
			  updateTimer.stop();
			  return;
		  }
		  
		  socketToMoteLabel.setText(inBytes + " bytes");
		  moteToSocketLabel.setText(outBytes + " bytes");
	  }
	  }
  });
}


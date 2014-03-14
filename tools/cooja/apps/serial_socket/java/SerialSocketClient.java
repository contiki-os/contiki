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
import java.awt.Dimension;
import java.io.DataInputStream;
import java.io.DataOutputStream;
import java.io.IOException;
import java.net.Socket;
import java.util.Collection;
import java.util.Observable;
import java.util.Observer;

import javax.swing.BorderFactory;
import javax.swing.Box;
import javax.swing.JComponent;
import javax.swing.JLabel;
import javax.swing.JPanel;
import javax.swing.SwingUtilities;

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
 */
@ClassDescription("Serial Socket (CLIENT)")
@PluginType(PluginType.MOTE_PLUGIN)
public class SerialSocketClient extends VisPlugin implements MotePlugin {
  private static final long serialVersionUID = 1L;
  private static Logger logger = Logger.getLogger(SerialSocketClient.class);

  private final static int LABEL_WIDTH = 100;
  private final static int LABEL_HEIGHT = 15;

  public final static String SERVER_HOST = "localhost";
  public final static int SERVER_PORT = 1234;

  private SerialPort serialPort;
  private Observer serialDataObserver;

  private JLabel statusLabel, inLabel, outLabel;
  private int inBytes = 0, outBytes = 0;

  private Socket socket;
  private DataInputStream in;
  private DataOutputStream out;

  private Mote mote;

  public SerialSocketClient(Mote mote, Simulation simulation, final Cooja gui) {
    super("Serial Socket (CLIENT) (" + mote + ")", gui, false);
    this.mote = mote;

    /* GUI components */
    if (Cooja.isVisualized()) {
      Box northBox = Box.createHorizontalBox();
      northBox.setBorder(BorderFactory.createEmptyBorder(5, 5, 5, 5));
      statusLabel = configureLabel(northBox, "", "");

      Box mainBox = Box.createHorizontalBox();
      mainBox.setBorder(BorderFactory.createEmptyBorder(0, 5, 5, 5));
      inLabel = configureLabel(mainBox, "socket -> mote:", "0 bytes");
      outLabel = configureLabel(mainBox, "mote -> socket", "0 bytes");

      getContentPane().add(BorderLayout.NORTH, northBox);
      getContentPane().add(BorderLayout.CENTER, mainBox);
      pack();
    }

    /* Mote serial port */
    serialPort = (SerialPort) mote.getInterfaces().getLog();
    if (serialPort == null) {
      throw new RuntimeException("No mote serial port");
    }

    try {
      logger.info("Connecting: " + SERVER_HOST + ":" + SERVER_PORT);
      socket = new Socket(SERVER_HOST, SERVER_PORT);
      in = new DataInputStream(socket.getInputStream());
      out = new DataOutputStream(socket.getOutputStream());
      out.flush();
      startSocketReadThread(in);
    } catch (Exception e) {
      throw (RuntimeException) new RuntimeException(
          "Connection error: " + e.getMessage()).initCause(e);
    }

    /* Observe serial port for outgoing data */
    serialPort.addSerialDataObserver(serialDataObserver = new Observer() {
      public void update(Observable obs, Object obj) {
        try {
          if (out == null) {
            return;
          }
          out.write(serialPort.getLastSerialData());
          out.flush();
          outBytes++;
          if (Cooja.isVisualized()) {
            outLabel.setText(outBytes + " bytes");
          }
        } catch (IOException e) {
          e.printStackTrace();
        }
      }
    });
  }

  private void startSocketReadThread(final DataInputStream in) {
    /* Forward data: virtual port -> mote */
    Thread incomingDataThread = new Thread(new Runnable() {
      public void run() {
        int numRead = 0;
        byte[] data = new byte[1024];
        logger.info("Forwarder: socket -> serial port");
        while (true) {
          numRead = -1;
          try {
            numRead = in.read(data);
          } catch (IOException e) {
            e.printStackTrace();
            return;
          }

          if (numRead >= 0) {
            for (int i=0; i < numRead; i++) {
              serialPort.writeByte(data[i]);
            }
            inBytes += numRead;
            if (Cooja.isVisualized()) {
              inLabel.setText(inBytes + " bytes");
            }
          } else {
            logger.warn("Incoming data thread shut down");
            cleanup();
            break;
          }
        }
      }
    });
    incomingDataThread.start();
  }

  private JLabel configureLabel(JComponent pane, String desc, String value) {
    JPanel smallPane = new JPanel(new BorderLayout());
    JLabel label = new JLabel(desc);
    label.setPreferredSize(new Dimension(LABEL_WIDTH,LABEL_HEIGHT));
    smallPane.add(BorderLayout.WEST, label);
    label = new JLabel(value);
    label.setPreferredSize(new Dimension(LABEL_WIDTH,LABEL_HEIGHT));
    smallPane.add(BorderLayout.CENTER, label);
    pane.add(smallPane);
    return label;
  }

  public boolean setConfigXML(Collection<Element> configXML, boolean visAvailable) {
    return true;
  }

  public Collection<Element> getConfigXML() {
    return null;
  }

  private void cleanup() {
    serialPort.deleteSerialDataObserver(serialDataObserver);

    try {
      if (socket != null) {
        socket.close();
        socket = null;
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

    SwingUtilities.invokeLater(new Runnable() {
      public void run() {
        SerialSocketClient.this.setTitle(SerialSocketClient.this.getTitle() + " *DISCONNECTED*");
        statusLabel.setText("Disconnected from server");
      }
    });
  }

  public void closePlugin() {
    cleanup();
  }

  public Mote getMote() {
    return mote;
  }
}


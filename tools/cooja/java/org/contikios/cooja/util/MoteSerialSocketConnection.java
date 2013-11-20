/*
 * Copyright (c) 2011, Swedish Institute of Computer Science.
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

package org.contikios.cooja.util;

import java.io.DataInputStream;
import java.io.DataOutputStream;
import java.io.IOException;
import java.net.Socket;
import java.util.ArrayList;
import java.util.Observable;
import java.util.Observer;

import org.apache.log4j.Logger;

import org.contikios.cooja.Mote;
import org.contikios.cooja.interfaces.SerialPort;

/**
 * Help class for forwarding serial data between a mote and a socket.
 * 
 * @author Fredrik Osterlind
 */
public class MoteSerialSocketConnection {
  private static final long serialVersionUID = 1L;
  private static Logger logger = Logger.getLogger(MoteSerialSocketConnection.class);

  private boolean isConnected = false;
  public int toMote = 0, toSocket = 0;

  private SerialPort motePort;
  private Observer moteObserver;

  private Socket socket;
  private DataInputStream socketIn;
  private DataOutputStream socketOut;

  private ArrayList<MoteSerialSocketConnectionListener> listeners = null;

  public MoteSerialSocketConnection(Mote mote, String server, int serverPort)
  throws IOException {
    isConnected = true;

    /* Simulated -> socket */
    motePort = (SerialPort) mote.getInterfaces().getLog();
    motePort.addSerialDataObserver(moteObserver = new Observer() {
      public void update(Observable obs, Object obj) {
        try {
          if (socketOut == null) {
            return;
          }
          socketOut.write(motePort.getLastSerialData());
          socketOut.flush();
          toSocket++;

          if (listeners != null) {
            for (MoteSerialSocketConnectionListener dl: listeners) {
              dl.dataTransferred(MoteSerialSocketConnection.this, toMote, toSocket, motePort.getLastSerialData());
            }
          }
        } catch (IOException e) {
          e.printStackTrace();
          logger.fatal("Write to socket error: " + e.getMessage(), e);
          cleanup();
        }
      }
    });

    /* Socket -> simulated */
    socket = new Socket(server, serverPort);
    socketIn = new DataInputStream(socket.getInputStream());
    socketOut = new DataOutputStream(socket.getOutputStream());
    socketOut.flush();
    Thread socketThread = new Thread(new Runnable() {
      public void run() {
        int numRead = 0;
        byte[] data = new byte[1024];
        while (true) {
          numRead = -1;
          try {
            numRead = socketIn.read(data);
          } catch (IOException e) {
            e.printStackTrace();
            return;
          }

          if (numRead >= 0) {
            for (int i = 0; i < numRead; i++) {
              toMote ++;
              motePort.writeByte(data[i]);
              if (listeners != null) {
                for (MoteSerialSocketConnectionListener dl: listeners) {
                  dl.dataTransferred(MoteSerialSocketConnection.this, toMote, toSocket, data[i]);
                }
              }
            }

          } else {
            logger.fatal("Incoming data thread shut down");
            cleanup();
            break;
          }
        }
      }
    });
    socketThread.start();
  }

  public boolean isConnected() {
    return isConnected;
  }

  public void cleanup() {
    if (!isConnected) {
      return;
    }
    isConnected = false;

    motePort.deleteSerialDataObserver(moteObserver);

    try {
      if (socket != null) {
        socket.close();
        socket = null;
      }
    } catch (IOException e1) {
    }
    try {
      if (socketIn != null) {
        socketIn.close();
        socketIn = null;
      }
    } catch (IOException e) {
    }
    try {
      if (socketOut != null) {
        socketOut.close();
        socketOut = null;
      }
    } catch (IOException e) {
    }

    if (listeners != null) {
      for (MoteSerialSocketConnectionListener dl: listeners) {
        dl.wasDisconnected(MoteSerialSocketConnection.this);
      }
      listeners = null;
    }
  }

  public void addListener(MoteSerialSocketConnectionListener l) {
    if (listeners == null) {
      listeners = new ArrayList<MoteSerialSocketConnectionListener>();
    }

    listeners.add(l);
  }

  public interface MoteSerialSocketConnectionListener {
    public void wasDisconnected(MoteSerialSocketConnection s);
    public void dataTransferred(MoteSerialSocketConnection s, int toMote, int toSocket, byte data);
  }

}
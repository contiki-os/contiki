/*
 * Copyright (c) 2007, Swedish Institute of Computer Science. All rights
 * reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer. 2. Redistributions in
 * binary form must reproduce the above copyright notice, this list of
 * conditions and the following disclaimer in the documentation and/or other
 * materials provided with the distribution. 3. Neither the name of the
 * Institute nor the names of its contributors may be used to endorse or promote
 * products derived from this software without specific prior written
 * permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 * $Id: ChakanaPlugin.java,v 1.1 2009/02/26 13:47:38 fros4943 Exp $
 */

package se.sics.chakana;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.PrintWriter;
import java.net.ServerSocket;
import java.net.Socket;
import java.util.Collection;
import javax.swing.SwingUtilities;
import org.apache.log4j.Logger;
import org.jdom.Element;

import se.sics.cooja.*;

/**
 * Main Chakana plugin.
 * 
 * Provides remote control functionality to COOJA via socket connections.
 * 
 * @see #SERVER_PORT
 * @author Fredrik Osterlind
 */
@ClassDescription("Chakana COOJA Server")
@PluginType(PluginType.COOJA_STANDARD_PLUGIN)
public class ChakanaPlugin implements Plugin {
  public static final String VERSION = "0.1";
  
  /**
   * COOJA's Chakana-plugin server port
   */
  public static final int SERVER_PORT = 1234;

  private static final long serialVersionUID = 1L;

  private static Logger logger = Logger.getLogger(ChakanaPlugin.class);

  private GUI myGUI;

  // Chakana communication
  private Thread serverThread = null;

  private ServerSocket serverSocket = null;

  private Thread clientThread = null;

  private Socket clientSocket = null;

  private EventpointEvaluator myEvaluator = null;
  
  private XMLCommandHandler myCommandHandler = null;

  private boolean shutdown = false;

  private Object tag = null;

  /**
   * Creates new Chakana remote control plugin.
   * 
   * @param gui COOJA Simulator
   */
  public ChakanaPlugin(GUI gui) {
    myGUI = gui;

    // Create eventpoint evaluator
    myEvaluator = new EventpointEvaluator(myGUI);

    // Create remote command handler
    myCommandHandler = new XMLCommandHandler(myGUI, this, myEvaluator);

    // Open server socket
    serverThread = new Thread(new Runnable() {
      public void run() {
        try {
          serverSocket = new ServerSocket(SERVER_PORT);
          logger.info("Chakana server listening on port " + SERVER_PORT + ".");
        } catch (Exception e) {
          logger.fatal("Could not start server thread: " + e.getMessage());
          return;
        }

        // Handle incoming connections
        while (serverSocket != null) {
          try {
            Socket skt = serverSocket.accept();
            handleNewConnection(skt);
          } catch (Exception e) {
            if (!shutdown)
              logger.fatal("Server thread exception: " + e.getMessage());
            if (serverThread != null && serverThread.isInterrupted()) {
              serverThread = null;
            }
            serverSocket = null;
          }
        }
        logger.info("Chakana server thread terminating");
      }
    }, "chakana listen thread");
    serverThread.start();
  }

  public void performCOOJAShutdown() {
    shutdown = true;
  }
  
  /**
   * Handles incoming connection.
   * 
   * @param socket Socket
   */
  private void handleNewConnection(Socket socket) {
    logger.info("Received request from " + socket.getInetAddress() + ":"
        + socket.getPort());

    if (clientThread != null) {
      // Refuse connection
      logger.warn("A client is already connected, refusing new connection");
      try {
        socket.close();
      } catch (IOException e) {
      }
      return;
    }

    // Start thread handling connection
    clientSocket = socket;
    clientThread = new Thread(new Runnable() {
      public void run() {
        try {
          // Open stream
          PrintWriter writer = new PrintWriter(clientSocket.getOutputStream());
          BufferedReader reader = new BufferedReader(new InputStreamReader(clientSocket
              .getInputStream()));

          // Send welcome message
          writer.print(XMLCommandHandler.createInfoMessage("Chakana COOJA plugin, version " + VERSION) + "\n");
          writer.flush();

          // Handle incoming data (blocks until connection terminated)
          String line;
          String command = "";
          while ((!shutdown) && ((line = reader.readLine()) != null)) {
            logger.debug("<-- " + line);
            command += line;

            if (myCommandHandler.containsEntireCommand(command)) {
              String reply = myCommandHandler.handleCommand(command);
              logger.debug("--> " + reply);
              writer.print(reply + "\n");
              writer.flush();
              command = "";
            }
          }

          logger.debug("Terminating Chakana connection");

          // Clean up connection
          if (writer != null) {
            writer.flush();
            writer.close();
          }
          if (clientSocket != null && !clientSocket.isClosed())
            clientSocket.close();

        } catch (IOException e) {
          logger.fatal("Client connection exception: " + e);
        }

        clientThread = null;
        clientSocket = null;
	SwingUtilities.invokeLater(new Runnable() {
	    public void run() {
	      myGUI.doQuit(false);        
	    }
	  });
        logger.debug("Chakana client thread terminating");
      }
    }, "chakana client thread");
    clientThread.start();

  }

  /**
   * Shut down any open server socket, and kill server thread.
   */
  protected void closeServer() {
    // Shut down server
    if (serverSocket != null && !serverSocket.isClosed()) {
      logger.info("Closing server socket");
      try {
        serverSocket.close();
      } catch (IOException e) {
        logger.fatal("Error when closing server socket: " + e);
      }
      serverSocket = null;
    }

    if (serverThread != null && serverThread.isAlive()) {
      logger.info("Interrupting server thread");
      serverThread.interrupt();
      serverThread = null;
    }
  }

  /**
   * Disconnect all connected clients. Currently only one client can be
   * connected at the same time.
   */
  protected void disconnectClients() {
    // Disconnect any current accepted client
    if (clientSocket != null && !clientSocket.isClosed()) {
      logger.info("Closing client socket");
      try {
        clientSocket.close();
      } catch (IOException e) {
        logger.fatal("Error when closing client socket: " + e);
      }
      clientSocket = null;
    }

    if (clientThread != null && clientThread.isAlive()) {
      logger.info("Interrupting client thread");
      clientThread.interrupt();
      clientThread = null;
    }
  }

  public void closePlugin() {
    closeServer();
    disconnectClients();
  }

  public void tagWithObject(Object tag) {
    this.tag = tag;
  }

  public Object getTag() {
    return tag;
  }

  public Collection<Element> getConfigXML() {
    return null;
  }

  public boolean setConfigXML(Collection<Element> configXML,
      boolean visAvailable) {
    return false;
  }

}

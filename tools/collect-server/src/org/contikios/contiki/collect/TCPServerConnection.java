/*
 * Copyright (c) 2015, Singapore University of Technology and Design.
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
 * -----------------------------------------------------------------
 *
 * TCPServerConnection
 *
 * Authors : Mao Ngo
 * Email: newngovanmao@gmail.com
 */

package org.contikios.contiki.collect;
import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.PrintStream;
import java.net.Socket;
import java.net.ServerSocket;

/**
 *
 */
public class TCPServerConnection extends SerialConnection {

    private ServerSocket serverSk;
    private Socket clientSk;
    private String clientSbanIP = "";

    private BufferedReader in;
    private PrintStream out;

    public TCPServerConnection(SerialConnectionListener listener, ServerSocket socket) {
        super(listener);
        this.serverSk = socket;
        isMultiplePorts = true; //support both serial and Server 
    }

    @Override
    public String getConnectionName() {
        if (clientSbanIP.length() > 1) {
        	int colon = clientSbanIP.lastIndexOf(':');
        	return clientSbanIP.substring(1,colon) + "_" + System.currentTimeMillis();
        }
        else {
        	return clientSbanIP;
        }
    }
    

    @Override
    public void open(String comPort) {
        close();
        this.comPort = comPort == null ? "" : comPort;
        System.out.println("Ready for listening for a comming SBAN");
        isClosed = false;      
        try {
        	new MultiServerThread(serverSk.accept()).start();
        }catch (Exception e) {
            e.printStackTrace();
        }
        isOpen =true;

//            /* Start thread listening on UDP */
//            Thread readInput = new Thread(new Runnable() {
//                public void run() {
//                    try {
//                    	clientSk = serverSk.accept();
//                    	in = new BufferedReader(new InputStreamReader(clientSk.getInputStream()));
//                        out = new PrintStream(clientSk.getOutputStream());
//                    	System.out.println(" Handling new client, opened readInput thread OK");    
//                        String line;
//                        while (isOpen && ((line = in.readLine()) != null)) {
//                          serialData(line);
//                          //System.out.println("receiving data: "+ line);
//                        }
//                        //closeConnection();
//                        doCloseClient();
//                   } catch (IOException e) {
//                        lastError = "Error when reading from SerialConnection TCP: " + e;
//                        System.err.println(lastError);
//                        if (!isClosed) {
//                            e.printStackTrace();
//                            //closeConnection();
//                            doCloseClient();
//                        }
//                    } finally {
//                        System.out.println("SerialConnection TCP terminated.");
//                        // closeConnection();
//                        doCloseClient();
//                    }
//                }
//            }, "TCP thread");
//            isOpen = true;
//            while (isOpen) {
//               serialOpened();
//               readInput.start();
//            }
            
    }
    


    
    @Override
    protected void doClose() {
        try {
            if (in != null) {
                in.close();
                in = null;
            }
            if (out != null) {
                out.close();
                out = null;
            }
            if (clientSk != null) {
                clientSk.close();
                clientSk = null;
            }
        } catch (IOException e) {
            e.printStackTrace();
        }
    }
    
    private class MultiServerThread extends Thread {
        private Socket socket = null;

        public MultiServerThread(Socket socket) {
            this.socket = socket;
        }

        public void run() {

            try {
//            	PrintStream out = new PrintStream(socket.getOutputStream(), true);
            	clientSbanIP = socket.getRemoteSocketAddress().toString();
            	BufferedReader in = new BufferedReader(
                        				new InputStreamReader(
                        						socket.getInputStream()));
            	System.out.println("Create new thread to handle new client...");
                String inputLine;// , outputLine;
                //KnockKnockProtocol kkp = new KnockKnockProtocol();
                //outputLine = kkp.processInput(null);
                //out.println(outputLine);
//                String fromUser;
//                BufferedReader stdIn =
//                                         new BufferedReader(new InputStreamReader(System.in));

                while ((inputLine = in.readLine()) != null) {
                	serialData(inputLine);
                	//outputLine = kkp.processInput(inputLine);
                    //System.out.println("From: " + socket + " receives: " + inputLine);
                    //outputLine = inputLine.toUpperCase();
                    //out.println(outputLine);
                    //if (outputLine.equals("Bye"))
                      //  break;
//                  fromUser = stdIn.readLine();
//                  if (fromUser != null) {
//                    System.out.println("From user: "+ fromUser);
//                    out.println(fromUser);
    //
//                  }
                }
                //socket.close();
                doCloseClient();
            } catch (IOException e) {
                e.printStackTrace();
            }
        }
	    
        private void doCloseClient() {
	    	try {
	            if (in != null) {
	                in.close();
	                in = null;
	            }
//	            if (out != null) {
//	                out.close();
//	                out = null;
//	            }
	            if (socket != null) {
	            	System.out.println("Close Client Socket!!!");
	            	socket.close();
	            	socket = null;
	            }
	        } catch (IOException e) {
	        	e.printStackTrace();
	        }
	    }
    }

    

}

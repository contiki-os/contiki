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

    private final int port;

    private ServerSocket serverSk;
    private Socket clientSk;
    private BufferedReader in;
    private PrintStream out;

    public TCPServerConnection(SerialConnectionListener listener, int port) {
        super(listener);
        this.port = port;
	System.out.println("TCPServerConnection constructor");
    }

    @Override
    public String getConnectionName() {
        return "<tcp://localhost:" + port + '>';
    }

    @Override
    public void open(String comPort) {
        close();
        this.comPort = comPort == null ? "" : comPort;

        System.out.println("Opening TCP server in the port:" + port);
        isClosed = false;
        try {
	    serverSk = new ServerSocket(port);
	    clientSk = serverSk.accept();
            in = new BufferedReader(new InputStreamReader(clientSk.getInputStream()));
            out = new PrintStream(clientSk.getOutputStream());
            System.out.println("Opened TCP server in the port:" + port);
            /* Start thread listening on UDP */
            Thread readInput = new Thread(new Runnable() {
                public void run() {
                    try {
			System.out.println("Mao Ngo Opened readInput thread OK");    
                        String line;
                        while (isOpen && (line = in.readLine()) != null) {
                          serialData(line);
			  System.out.println("receiving data: "+ line);
                        }
                   } catch (IOException e) {
                        lastError = "Error when reading from SerialConnection TCP: " + e;
                        System.err.println(lastError);
                        if (!isClosed) {
                            e.printStackTrace();
                          //  closeConnection();
                        }
                    } finally {
                        System.out.println("SerialConnection TCP terminated.");
                        //closeConnection();
                    }
                }
            }, "TCP thread");
            isOpen = true;
            serialOpened();
            readInput.start();

        } catch (Exception e) {
            lastError = "Failed to open TCP server at port: " + port + ": " + e;
            System.err.println(lastError);
            e.printStackTrace();
           // closeConnection();
        }
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
            if (serverSk != null) {
                serverSk.close();
                serverSk = null;
            }
            if (clientSk != null) {
                clientSk.close();
                clientSk = null;
            }
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

}
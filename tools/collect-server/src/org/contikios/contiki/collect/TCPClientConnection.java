/*
 * Copyright (c) 2012, Swedish Institute of Computer Science.
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
 * TCPClientConnection
 *
 * Authors : Niclas Finne
 */

package org.contikios.contiki.collect;
import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.PrintStream;
import java.net.Socket;

/**
 *
 */
public class TCPClientConnection extends SerialConnection {

    private final String host;
    private final int port;

    private Socket client;
    private BufferedReader in;
    private PrintStream out;

    public TCPClientConnection(SerialConnectionListener listener, String host, int port) {
        super(listener);
        this.host = host;
        this.port = port;
    }

    @Override
    public String getConnectionName() {
        return "<tcp://" + host + ':' + port + '>';
    }

    @Override
    public void open(String comPort) {
        close();
        this.comPort = comPort == null ? "" : comPort;

        isClosed = false;
        try {
            client = new Socket(host, port);
            in = new BufferedReader(new InputStreamReader(client.getInputStream()));
            out = new PrintStream(client.getOutputStream());
            System.out.println("Opened TCP connection to " + host + ':' + port);
            /* Start thread listening on UDP */
            Thread readInput = new Thread(new Runnable() {
                public void run() {
                    try {
                        String line;
                        while (isOpen && (line = in.readLine()) != null) {
                            serialData(line);
                        }
                   } catch (IOException e) {
                        lastError = "Error when reading from SerialConnection TCP: " + e;
                        System.err.println(lastError);
                        if (!isClosed) {
                            e.printStackTrace();
                            closeConnection();
                        }
                    } finally {
                        System.out.println("SerialConnection TCP terminated.");
                        closeConnection();
                    }
                }
            }, "TCP thread");
            isOpen = true;
            serialOpened();
            readInput.start();

        } catch (Exception e) {
            lastError = "Failed to open TCP connection to " + host + ':' + port + ": " + e;
            System.err.println(lastError);
            e.printStackTrace();
            closeConnection();
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
            if (client != null) {
                client.close();
                client = null;
            }
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

}

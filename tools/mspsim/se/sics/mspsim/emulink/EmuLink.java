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
 * This file is part of MSPSim.
 *
 * -----------------------------------------------------------------
 *
 * EmuLink
 *
 * Authors : Joakim Eriksson, Niclas Finne
 * Created : 11 oct 2012
 */

package se.sics.mspsim.emulink;

import java.io.BufferedReader;
import java.io.File;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.PrintWriter;
import java.io.Reader;
import java.net.ServerSocket;
import java.net.Socket;
import java.util.Hashtable;

import se.sics.json.JSONArray;
import se.sics.json.JSONObject;
import se.sics.json.ParseException;
import se.sics.mspsim.Main;
import se.sics.mspsim.core.MSP430Constants;
import se.sics.mspsim.platform.GenericNode;
import se.sics.mspsim.util.Utils;

public class EmuLink {

    private Socket socket;
    private BufferedReader in;
    private PrintWriter out;
    private boolean isConnected = false;
    private long globalEtime = 0L;
    private long globalLastTime = 0L;

    private final Hashtable<String, GenericNode> nodes = new Hashtable<String, GenericNode>();

    public boolean isConnected() {
        return !isConnected;
    }

    String[] getNodes(JSONObject json) {
        JSONArray nodes;
        String node;
        String[] nString = null;
        if ((nodes = json.getJSONArray("node")) != null) {
            nString = new String[nodes.size()];
            for(int i = 0, n = nodes.size(); i < n; i++) {
                node = nodes.getAsString(i);
                nString[i] = node;
            }
        } else if ((node = json.getAsString("node")) != null) {
            nString = new String[1];
            nString[0] = node;
        }
        return nString;
    }
    
    private boolean createNode(String type, String id) {
        String nt = Main.getNodeTypeByPlatform(type);
        System.out.println("EmuLink: Creating node '" + id + "' of type '" + type + "' => " + nt);
        GenericNode node = Main.createNode(nt);
        if (node == null) {
            System.err.println("EmuLink: failed to create node '" + id + "' of type '" + type + "'");
            return false;
        }
        nodes.put(id, node);
        return true;
    }
    
    private boolean createNodes(JSONObject json) {
        String type = json.getAsString("type");
        System.out.println("Should create: " + type);

        String[] nodes = getNodes(json);
        if (nodes != null) {
            for (int i = 0; i < nodes.length; i++) {
                createNode(type, nodes[i]);
            }
        }
        return true;
    }

    protected void processInput(Reader input) throws IOException, ParseException {
        StringBuilder sb = new StringBuilder();
        int brackets = 0;
        boolean stuffed = false;
        boolean quoted = false;
        while (isConnected()) {
            int c = input.read();
            if (c < 0) {
                disconnect();
                break;
            }
            sb.append((char)c);
            if (stuffed) {
                stuffed = false;
            } else if (c == '\\') {
                stuffed = true;
            } else if (quoted) {
                if (c == '"') {
                    quoted = false;
                }
            } else if (c == '"') {
                quoted = true;
            } else if (c == '{') {
                brackets++;
            } else if (c == '}') {
                brackets--;
                if (brackets == 0) {
                    JSONObject json = JSONObject.parseJSONObject(sb.toString());
                    sb.setLength(0);
                    if (!handleMessage(json)) {
                        // This connection should no longer be kept alive
                        break;
                    }
                }
            }
        }
    };

    protected boolean handleMessage(JSONObject json) {
        System.out.println("EmuLink: RECV " + json.toJSONString());
        String event = json.getAsString("event");
        if ("emulation_control".equals(event)) {
            String command = json.getAsString("data");
            if ("start".equals(command)) {
                String[] nodes = getNodes(json);
                long etime = json.getAsLong("etime", 0);

                if (etime == 0) {
                    if (nodes == null) {
                        for (GenericNode node : this.nodes.values()) {
                            node.start();
                        }
                    } else {
                        for(String id : nodes) {
                            GenericNode node = this.nodes.get(id);
                            if (node != null) {
                                node.start();
                            } else {
                                System.err.println("EmuLink: could not find node '" + id + "'");
                            }
                        }
                    }
                } else {
                    // Execute for specified time
                    globalEtime += etime;
                    if (nodes == null) {
                        for (GenericNode node : this.nodes.values()) {
                            long t0 = System.nanoTime(), t1;
                            node.getCPU().stepMicros(globalLastTime, etime);
                            t1 = System.nanoTime();
                            System.out.println("EmuLink: node " + node.getID()
                                    + " is now at $" + Utils.hex(node.getCPU().getRegister(MSP430Constants.PC), 4)
                                    + " " + (long)(node.getCPU().getTimeMillis() + 0.5) + "/" + (globalEtime / 1000)
                                    + " (elapsed " + (t1 - t0) + " ns)");
                        }

                    } else {
                        for(String id : nodes) {
                            GenericNode node = this.nodes.get(id);
                            if (node != null) {
                                long t0 = System.nanoTime(), t1;
                                node.getCPU().stepMicros(globalLastTime, etime);
                                t1 = System.nanoTime();
                                System.out.println("EmuLink: node " + node.getID()
                                        + " is now at $" + Utils.hex(node.getCPU().getRegister(MSP430Constants.PC), 4)
                                        + " " + (long)(node.getCPU().getTimeMillis() + 0.5) + "/" + (globalEtime / 1000)
                                        + " (elapsed " + (t1 - t0) + " ns)");
                            } else {
                                System.err.println("EmuLink: could not find node '" + id + "'");
                            }
                        }
                    }
                    globalLastTime = etime;
                }
            } else if ("stop".equals(command)) {
                String[] nodes = getNodes(json);
                if (nodes == null) {
                    for (GenericNode node : this.nodes.values()) {
                        node.stop();
                    }
                } else {
                    for(String id : nodes) {
                        GenericNode node = this.nodes.get(id);
                        if (node != null) {
                            node.stop();
                        } else {
                            System.err.println("EmuLink: could not find node '" + id + "'");
                        }
                    }
                }
            } else if ("close".equals(command)) {
                sendToSimulator("{\"response\":\"emulation_control\",\"data\":1}");

                // Time to close the connection
                return false;
            }
            sendToSimulator("{\"response\":\"emulation_control\",\"data\":1}");

        } else if ("create".equals(event)) {
            createNodes(json);
            sendToSimulator("{\"response\":\"create\",\"data\":1}");
        } else if ("write".equals(event)) {
            String[] nodes = getNodes(json);
            String file = json.getAsString("file");
            if (nodes == null) {
                System.err.println("EmuLink: no node specified for write");
            } else if (file == null) {
                System.err.println("EmuLink: no file specified for write");
            } else if (!new File(file).isFile()) {
                System.err.println("EmuLink: could not find file '" + file + "' for write");
            } else {
                for(String id : nodes) {
                    GenericNode node = this.nodes.get(id);
                    if (node == null) {
                        System.err.println("EmuLink: could not find node '" + id + "'");
                        continue;
                    }
                    try {
                        node.loadFirmware(file);
                        node.getCPU().reset();
                    } catch (IOException e) {
                        System.err.println("EmuLink: failed to load firmware '" + file + "'");
                        e.printStackTrace();
                    }
                }
            }
            sendToSimulator("{\"response\":\"write\",\"data\":1}");
        } else if ("remove".equals(event)) {
            // TODO remove nodes
        } else if ("serial".equals(event)) {
            int data = json.getAsInt("data", -1);
            JSONArray nodes;
            String node;
            if (data < 0) {
                // No data - ignore serial event
            } else if ((nodes = json.getJSONArray("node")) != null) {
                for(int i = 0, n = nodes.size(); i < n; i++) {
                    node = nodes.getAsString(i);
                    if (node != null) {
                        sendSerialToNode(node, data);
                    }
                }
            } else if ((node = json.getAsString("node")) != null) {
                sendSerialToNode(node, data);
            } else {
                // No target node specified
            }
        } else {
            System.err.println("EmuLink: ignoring unhandled event '" + event + "'");
        }
        return true;
    }

    protected void sendToSimulator(String message) {
        if (out != null) {
            out.write(message);
            out.flush();
        }
    }

    protected void sendSerialToNode(String node, int data) {
    }

    protected void disconnect() {
        boolean isDisconnecting = isConnected;
        isConnected = false;
        try {
            if (isDisconnecting) {
                System.err.println("EmuLink: disconnecting...");
            }
            if (out != null) {
                out.close();
                out = null;
            }
            if (in != null) {
                in.close();
                in = null;
            }
            if (socket != null) {
                socket.close();
                socket = null;
            }
            if (isDisconnecting) {
                System.err.println("EmuLink: disconnected");
            }
        } catch (IOException e) {
            System.err.println("EmuLink: failed to close emulation link connection");
            e.printStackTrace();
        }
    }

    public void run() throws IOException {
        try {
            ServerSocket serverSocket = new ServerSocket(8000);

            while(true) {

                System.out.println("EmuLink: Waiting for connection...");
                try {
                    socket = serverSocket.accept();
                } catch (IOException e) {
                    System.out.println("Accept failed: 8000");
                    System.exit(-1);
                }
                System.out.println("EmuLink: Connection accepted...");

                try {
                    out = new PrintWriter(socket.getOutputStream(), true);
                    in = new BufferedReader(new InputStreamReader(socket.getInputStream()));
                    processInput(in);
                } catch (Exception e) {
                    System.err.println("EmuLink: emulator link connection failed");
                    e.printStackTrace();
                } finally {
                    disconnect();
                }
            }
        } catch (IOException e) {
            System.out.println("Could not listen on port: 8000");
            System.exit(-1);
        }
    }

    public static void main(String[] args) throws IOException {
        EmuLink el = new EmuLink();
        el.run();
    }

}

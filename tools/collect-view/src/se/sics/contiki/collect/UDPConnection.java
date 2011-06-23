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
 *
 * -----------------------------------------------------------------
 *
 * UDPConnection
 *
 * Authors : Niclas Finne
 * Created : 1 June 2011
 */

package se.sics.contiki.collect;

import java.io.IOException;
import java.net.DatagramPacket;
import java.net.DatagramSocket;
import java.net.InetAddress;
import java.io.*;
/**
 *
 */
public class UDPConnection extends SerialConnection {

  private final int port;
private DatagramSocket serverSocket;

  public UDPConnection(SerialConnectionListener listener, int port) {
    super(listener);
    this.port = port;
  }

  @Override
  public String getConnectionName() {
    return "<UDP:" + port + ">";
  }

  @Override
  public void open(String comPort) {
    close();
    this.comPort = comPort == null ? "" : comPort;

    isClosed = false;
    try {
      serverSocket = new DatagramSocket(port);
      System.out.println("Opened UDP port: " + port);
      /* Start thread listening on UDP */
      Thread readInput = new Thread(new Runnable() {
        public void run() {
          byte[] data = new byte[1024];
          try {
            while (isOpen) {
              DatagramPacket packet = new DatagramPacket(data, data.length);
              serverSocket.receive(packet);

              InetAddress addr = packet.getAddress();
              System.out.println("UDP: received " + packet.getLength() + " bytes from " + addr.getHostAddress() + ":" + packet.getPort());

	      StringWriter strOut = new StringWriter();
	      PrintWriter out = new PrintWriter(strOut);
	      int payloadLen = packet.getLength() - 2;
	      out.printf("%d", 8 + payloadLen / 2);
	      /* Timestamp. Ignore time synch for now. */
	      long time = System.currentTimeMillis() / 1000;
	      out.printf(" %d %d 0",
				((time >> 16) & 0xffff), time & 0xffff);
	      byte[] payload = packet.getData();
	      int seqno = payload[0] & 0xff;
	      int hops = 0;  /* how to get TTL / hot limit in Java??? */
	      byte[] address = addr.getAddress();
	      /* Ignore latency for now */
	      out.printf(" %d %d %d %d",
			 ((address[14] & 0xff) +
			  ((address[15] & 0xff) << 8))&0xffff, seqno, hops, 0);
	      int d = 0;
	      for(int i = 0; i < payloadLen ; i += 2) {
		  d = (payload[i + 2] & 0xff) + ((payload[i + 3] & 0xff) << 8);
		  out.printf(" %d", d & 0xffff);
	      }

	      String line = strOut.toString();
              serialData(line);
            }
            System.out.println("SerialConnection UDP terminated.");
            closeConnection();
          } catch (IOException e) {
            lastError = "Error when reading from SerialConnection UDP: " + e;
            System.err.println(lastError);
            if (!isClosed) {
              e.printStackTrace();
              closeConnection();
            }
          }
        }
      }, "UDP thread");
      isOpen = true;
      serialOpened();
      readInput.start();

    } catch (Exception e) {
      lastError = "Failed to open UDP server at port " + port + ": " + e;
      System.err.println(lastError);
      e.printStackTrace();
      closeConnection();
    }
  }

  @Override
  protected void doClose() {
      if (serverSocket != null) {
          serverSocket.close();
          serverSocket = null;
      }
  }

}

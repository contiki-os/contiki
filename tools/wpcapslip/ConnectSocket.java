/*
 * Copyright (c) 2013, Thingsquare, http://www.thingsquare.com/.
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
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

import java.io.BufferedInputStream;
import java.io.BufferedOutputStream;
import java.io.DataInputStream;
import java.net.Socket;

public class ConnectSocket {
	private static BufferedOutputStream stdoutOutput = null;
	private static BufferedOutputStream networkServerOutput = null;

	private static final int BUFSIZE = 512;

	public static void main(String[] args) throws Exception {
		if (args.length != 2) {
			System.err.println("Usage: " + ConnectSocket.class.getName() + " [server ip] [server port]");
			System.exit(1);
		}

		/* Stdin */
		final BufferedInputStream stdinInput = new BufferedInputStream(new DataInputStream(System.in));
		stdoutOutput = new BufferedOutputStream(System.out);
		Thread readInput = new Thread(new Runnable() {
			public void run() {
				int numRead = 0;
				byte[] buf = new byte[BUFSIZE];
				try {
					while (true) {
						numRead = stdinInput.read(buf, 0, BUFSIZE);
						if (numRead > 0 && networkServerOutput != null) {
							/* System.err.println("1>2 " + numRead); */

							networkServerOutput.write(buf, 0, numRead);
							networkServerOutput.flush();
						}
						Thread.sleep(1);
					}
				} catch (Exception e) {
					e.printStackTrace();
				}
				exit();
			}
		}, "read stdin");
		readInput.start();

		/* Network server */
		Socket networkServer = new Socket(args[0], Integer.parseInt(args[1]));
		final BufferedInputStream networkServerInput = new BufferedInputStream(new DataInputStream(networkServer.getInputStream()));
		networkServerOutput = new BufferedOutputStream(networkServer.getOutputStream());
		readInput = new Thread(new Runnable() {
			public void run() {
				int numRead = 0;
				byte[] buf = new byte[BUFSIZE];
				try {
					while (true) {
						numRead = networkServerInput.read(buf, 0, BUFSIZE);
						if (numRead > 0 && stdoutOutput != null) {
							/* System.err.println("2>1 " + numRead); */
							stdoutOutput.write(buf, 0, numRead);
							stdoutOutput.flush();
						}
						Thread.sleep(1);
					}
				} catch (Exception e) {
					e.printStackTrace();
				}
				exit();
			}
		}, "read network server");
		readInput.start();

		while (true) {
			Thread.sleep(100);
		}
	}

	private static void exit() {
		try {
			Thread.sleep(500);
		} catch (InterruptedException e) {
		}
		System.err.flush();
		System.exit(1);
	}
}

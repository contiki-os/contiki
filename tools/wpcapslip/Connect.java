/*
 * Copyright (c) 2012, Thingsquare, http://www.thingsquare.com/.
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

import java.io.BufferedOutputStream;
import java.io.BufferedReader;
import java.io.DataInputStream;
import java.io.File;
import java.io.IOException;
import java.io.InputStreamReader;

public class Connect {
	private static Process cmd1Process;
	private static Process cmd2Process;
	private static BufferedOutputStream cmd1Out = null;
	private static BufferedOutputStream cmd2Out = null;

	private static final int BUFSIZE = 512;

	public static void main(String[] args) throws Exception {
		if (args.length != 2) {
			System.err.println("Usage: " + Connect.class.getName() + " [cmd1] [cmd2]");
			System.exit(1);
		}

		/* Command 1 */
		String cmd1 = args[0];
		System.out.println("> " + cmd1);
		cmd1Process = Runtime.getRuntime().exec(cmd1, null, new File("."));
		final DataInputStream cmd1Input = new DataInputStream(
				cmd1Process.getInputStream());
		final BufferedReader cmd1Err = new BufferedReader(
				new InputStreamReader(cmd1Process.getErrorStream()));
		cmd1Out = new BufferedOutputStream(cmd1Process.getOutputStream());
		Thread readInput = new Thread(new Runnable() {

			public void run() {
				int numRead = 0;
				byte[] buf = new byte[BUFSIZE];
				try {
					while (true) {
						numRead = cmd1Input.read(buf, 0, BUFSIZE);
						if (numRead > 0 && cmd2Out != null) {
							/* System.err.println("1>2 " + numRead); */
							cmd2Out.write(buf, 0, numRead);
							cmd2Out.flush();
						}
						Thread.sleep(1);
					}
				} catch (Exception e) {
					e.printStackTrace();
				}
				String exitVal = "?";
				try {
					if (cmd1Process != null) {
						exitVal = "" + cmd1Process.exitValue();
					}
				} catch (IllegalStateException e) {
					e.printStackTrace();
					exitVal = "!";
				}
				System.out.println("cmd1 terminated: " + exitVal);
				exit();
			}
		}, "read stdout cmd1");
		Thread readError = new Thread(new Runnable() {
			public void run() {
				String line;
				try {
					while ((line = cmd1Err.readLine()) != null) {
						System.err.println("cmd1 err: " + line);
					}
					cmd1Err.close();
				} catch (IOException e) {
				}
				System.err.println("cmd1 terminated.");
				exit();
			}
		}, "read error cmd1");
		readInput.start();
		readError.start();

		/* Command 2 */
		String cmd2 = args[1];
		System.err.println("> " + cmd2);
		cmd2Process = Runtime.getRuntime().exec(cmd2, null, new File("."));
		final DataInputStream cmd2Input = new DataInputStream(
				cmd2Process.getInputStream());
		final BufferedReader cmd2Err = new BufferedReader(
				new InputStreamReader(cmd2Process.getErrorStream()));
		cmd2Out = new BufferedOutputStream(cmd2Process.getOutputStream());
		readInput = new Thread(new Runnable() {
			public void run() {
				int numRead = 0;
				byte[] buf = new byte[BUFSIZE];
				try {
					while (true) {
						numRead = cmd2Input.read(buf, 0, BUFSIZE);
						if (numRead > 0 && cmd1Out != null) {
							/* System.err.println("2>1 " + numRead); */
							cmd1Out.write(buf, 0, numRead);
							cmd1Out.flush();
						}
						Thread.sleep(1);
					}
				} catch (Exception e) {
					e.printStackTrace();
				}
				String exitVal = "?";
				try {
					if (cmd2Process != null) {
						exitVal = "" + cmd2Process.exitValue();
					}
				} catch (IllegalStateException e) {
					e.printStackTrace();
					exitVal = "!";
				}
				System.out.println("cmd2 terminated: " + exitVal);
				exit();
			}
		}, "read stdout cmd2");
		readError = new Thread(new Runnable() {
			public void run() {
				String line;
				try {
					while ((line = cmd2Err.readLine()) != null) {
						System.err.println("cmd2 err: " + line);
					}
					cmd2Err.close();
				} catch (IOException e) {
				}
				System.err.println("cmd2 terminated.");
				exit();
			}
		}, "read error cmd2");
		readInput.start();
		readError.start();

		while (true) {
			Thread.sleep(100);
		}
	}

	private static void exit() {
		try {
			Thread.sleep(500);
		} catch (InterruptedException e) {
		}
		try {
			if (cmd1Process != null) {
				cmd1Process.destroy();
			}
		} catch (Exception e) {
		}
		try {
			if (cmd2Process != null) {
				cmd2Process.destroy();
			}
		} catch (Exception e) {
		}
		System.err.flush();
		System.exit(1);
	}

}

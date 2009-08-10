/*
 * Copyright (c) 2009, Swedish Institute of Computer Science
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
 * This file is part of the Contiki operating system.
 *
 * $Id: CoffeeManager.java,v 1.3 2009/08/10 12:51:52 nvt-se Exp $
 *
 * @author Nicolas Tsiftes
 *
 */

package se.sics.coffee;

import java.io.IOException;
import java.util.Iterator;
import java.util.Map;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

public class CoffeeManager {
	private static CoffeeFS coffeeFS;
	public enum Command { INSERT, EXTRACT, REMOVE, LIST, STATS };

	public static void main(String args[]) {
		String platform = "sky";
		String usage = "Usage: java -jar coffee.jar ";
		Command command = Command.STATS;
		String filename = "";
		String fsImage = "";

		usage += "[-i|e|r <file>] ";
		usage += "[-l|s] ";
		usage += "<file system image>";

		if (args.length < 1) {
			System.err.println(usage);
			System.exit(1);
		}

		if (args.length > 1) {
			Pattern validArg = Pattern.compile("-(i|e|r|l|s)");
			Matcher m = validArg.matcher(args[0]);
			if (!m.matches()) {
				System.err.println(usage);
				System.exit(1);
			}

			if (args[0].equals("-i") || args[0].equals("-e") || args[0].equals("-r")) {
				if (args.length != 3) {
					System.err.println(usage);
					System.exit(1);
				}
				if (args[0].equals("-i")) {
					command = Command.INSERT;
				} else if (args[0].equals("-r")) {
					command = Command.REMOVE;
				} else {
					command = Command.EXTRACT;
				}
				filename = args[1];
				fsImage = args[2];
			} else {
				if (args.length != 2) {
					System.err.println(usage);
					System.exit(1);
				}
				if (args[0].equals("-l")) {
					command = Command.LIST;
				} else {
					command = Command.STATS;
				}
				fsImage = args[1];
			}
		}

		try {
			CoffeeConfiguration conf = new CoffeeConfiguration(platform + ".properties");
			coffeeFS = new CoffeeFS(new CoffeeImageFile(fsImage, conf));
			switch (command) {
			case INSERT:
				if (coffeeFS.getFiles().get(filename) != null) {
					System.err.println("error: file \"" +
						filename + "\" already exists");
					break;
				}
				if (coffeeFS.insertFile(filename) != null) {
					System.out.println("Inserted the local file \"" +
						filename +
						"\" into the file system image");
				}
				break;
			case EXTRACT:
				if (coffeeFS.extractFile(filename) == false) {
					System.err.println("Inexistent file: " +
						filename);
					System.exit(1);
				}
				System.out.println("Saved the file \"" +
					filename + "\"");
				break;
			case REMOVE:
				coffeeFS.removeFile(filename);
				System.out.println("Removed the file \"" +
					filename +
					"\" from the Coffee file system image");
				break;
			case LIST:
				printFiles(coffeeFS.getFiles());
				break;
			case STATS:
				printStatistics(coffeeFS);
				break;
			default:
				System.err.println("Unknown command!");
				System.exit(1);
			}
		} catch (IOException e) {
			System.err.println(e.getMessage());
		}
	}

	private static void printStatistics(CoffeeFS coffeeFS) {
		int bytesWritten = 0;
		int bytesReserved = 0;
		int fileCount = 0;
		CoffeeConfiguration conf = coffeeFS.getConfiguration();

		try {
			Iterator<Map.Entry<String, CoffeeFile>> iterator =
				coffeeFS.getFiles().entrySet().iterator();
			while (iterator.hasNext()) {
				Map.Entry<String, CoffeeFile> pair = (Map.Entry<String, CoffeeFile>) iterator.next();
				String key = pair.getKey();
				CoffeeFile file = pair.getValue();
				bytesWritten += file.getLength();
				bytesReserved += file.getHeader().getReservedSize();
				fileCount++;
			}
			bytesReserved *= conf.pageSize;
			System.out.println("File system size: " +
				conf.fsSize / 1024 + "kb");
			System.out.println("Allocated files: " + fileCount);
			System.out.println("Reserved bytes: " + bytesReserved + " (" + 
				(100 * ((float) bytesReserved / conf.fsSize)) +
				"%)");
			System.out.println("Written bytes: " + bytesWritten +
				" (" +
				(100 * ((float) bytesWritten / conf.fsSize)) +
				"%)");
		} catch (IOException e) {
			System.err.println("failed to determine the file length");
		}
	}

	private static void printFiles(Map<String, CoffeeFile> files) {
		try {
			Iterator<Map.Entry<String, CoffeeFile>> iterator = files.entrySet().iterator();
			while (iterator.hasNext()) {
				Map.Entry<String, CoffeeFile> pair = (Map.Entry<String, CoffeeFile>) iterator.next();
				String key = pair.getKey();
				CoffeeFile file = pair.getValue();
				System.out.println(file.getName() + " " + file.getLength());
			}
		} catch (IOException e) {
			System.err.println("failed to determine the file length");
		}
	}
}

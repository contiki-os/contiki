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
 * $Id: CoffeeManager.java,v 1.1 2009/08/04 10:36:53 nvt-se Exp $
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
	private static final int COMMAND_NULL    = 0;
	private static final int COMMAND_INSERT  = 1;
	private static final int COMMAND_EXTRACT = 2;
	private static final int COMMAND_REMOVE  = 3;
	private static final int COMMAND_LIST    = 4;
	private static final int COMMAND_STATS   = 5;

	public static void main(String args[]) {
		String platform = "sky";
		String usage = "Usage: java -jar coffee.jar ";
		int command = COMMAND_NULL;
		String coffeeFile = "";
		String localFile = "";
		String fsImage = "";

		usage += "[-i|e <Coffee file> <file>] ";
		usage += "[-r <Coffee file>] ";
		usage += "[-l|s] ";
		usage += "<file system image>";

		if(args.length < 1) {
			System.err.println(usage);
			System.exit(1);
		}

		if(args.length > 1) {
			Pattern validArg = Pattern.compile("-(i|e|r|l|s)");
			Matcher m = validArg.matcher(args[0]);
			if(!m.matches()) {
				System.err.println(usage);
				System.exit(1);
			}

			if(args[0].equals("-i") || args[0].equals("-e")) {
				if(args.length != 4) {
					System.err.println(usage);
					System.exit(1);
				}
				if(args[0].equals("-i")) {
					command = COMMAND_INSERT;
				} else {
					command = COMMAND_EXTRACT;
				}
				coffeeFile = args[1];
				localFile = args[2];
				fsImage = args[3];
			} else if (args[0].equals("-r")) {
				if(args.length != 3) {
					System.err.println(usage);
					System.exit(1);
				}
				command = COMMAND_REMOVE;
				coffeeFile = args[1];
				fsImage = args[2];
			} else {
				if(args.length != 2) {
					System.err.println(usage);
					System.exit(1);
				}
				if(args[0].equals("-l")) {
					command = COMMAND_LIST;
				} else {
					command = COMMAND_STATS;
				}
				fsImage = args[1];
			}
		}

		try {
			CoffeeConfiguration conf = new CoffeeConfiguration(platform + ".properties");
			coffeeFS = new CoffeeFS(new CoffeeImageFile(fsImage, conf));
			switch (command) {
			case COMMAND_INSERT:
				if(coffeeFS.getFiles().get(localFile) != null) {
					System.err.println("error: file \"" + localFile + "\" already exists");
					break;
				}
				if(coffeeFS.insertFile(localFile) != null) {
					System.out.println("Inserted the local file \"" + localFile + "\" into the file system image");
				}
				break;
			case COMMAND_EXTRACT:
				if(coffeeFS.extractFile(coffeeFile, localFile) == false) {
					System.err.println("Inexistent file: " + coffeeFile);
					System.exit(1);
				}
				System.out.println("Saved the file \"" + coffeeFile + "\" from the system image into the local file \"" + localFile + "\"");
				break;
			case COMMAND_REMOVE:
				coffeeFS.removeFile(coffeeFile);
				System.out.println("Removed the file \"" + coffeeFile + "\" from the Coffee file system image");
				break;
			case COMMAND_LIST:
				printFiles(coffeeFS.getFiles());
				break;
			case COMMAND_STATS:
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

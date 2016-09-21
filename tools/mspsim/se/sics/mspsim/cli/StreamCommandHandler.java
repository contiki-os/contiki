/*
 * Copyright (c) 2008, Swedish Institute of Computer Science.
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
 * $Id$
 *
 * -----------------------------------------------------------------
 *
 * StreamCommandHandler
 *
 * Authors : Joakim Eriksson, Niclas Finne
 * Created : 13 okt 2008
 * Updated : $Date$
 *           $Revision$
 */

package se.sics.mspsim.cli;
import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.InterruptedIOException;
import java.io.PrintStream;

/**
 *
 */
public class StreamCommandHandler extends CommandHandler implements Runnable {

  private BufferedReader inReader;
  private boolean workaround = false;
  private boolean exit;
  private String prompt;

  public StreamCommandHandler(InputStream in, PrintStream out, PrintStream err, String prompt) {
    super(out, err);
    this.prompt = prompt;
    this.exit = false;
    this.inReader = new BufferedReader(new InputStreamReader(in));
    registerCommand("workaround", new BasicCommand("activate workaround for Java console input bug", "") {
      public int executeCommand(CommandContext context) {
        workaround = true;
        return 0;
      }     
    });
  }
  
  public void setWorkaround(boolean w) {
    workaround = w;
  }

  public void start() {
    super.start();
    new Thread(this, "cmd").start();
  }

  private String readLine(BufferedReader inReader2) throws IOException {
    if (workaround) {
      StringBuilder str = new StringBuilder();
      while(true) {
        if (inReader2.ready()) {
          int c = inReader2.read();
          if (c == '\n') {
            return str.toString();
          }
          if (c != '\r') {
            str.append((char)c);
          }
        } else {
          try {
            Thread.sleep(500);
          } catch (InterruptedException e) {
            throw new InterruptedIOException();
          }
        }
      }
    } else {
      return inReader2.readLine();
    }
  }

  public void run() {
    String lastLine = null;
    while(!exit) {
      try {
        out.print(prompt);
        out.flush();
        String line = readLine(inReader);//.readLine();
        // Simple execution of last called command line when not running from terminal with history support
        if (((char) 27 + "[A").equals(line)) {
          line = lastLine;          
        }
        if (line != null && line.length() > 0) {
          lastLine = line;
          lineRead(line);
        }
      } catch (IOException e) {
        e.printStackTrace(err);
        err.println("Command line tool exiting...");
        exit = true;
      }
    }
  }

}

/**
 * Copyright (c) 2008, Swedish Institute of Computer Science.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
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
 * $Id: AsyncCommand.java 187 2008-03-17 19:34:12Z joxe $
 *
 * -----------------------------------------------------------------
 *
 *
 * Author  : Joakim Eriksson
 * Created : 9 april 2008
 * Updated : $Date: 2008-03-17 20:34:12 +0100 (Mon, 17 Mar 2008) $
 *           $Revision: 187 $
 */
package se.sics.mspsim.cli;

/**
 * @author joakime
 *
 */
public abstract class AbstractWindowDataHandler implements WindowDataHandler {

  public void handleCommand(String[] parts) {
    String cmd = parts[0];
    if ("set".equals(cmd)) {
      int index = atoi(parts[1], 0);
      String[] args = new String[parts.length - 3];
      System.arraycopy(parts, 3, args, 0, args.length);
      setProperty(index, parts[2], args);
    } else {
      System.err.println("unknown command: " + cmd);
    }
  }

  public abstract void setProperty(int index, String param, String[] args);

  public static int atoi(String data, int defaultValue) {
    try {
      return Integer.parseInt(data);
    } catch (NumberFormatException e) {
      return defaultValue;
    }
  }

  public static double atod(String data, double defaultValue) {
    try {
      return Double.parseDouble(data);
    } catch (NumberFormatException e) {
      return defaultValue;
    }
  }  
}

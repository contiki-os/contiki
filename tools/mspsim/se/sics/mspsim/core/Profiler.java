/**
 * Copyright (c) 2007, Swedish Institute of Computer Science.
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
 * $Id$
 *
 * -----------------------------------------------------------------
 *
 * MSP430
 *
 * Author  : Joakim Eriksson
 * Created : Sun Oct 21 22:00:00 2007
 * Updated : $Date$
 *           $Revision$
 */

package se.sics.mspsim.core;
import java.io.PrintStream;
import java.util.Properties;

import se.sics.mspsim.profiler.CallListener;
import se.sics.mspsim.util.MapEntry;

public interface Profiler {

  public static final String PARAM_FUNCTION_NAME_REGEXP = "function.regexp";
  public static final String PARAM_PROFILE_CALLERS = "showcallers";
  public static final String PARAM_SORT_MODE = "sortmode";
  
  public void setCPU(MSP430Core cpu);

  public void profileCall(MapEntry entry, long cycles, int from);
  public void profileReturn(long cycles);
  public void profileInterrupt(int vector, long cycles);
  public void profileRETI(long cycles);

  public void resetProfile();
  public void clearProfile();

  public void addCallListener(CallListener listener);
  public void removeCallListener(CallListener listener);
  
  public void printProfile(PrintStream out);

  public void printProfile(PrintStream out, Properties parameters);

  public void printStackTrace(PrintStream out);
  
  /* function call logging */
  public void setLogger(PrintStream out);

  public String getCall(int i);

}

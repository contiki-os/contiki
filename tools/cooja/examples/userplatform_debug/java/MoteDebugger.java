/*
 * Copyright (c) 2006, Swedish Institute of Computer Science.
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
 * $Id: MoteDebugger.java,v 1.2 2006/09/06 12:26:33 fros4943 Exp $
 */

import java.awt.event.*;
import java.util.regex.Matcher;
import java.util.regex.Pattern;
import javax.swing.*;
import org.apache.log4j.Logger;
import java.io.*;
import java.lang.management.*;

import se.sics.cooja.*;
import se.sics.cooja.contikimote.ContikiMote;
import se.sics.cooja.contikimote.ContikiMoteType;

/**
 * Mote debugger lets a user debug a mote using an external debugger.
 * 
 * It executes the external program 'gdb' and sets up breakpoints
 * at the entry of the tick function.
 *
 * The selected mote is then set to state active and ticked.
 * 
 * OBSERVE! Experimental code. Not fully tested yet!
 * 
 * @author Fredrik Osterlind
 */
@ClassDescription("Debug using GDB")
@VisPluginType(VisPluginType.MOTE_PLUGIN)
public class MoteDebugger extends VisPlugin {;
  
  private static final long serialVersionUID = 1L;
  private static Logger logger = Logger.getLogger(MoteDebugger.class);
  private ContikiMote moteToDebug;
  
  /**
   * Creates a new VisDebug.
   * @param mote Contiki mote to debug next tick
   */
  public MoteDebugger(Mote mote) {
    super("VisDebug (" + mote + ")");
    this.moteToDebug = (ContikiMote) mote;
    
    JButton debugButton = new JButton("Debug now");
    debugButton.addActionListener(new ActionListener() {
      public void actionPerformed(ActionEvent e) {
        logger.warn("OBSERVE! This is experimental code");
        
        logger.info("Getting JVM pid");
        RuntimeMXBean rt = ManagementFactory.getRuntimeMXBean();
        String runtimeName = rt.getName();
        int pid = 1;
        
        String pidExtraction = "^([0-9]*)[^$]*$";
        Pattern pattern = Pattern.compile(pidExtraction);
        Matcher matcher = pattern.matcher(runtimeName);
        if (!matcher.find()) {
          logger.fatal("Could not determine pid, aborting");
          return;
        }
        
        pid = Integer.parseInt(matcher.group(1));
        if (pid <= 0) {
          logger.fatal("Pid seems to be strange, aborting. pid=" + pid);
          return;
        }
        
        logger.info("Extracted PID=" + pid);
        
        logger.info("Checking that source code file exists..");
        File sourceFile = new File(ContikiMoteType.tempOutputDirectory, moteToDebug.getType().getIdentifier() + ".c");
        if (!sourceFile.exists()) {
          logger.fatal("Can't find source file: " + sourceFile);
          return;
        }
        
        logger.info("Source file ok: " + sourceFile);
        
        
        logger.info("Determining function name to break at (entry of tick)");
        String libName = ((ContikiMoteType) moteToDebug.getType()).getLibraryClassName();
        
        String functionName = "Java." + CoreComm.class.getPackage().getName() + ".corecomm." + libName + ".tick";
        functionName = functionName.replaceAll("\\.", "_");
        
        logger.info("Function name is: " + functionName);
        
        logger.info("Creating temporary file .tmp with initial commands");
        File tmpFile = new File(ContikiMoteType.tempOutputDirectory, ".tmp");
        if (tmpFile.exists()) {
          tmpFile.delete();
        }
        try {
          BufferedWriter tmpStream = new BufferedWriter(
              new OutputStreamWriter(
                  new FileOutputStream(
                      tmpFile)));
          tmpStream.write("break " + functionName + "\n");
          tmpStream.write("cont\n");
          tmpStream.close();
        } catch (Exception ex) {
          logger.fatal("Could not create temporary command file: " + tmpFile);
          return;
        }
        
        logger.info("Command file created ok: " + tmpFile.getName());
        
        logger.info("Starting external GDB");
        logger.info("> GDB must be exited before control is returned to COOJA");
        logger.info("> Use command 'quit' followed by y to exit GDB");
        
        Process gdbProcess = null;
        try {
          gdbProcess = Runtime.getRuntime().exec("xterm -e gdb"
              + " -nw -quiet "
              + " --pid=" + pid
              + " -x " + ContikiMoteType.tempOutputDirectory.getName() + "/" + tmpFile.getName()
          );
          
          logger.info("Sleeping 2500 ms while starting up GDB");
          Thread.sleep(2500);
          logger.info("Ticking chosen mote now! (setting state to active)");
          moteToDebug.setState(Mote.STATE_ACTIVE);
          moteToDebug.tick(GUI.currentSimulation.getSimulationTime());
          
          gdbProcess.waitFor();
        } catch (Exception ex) {
          logger.fatal("Exception while starting gdb, aborting");
        }
        
        logger.debug("GDB terminated with exit code: " + gdbProcess.exitValue());
      }
    });

    add(debugButton); 
    setSize(250, 80);
  }
  
  public void closePlugin() {
  }
  
}

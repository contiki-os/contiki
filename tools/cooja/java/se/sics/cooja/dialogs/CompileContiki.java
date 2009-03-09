/*
 * Copyright (c) 2009, Swedish Institute of Computer Science.
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
 * $Id: CompileContiki.java,v 1.1 2009/03/09 13:32:50 fros4943 Exp $
 */

package se.sics.cooja.dialogs;

import java.io.BufferedReader;
import java.io.File;
import java.io.IOException;
import java.io.InputStreamReader;

import javax.swing.Action;

import org.apache.log4j.Logger;

import se.sics.cooja.MoteType.MoteTypeCreationException;
import se.sics.cooja.dialogs.MessageList;

/**
 * Contiki compiler library.
 * Uses notion of Contiki platforms to compile a Contiki firmware.
 *
 * @author Fredrik Osterlind
 */
public class CompileContiki {
  private static Logger logger = Logger.getLogger(CompileContiki.class);

  /**
   * Execute Contiki compilation command.
   *
   * @param command Single compilation command
   * @param outputFile (Optional) Expected output file
   * @param directory Directory in which to execute command
   * @param onSuccess Action called if compilation succeeds
   * @param onFailure Action called if compilation fails
   * @param compilationOutput Is written both std and err process output
   * @param synchronous If true, method blocks until process completes
   *
   * @return Sub-process if called asynchronously
   * @throws Exception If process returns error, or outputFile does not exist
   */
  public static Process compile(
      final String command,
      final File outputFile,
      final File directory,
      final Action onSuccess,
      final Action onFailure,
      final MessageList compilationOutput,
      boolean synchronous)
  throws Exception {

    compilationOutput.addMessage("", MessageList.NORMAL);
    compilationOutput.addMessage("> " + command, MessageList.NORMAL);
    logger.info("> " + command);

    final Process compileProcess;
    try {
      /* TODO Split into correct arguments: parse " and ' */
      compileProcess = Runtime.getRuntime().exec(command, null, directory);

      final BufferedReader processNormal = new BufferedReader(
          new InputStreamReader(compileProcess.getInputStream()));
      final BufferedReader processError = new BufferedReader(
          new InputStreamReader(compileProcess.getErrorStream()));

      if (outputFile != null) {
        if (outputFile.exists()) {
          outputFile.delete();
        }
        if (outputFile.exists()) {
          compilationOutput.addMessage("Error when deleting old " + outputFile.getName(), MessageList.ERROR);
          if (onFailure != null) {
            onFailure.actionPerformed(null);
          }
          throw new MoteTypeCreationException("Error when deleting old " + outputFile.getName());
        }
      }

      Thread readInput = new Thread(new Runnable() {
        public void run() {
          try {
            String readLine;
            while ((readLine = processNormal.readLine()) != null) {
              if (compilationOutput != null) {
                compilationOutput.addMessage(readLine, MessageList.NORMAL);
              }
            }
          } catch (IOException e) {
            logger.warn("Error while reading from process");
          }
        }
      }, "read input stream thread");

      Thread readError = new Thread(new Runnable() {
        public void run() {
          try {
            String readLine;
            while ((readLine = processError.readLine()) != null) {
              if (compilationOutput != null) {
                compilationOutput.addMessage(readLine, MessageList.ERROR);
              }
            }
          } catch (IOException e) {
            logger.warn("Error while reading from process");
          }
        }
      }, "read error stream thread");

      final MoteTypeCreationException syncException = new MoteTypeCreationException("");
      Thread handleCompilationResultThread = new Thread(new Runnable() {
        public void run() {

          /* Wait for compilation to end */
          try {
            compileProcess.waitFor();
          } catch (Exception e) {
            compilationOutput.addMessage(e.getMessage(), MessageList.ERROR);
            syncException.setCompilationOutput(new MessageList());
            syncException.fillInStackTrace();
            return;
          }

          /* Check return value */
          if (compileProcess.exitValue() != 0) {
            compilationOutput.addMessage("Process returned error code " + compileProcess.exitValue(), MessageList.ERROR);
            if (onFailure != null) {
              onFailure.actionPerformed(null);
            }
            syncException.setCompilationOutput(new MessageList());
            syncException.fillInStackTrace();
            return;
          }

          if (outputFile == null) {
            /* No firmware to generate: OK */
            if (onSuccess != null) {
              onSuccess.actionPerformed(null);
            }
            return;
          }

          if (!outputFile.exists()) {
            compilationOutput.addMessage("No firmware file: " + outputFile, MessageList.ERROR);
            if (onFailure != null) {
              onFailure.actionPerformed(null);
            }
            syncException.setCompilationOutput(new MessageList());
            syncException.fillInStackTrace();
            return;
          }

          compilationOutput.addMessage("", MessageList.NORMAL);
          compilationOutput.addMessage("Compilation succeded", MessageList.NORMAL);
          if (onSuccess != null) {
            onSuccess.actionPerformed(null);
          }
        }
      }, "handle compilation results");

      readInput.start();
      readError.start();
      handleCompilationResultThread.start();

      if (synchronous) {
        try {
          handleCompilationResultThread.join();
        } catch (Exception e) {
          throw (MoteTypeCreationException) new MoteTypeCreationException(
              "Compilation error: " + e.getMessage()).initCause(e);
        }

        /* Detect error manually */
        if (syncException.hasCompilationOutput()) {
          throw (MoteTypeCreationException) new MoteTypeCreationException(
          "Bad return value").initCause(syncException);
        }
      }
    } catch (IOException ex) {
      if (onFailure != null) {
        onFailure.actionPerformed(null);
      }
      throw (MoteTypeCreationException) new MoteTypeCreationException(
          "Compilation error: " + ex.getMessage()).initCause(ex);
    }

    return compileProcess;
  }

}

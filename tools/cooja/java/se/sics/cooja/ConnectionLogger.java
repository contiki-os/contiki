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
 * $Id: ConnectionLogger.java,v 1.2 2006/12/11 16:37:11 fros4943 Exp $
 */

package se.sics.cooja;

import java.io.File;
import java.io.FileOutputStream;
import org.apache.log4j.Logger;

import se.sics.cooja.interfaces.Position;
import se.sics.cooja.plugins.SimControl;

/**
 * ConnectionLogger is a simple connection information outputter. All
 * connections given via the method logConnection will be written to either
 * default Log4J info stream, a log file or both.
 * 
 * Log files have the following structure (spaces are tabs): SRC_POS [src_x]
 * [src_y] [src_z] SRC_DATA [sent data bytes] DEST_POS [dest_x] [dest_y]
 * [dest_z] DEST_DATA [received data bytes] [newline]
 * 
 * @see RadioConnection
 * @see RadioMedium
 * 
 * @author Fredrik Osterlind
 */
public class ConnectionLogger {
  private static final long serialVersionUID = 1L;
  private static Logger logger = Logger.getLogger(SimControl.class);

  private static int LOG_TO_FILE = 1;
  private static int LOG_TO_LOG4J = 2;
  private static int LOG_TO_FILE_AND_LOG4J = 3;

  private int myType;
  private File myFile;

  /**
   * Creates a new connection logger outputting to Log4J info stream.
   */
  public ConnectionLogger() {
    myType = LOG_TO_LOG4J;
  }

  /**
   * Creates a new connection logger outputting to a file.
   * 
   * @param logFile
   *          Log file
   */
  public ConnectionLogger(File logFile) {
    myType = LOG_TO_FILE;
    myFile = logFile;
    if (myFile.exists())
      myFile.delete();
  }

  /**
   * Output given connection to either Log4J info stream or a file.
   * 
   * @param conn
   *          Connection to output
   */
  public void logConnection(RadioConnection conn) {

    if (myType == LOG_TO_LOG4J || myType == LOG_TO_FILE_AND_LOG4J) {
      if (conn.getDestinationPositons() != null
          && conn.getDestinationPositons().length > 0)
        for (Position destPos : conn.getDestinationPositons()) {
          logger.info("RADIODATA from " + conn.getSourcePosition() + " to "
              + destPos + "\tsize:" + conn.getSourceData().length);
        }
      else
        logger.info("RADIODATA from " + conn.getSourcePosition() + " to "
            + "[NOWHERE]" + "\tsize:" + conn.getSourceData().length);
    }
    if (myType == LOG_TO_FILE || myType == LOG_TO_FILE_AND_LOG4J) {

      try {
        FileOutputStream out = new FileOutputStream(myFile, true);

        if (conn.getDestinationPositons() != null
            && conn.getDestinationPositons().length > 0) {
          for (int i = 0; i < conn.getDestinationPositons().length; i++) {
            // Source pos
            out.write("SRC_POS\t".getBytes());
            Position pos = conn.getSourcePosition();
            out.write(Double.toString(pos.getXCoordinate()).getBytes());
            out.write("\t".getBytes());
            out.write(Double.toString(pos.getYCoordinate()).getBytes());
            out.write("\t".getBytes());
            out.write(Double.toString(pos.getZCoordinate()).getBytes());
            out.write("\t".getBytes());

            // Source data
            out.write("SRC_DATA\t".getBytes());
            for (byte b : conn.getSourceData()) {
              String hexString = Integer.toHexString((int) b);
              if (hexString.length() == 1) hexString = "0" + hexString;
              out.write(hexString.getBytes());
            }
            out.write("\t".getBytes());

            // Destination pos
            out.write("DEST_POS\t".getBytes());
            pos = conn.getDestinationPositons()[i];
            out.write(Double.toString(pos.getXCoordinate()).getBytes());
            out.write("\t".getBytes());
            out.write(Double.toString(pos.getYCoordinate()).getBytes());
            out.write("\t".getBytes());
            out.write(Double.toString(pos.getZCoordinate()).getBytes());
            out.write("\t".getBytes());

            // Source data
            out.write("DEST_DATA\t".getBytes());
            for (byte b : conn.getDestinationData()[i]) {
              String hexString = Integer.toHexString((int) b);
              if (hexString.length() == 1) hexString = "0" + hexString;
              out.write(hexString.getBytes());
            } out.write("\t".getBytes());

            out.write("\n".getBytes());
          }

        } else {
          // Source pos
          out.write("SRC_POS\t".getBytes());
          Position pos = conn.getSourcePosition();
          out.write(Double.toString(pos.getXCoordinate()).getBytes());
          out.write("\t".getBytes());
          out.write(Double.toString(pos.getYCoordinate()).getBytes());
          out.write("\t".getBytes());
          out.write(Double.toString(pos.getZCoordinate()).getBytes());
          out.write("\t".getBytes());

          // Source data
          out.write("SRC_DATA\t".getBytes());
          for (byte b : conn.getSourceData()) {
            String hexString = Integer.toHexString((int) b);
            if (hexString.length() == 1) hexString = "0" + hexString;
            out.write(hexString.getBytes());
          }
          out.write("\n".getBytes());
        }
        out.close();

      } catch (Exception e) {
        logger.fatal("Exception while logging to file: " + e);
        myType = LOG_TO_LOG4J;
        return;
      }
    }

  }
}

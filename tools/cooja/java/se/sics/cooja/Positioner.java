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
 * $Id: Positioner.java,v 1.3 2009/02/18 13:55:01 fros4943 Exp $
 */

package se.sics.cooja;

import java.lang.reflect.Constructor;
import java.lang.reflect.InvocationTargetException;
import org.apache.log4j.Logger;

/**
 * A positioner is used for determining initial positions of motes.
 *
 * @author Fredrik Osterlind
 */
public abstract class Positioner {
  private static Logger logger = Logger.getLogger(Positioner.class);

  /**
   * This method creates an instance of the given class with the given interval
   * information as constructor arguments. Instead of calling the constructors
   * directly this method may be used.
   *
   * @param positionerClass
   *          Positioner class
   * @param totalNumberOfMotes
   *          Total number of motes that should be generated using this
   *          positioner
   * @param startX
   *          Lowest X value of positions generated using returned positioner
   * @param endX
   *          Highest X value of positions generated using returned positioner
   * @param startY
   *          Lowest Y value of positions generated using returned positioner
   * @param endY
   *          Highest Y value of positions generated using returned positioner
   * @param startZ
   *          Lowest Z value of positions generated using returned positioner
   * @param endZ
   *          Highest Z value of positions generated using returned positioner
   * @return Postioner instance
   */
  public static final Positioner generateInterface(
      Class<? extends Positioner> positionerClass, int totalNumberOfMotes,
      double startX, double endX,
      double startY, double endY,
      double startZ, double endZ) {
    try {
      Constructor<? extends Positioner> constr =
        positionerClass.getConstructor(new Class[] {
          int.class,
          double.class, double.class,
          double.class, double.class,
          double.class, double.class
        });
      return constr.newInstance(new Object[] {
          totalNumberOfMotes,
          startX, endX,
          startY, endY,
          startZ, endZ
      });
    } catch (Exception e) {

      if (e instanceof InvocationTargetException) {
        logger.fatal("Exception when creating " + positionerClass + ": " + e.getCause());
      } else {
        logger.fatal("Exception when creating " + positionerClass + ": " + e.getMessage());
      }
      return null;
    }
  }

  /**
   * Returns the next mote position.
   *
   * @return Position
   */
  public abstract double[] getNextPosition();

}

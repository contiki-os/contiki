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
 * $Id: LinearPositioner.java,v 1.1 2006/08/21 12:13:11 fros4943 Exp $
 */

package se.sics.cooja.positioners;
import se.sics.cooja.*;

/**
 * Generates positions linearly distributed in a given interval.
 *
 * @author Fredrik Osterlind
 */
@ClassDescription("Linear positioning")
public class LinearPositioner extends Positioner {

  private double startX, startY, startZ;
  private int addedInX, addedInY, addedInZ;
  private int numberInX, numberInY, numberInZ;
  private double xInterval, yInterval, zInterval;

  /**
   * Creates a linear positioner.
   *
   * @param totalNumberOfMotes Total number of motes
   * @param startX X interval start
   * @param endX X interval end
   * @param startY Y interval start
   * @param endY Y interval end
   * @param startZ Z interval start
   * @param endZ Z interval end
   */
  public LinearPositioner(int totalNumberOfMotes,
			  double startX, double endX,
			  double startY, double endY,
			  double startZ, double endZ) {

    this.startX = startX;
    this.startY = startY;
    this.startZ = startZ;

    double widthX = endX - startX;
    double widthY = endY - startY;
    double widthZ = endZ - startZ;

    if (widthX == 0) {
      widthX = -1;
    }
    if (widthY == 0) {
      widthY = -1;
    }
    if (widthZ == 0) {
      widthZ = -1;
    }

    double totalSpace = Math.abs(widthX * widthY * widthZ);
    double spaceOfEachMote = totalSpace / (double) totalNumberOfMotes;

    int noDimensions = 0;
    if (widthX > 0)
      noDimensions++;
    if (widthY > 0)
      noDimensions++;
    if (widthZ > 0)
      noDimensions++;

    double sideLengthOfEachMote = Math.pow(spaceOfEachMote, 1.0/(double) noDimensions);

    this.numberInX = 0;
    this.numberInY = 0;
    this.numberInZ = 0;

    if (widthX > 0)
      numberInX = (int) (widthX / sideLengthOfEachMote);

    if (widthY > 0)
      numberInY = (int) (widthY / sideLengthOfEachMote);

    if (widthZ > 0)
      numberInZ = (int) (widthZ / sideLengthOfEachMote);

    this.xInterval = widthX / numberInX;
    this.yInterval = widthY / numberInY;
    this.zInterval = widthZ / numberInZ;

    if (numberInX == 0)
      xInterval = 0.0;

    if (numberInY == 0)
      yInterval = 0.0;

    if (numberInZ == 0)
      zInterval = 0.0;

    this.addedInX = 0;
    this.addedInY = 0;
    this.addedInZ = 0;
  }

  public double[] getNextPosition() {
    double[] newPosition = new double[] {
        startX + addedInX*xInterval,
        startY + addedInY*yInterval,
        startZ + addedInZ*zInterval
    };

    addedInZ++;

    if (addedInZ >= numberInZ) {
      addedInY++;
      addedInZ = 0;
    }

    if (addedInY >= numberInY) {
      addedInX++;
      addedInY = 0;
    }

    return newPosition;
  }

}

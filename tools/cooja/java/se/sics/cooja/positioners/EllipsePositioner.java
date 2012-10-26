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
 */

package se.sics.cooja.positioners;
import se.sics.cooja.*;

/**
 * Generates positions in a ellipse in the XY plane.
 * (z position will always be start value of interval)
 *
 * @author Fredrik Osterlind
 */
@ClassDescription("Ellipse positioning")
public class EllipsePositioner extends Positioner {

  private double xRadius, yRadius, xMiddle, yMiddle;
  private double zValue, deltaAngle, currentAngle;

  /**
   * Creates a circle positioner.
   * @param totalNumberOfMotes Total number of motes
   * @param startX X interval start
   * @param endX X interval end
   * @param startY Y interval start
   * @param endY Y interval end
   * @param startZ Z interval start
   * @param endZ Z interval end
   */
  public EllipsePositioner(int totalNumberOfMotes,
			   double startX, double endX,
			   double startY, double endY,
			   double startZ, double endZ) {
    xRadius = (endX - startX) / 2.0;
    yRadius = (endY - startY) / 2.0;
    xMiddle = startX + xRadius;
    yMiddle = startY + yRadius;
    zValue = startZ;
    deltaAngle = 2*Math.PI / (double) totalNumberOfMotes;
    currentAngle = 0;
  }

  public double[] getNextPosition() {
    currentAngle += deltaAngle;
    return new double[] {
        xMiddle + xRadius * Math.cos(currentAngle),
        yMiddle + yRadius * Math.sin(currentAngle),
        zValue};
  }

}

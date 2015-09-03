/*
 * Copyright (c) 2011, Swedish Institute of Computer Science. All rights
 * reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer. 2. Redistributions in
 * binary form must reproduce the above copyright notice, this list of
 * conditions and the following disclaimer in the documentation and/or other
 * materials provided with the distribution. 3. Neither the name of the
 * Institute nor the names of its contributors may be used to endorse or promote
 * products derived from this software without specific prior written
 * permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

package org.contikios.cooja.interfaces;

/**
 * Directional antenna.
 *
 * @see MRM
 * @author Fredrik Osterlind
 */
public interface DirectionalAntennaRadio {

  /**
   * @return Current direction (radians). Typically direction 0 has the maximum
   * relative gain.
   *
   * @see #getRelativeGain(double)
   */
  public double getDirection();

  /**
   * Relative gain (dB) as compared to an omnidirectional antenna.
   * Note that the given angle is relative to the current direction!
   *
   * @see #getDirection()
   * @param radians Angle relative to current direction
   * @param distance Distance from antenna
   * @return
   */
  public double getRelativeGain(double radians, double distance);

  public void addDirectionChangeListener(DirectionChangeListener l);
  public void removeDirectionChangeListener(DirectionChangeListener l);

  public interface DirectionChangeListener {
    public void newDirection(DirectionalAntennaRadio radio, double direction);
  }

}

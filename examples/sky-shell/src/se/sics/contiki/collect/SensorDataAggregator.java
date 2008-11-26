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
 * $Id: SensorDataAggregator.java,v 1.2 2008/11/26 14:22:54 nifi Exp $
 *
 * -----------------------------------------------------------------
 *
 * SensorDataAggregator
 *
 * Authors : Joakim Eriksson, Niclas Finne
 * Created : 20 aug 2008
 * Updated : $Date: 2008/11/26 14:22:54 $
 *           $Revision: 1.2 $
 */

package se.sics.contiki.collect;

/**
 *
 */
public class SensorDataAggregator implements SensorInfo {

  private final Node node;
  private long[] values;
  private int dataCount;

  public SensorDataAggregator(Node node) {
    this.node = node;
    this.values = new long[VALUES_COUNT];
  }

  public Node getNode() {
    return node;
  }

  public String getNodeID() {
    return node.getID();
  }

  public long getValue(int index) {
    return values[index];
  }

  public long getAverageValue(int index) {
    return dataCount > 0 ? values[index] / dataCount : 0;
  }

  public int getValueCount() {
    return values.length;
  }

  public int getDataCount() {
    return dataCount;
  }

  public void addSensorData(SensorData data) {
    for (int i = 0, n = Math.min(VALUES_COUNT, data.getValueCount()); i < n; i++) {
      values[i] += data.getValue(i);
    }
    dataCount++;
  }

  public void clear() {
    for (int i = 0, n = values.length; i < n; i++) {
      values[i] = 0L;
    }
    dataCount = 0;
  }

  public String toString() {
    StringBuilder sb = new StringBuilder();
    for (int i = 0, n = values.length; i < n; i++) {
      if (i > 0) sb.append(' ');
      sb.append(values[i]);
    }
    return sb.toString();
  }

  public double getCPUPower() {
    return (values[TIME_CPU] * POWER_CPU) / (values[TIME_CPU] + values[TIME_LPM]);
  }

  public double getLPMPower() {
    return (values[TIME_LPM] * POWER_LPM) / (values[TIME_CPU] + values[TIME_LPM]);
  }

  public double getListenPower() {
    return (values[TIME_LISTEN] * POWER_LISTEN) / (values[TIME_CPU] + values[TIME_LPM]);
  }

  public double getTransmitPower() {
    return (values[TIME_TRANSMIT] * POWER_TRANSMIT) / (values[TIME_CPU] + values[TIME_LPM]);
  }

  public double getAveragePower() {
    return (values[TIME_CPU] * POWER_CPU + values[TIME_LPM] * POWER_LPM
    + values[TIME_LISTEN] * POWER_LISTEN + values[TIME_TRANSMIT] * POWER_TRANSMIT)
    / (values[TIME_CPU] + values[TIME_LPM]);
  }

  public long getPowerMeasureTime() {
    return (1000L * (values[TIME_CPU] + values[TIME_LPM])) / TICKS_PER_SECOND;
  }

  public double getAverageTemperature() {
    return dataCount > 0 ? (-39.6 + 0.01 * (values[TEMPERATURE] / dataCount)) : 0.0;
  }

  public double getAverageRadioIntensity() {
    return getAverageValue(RSSI);
  }

  public double getAverageLatency() {
    return getAverageValue(LATENCY) / 4096.0;
  }

  public double getAverageHumidity() {
    double v = 0.0;
    if (dataCount > 0) {
      v = -4.0 + 405.0 * (values[HUMIDITY] / dataCount) / 10000.0;
    }
    return v > 100 ? 100 : v;
  }

  public double getAverageLight1() {
    return 10.0 * getAverageValue(LIGHT1) / 7.0;
  }

  public double getAverageLight2() {
    return 46.0 * getAverageValue(LIGHT2) / 10.0;
  }

  public double getAverageBestNeighborETX() {
    return getAverageValue(BEST_NEIGHBOR_ETX) / 16.0;
  }

}

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
 * $Id: SensorData.java,v 1.7 2010/02/04 16:21:47 adamdunkels Exp $
 *
 * -----------------------------------------------------------------
 *
 * SensorData
 *
 * Authors : Joakim Eriksson, Niclas Finne
 * Created : 3 jul 2008
 * Updated : $Date: 2010/02/04 16:21:47 $
 *           $Revision: 1.7 $
 */

package se.sics.contiki.collect;
import java.util.Arrays;

/**
 *
 */
public class SensorData implements SensorInfo {

  private final Node node;
  private final int[] values;
  private final long nodeTime;
  private final long systemTime;

  public SensorData(Node node, int[] values, long systemTime) {
    this.node = node;
    this.values = values;
    this.nodeTime = ((values[TIMESTAMP1] << 16) + values[TIMESTAMP2]) * 1000L;
    this.systemTime = systemTime;
  }

  public Node getNode() {
    return node;
  }

  public String getNodeID() {
    return node.getID();
  }

  public int getValue(int index) {
    return values[index];
  }

  public int getValueCount() {
    return values.length;
  }

  public long getNodeTime() {
    return nodeTime;
  }

  public long getSystemTime() {
    return systemTime;
  }

  public String toString() {
    StringBuilder sb = new StringBuilder();
    if (systemTime > 0L) {
      sb.append(systemTime).append(' ');
    }
    for (int i = 0, n = values.length; i < n; i++) {
      if (i > 0) sb.append(' ');
      sb.append(values[i]);
    }
    return sb.toString();
  }

  public static SensorData parseSensorData(CollectServer server, String line) {
    return parseSensorData(server, line, 0);
  }

  public static SensorData parseSensorData(CollectServer server, String line, long systemTime) {
    String[] components = line.trim().split(" ");
    if (components.length == SensorData.VALUES_COUNT + 1) {
      // Sensor data with system time
      try {
        systemTime = Long.parseLong(components[0]);
        components = Arrays.copyOfRange(components, 1, components.length);
      } catch (NumberFormatException e) {
        // First column does not seem to be system time
      }
    }
    if (components.length != SensorData.VALUES_COUNT) {
      return null;
    }
    // Sensor data line (probably)
    int[] data = parseToInt(components);
    if (data == null || data[0] != VALUES_COUNT) {
      System.err.println("Failed to parse data line: '" + line + "'");
      return null;
    }
    String nodeID = mapNodeID(data[NODE_ID]);
    Node node = server.addNode(nodeID);
    return new SensorData(node, data, systemTime);
  }

  public static String mapNodeID(int nodeID) {
    return "" + (nodeID & 0xff) + '.' + ((nodeID >> 8) & 0xff);
  }

  private static int[] parseToInt(String[] text) {
    try {
      int[] data = new int[text.length];
      for (int i = 0, n = data.length; i < n; i++) {
        data[i] = Integer.parseInt(text[i]);
      }
      return data;
    } catch (NumberFormatException e) {
      return null;
    }
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

  public double getTemperature() {
    return -39.6 + 0.01 * values[TEMPERATURE];
  }

  public double getBatteryVoltage() {
    return values[BATTERY_VOLTAGE] * 2 * 2.5 / 4096.0;
  }

  public double getBatteryIndicator() {
    return values[BATTERY_INDICATOR];
  }

  public double getRadioIntensity() {
    return values[RSSI];
  }

  public double getLatency() {
    return values[LATENCY] / 8192.0;
  }

  public double getHumidity() {
    double v = -4.0 + 405.0 * values[HUMIDITY] / 10000.0;
    if(v > 100) {
      return 100;
    } else {
      return v;
    }
  }

  public double getLight1() {
    return 10.0 * values[LIGHT1] / 7.0;
  }

  public double getLight2() {
    return 46.0 * values[LIGHT2] / 10.0;
  }

  public String getBestNeighborID() {
    return values[BEST_NEIGHBOR] > 0 ? mapNodeID(values[BEST_NEIGHBOR]): null;
  }

  public double getBestNeighborETX() {
    return values[BEST_NEIGHBOR_ETX] / 16.0;
  }

}

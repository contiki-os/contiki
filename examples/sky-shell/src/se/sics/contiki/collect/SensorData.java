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
 * $Id: SensorData.java,v 1.2 2008/08/15 18:47:13 adamdunkels Exp $
 *
 * -----------------------------------------------------------------
 *
 * SensorData
 *
 * Authors : Joakim Eriksson, Niclas Finne
 * Created : 3 jul 2008
 * Updated : $Date: 2008/08/15 18:47:13 $
 *           $Revision: 1.2 $
 */

package se.sics.contiki.collect;


/**
 *
 */
public class SensorData {

  public static final int TICKS_PER_SECOND = 4096;
  private static final double VOLTAGE = 3;
  private static final double POWER_CPU = 1.800 * VOLTAGE; /* mW */
  private static final double POWER_LPM = 0.0545 * VOLTAGE; /* mW */
  private static final double POWER_TRANSMIT = 17.7 * VOLTAGE; /* mW */
  private static final double POWER_LISTEN = 20.0 * VOLTAGE; /* mW */

  public static final int DATA_LEN = 0;
  public static final int TIMESTAMP1 = 1;
  public static final int TIMESTAMP2 = 2;
  public static final int TIMESYNCTIMESTAMP = 3;
  public static final int NODE_ID = 4;
  public static final int SEQNO = 5;
  public static final int HOPS = 6;
  public static final int LATENCY = 7;
  public static final int DATA_LEN2 = 8;
  public static final int CLOCK = 9;
  public static final int TIMESYNCHTIME = 10;
  public static final int LIGHT1 = 11;
  public static final int LIGHT2 = 12;
  public static final int TEMPERATURE = 13;
  public static final int HUMIDITY = 14;
  public static final int RSSI = 15;
  public static final int TIME_CPU = 16;
  public static final int TIME_LPM = 17;
  public static final int TIME_TRANSMIT = 18;
  public static final int TIME_LISTEN = 19;
  public static final int BEST_NEIGHBOR = 20;
  public static final int BEST_NEIGHBOR_ETX = 21;
  public static final int BEST_NEIGHBOR_RTMETRIC = 22;

  public static final int VALUES_COUNT = 23;

  private final Node node;
  private final int[] values;
  private final long time;

  public SensorData(Node node, int[] values) {
    this.node = node;
    this.values = values;
    this.time = ((values[TIMESTAMP1] << 16) + values[TIMESTAMP2]) * 1000L;
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

  public long getTime() {
    return time;
  }

  public String toString() {
    StringBuilder sb = new StringBuilder();
    for (int i = 0, n = values.length; i < n; i++) {
      if (i > 0) sb.append(' ');
      sb.append(values[i]);
    }
    return sb.toString();
  }

  public static SensorData parseSensorData(CollectServer server, String line) {
    String[] components = line.split(" ");
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
    return new SensorData(node, data);
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
    return (1000 * (values[TIME_CPU] + values[TIME_LPM])) / TICKS_PER_SECOND;
  }

  public double getTemperature() {
    return -39.6 + 0.01 * values[TEMPERATURE];
  }

  public double getRadioIntensity() {
    return values[RSSI];
  }

  public double getLatency() {
    return values[LATENCY] / 4096.0;
  }

  public double getHumidity() {
    double v;
//    double v = values[HUMIDITY];
//    double humidity = -4.0 + 0.0405 * v + -0.0000028 * v * v;
//    // Correct humidity using temperature compensation
//    return (getTemperature() - 25) * (0.01 + 0.00008*v + humidity);
    v = -4.0 + 405.0 * values[HUMIDITY] / 10000.0;
    if(v > 100) {
      return 100;
    } else {
      return v;
    }
  }

  public double getLight1() {
//    double v = (values[LIGHT1] * VOLTAGE) / 4096.0;
//    return 0.625 * 1000000 * v * 10;
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

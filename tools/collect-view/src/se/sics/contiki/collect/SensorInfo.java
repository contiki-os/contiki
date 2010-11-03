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
 * $Id: SensorInfo.java,v 1.1 2010/11/03 14:53:05 adamdunkels Exp $
 *
 * -----------------------------------------------------------------
 *
 * SensorInfo
 *
 * Authors : Joakim Eriksson, Niclas Finne
 * Created : 20 aug 2008
 * Updated : $Date: 2010/11/03 14:53:05 $
 *           $Revision: 1.1 $
 */

package se.sics.contiki.collect;


/**
 *
 */
public interface SensorInfo {

  public static final long TICKS_PER_SECOND = 4096L;
  public static final double VOLTAGE = 3;
  public static final double POWER_CPU = 1.800 * VOLTAGE;       /* mW */
  public static final double POWER_LPM = 0.0545 * VOLTAGE;      /* mW */
  public static final double POWER_TRANSMIT = 17.7 * VOLTAGE;   /* mW */
  public static final double POWER_LISTEN = 20.0 * VOLTAGE;     /* mW */

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
  public static final int TIME_CPU = 11;
  public static final int TIME_LPM = 12;
  public static final int TIME_TRANSMIT = 13;
  public static final int TIME_LISTEN = 14;
  public static final int BEST_NEIGHBOR = 15;
  public static final int BEST_NEIGHBOR_ETX = 16;
  public static final int RTMETRIC = 17;
  public static final int NUM_NEIGHBORS = 18;
  public static final int BEACON_INTERVAL = 19;
  public static final int BATTERY_VOLTAGE = 20;
  public static final int BATTERY_INDICATOR = 21;
  public static final int LIGHT1 = 22;
  public static final int LIGHT2 = 23;
  public static final int TEMPERATURE = 24;
  public static final int HUMIDITY = 25;
  public static final int RSSI = 26;

  public static final int VALUES_COUNT = 30;

}

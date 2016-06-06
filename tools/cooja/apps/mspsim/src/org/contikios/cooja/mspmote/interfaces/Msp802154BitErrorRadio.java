/*
 * Copyright (c) 2014, Uppsala University
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

package org.contikios.cooja.mspmote.interfaces;

import java.util.Random;

import org.apache.log4j.Logger;

import org.contikios.cooja.ClassDescription;
import org.contikios.cooja.Mote;
import org.contikios.cooja.Simulation;
import org.contikios.cooja.radiomediums.AbstractRadioMedium;

import org.contikios.cooja.mspmote.MspMoteTimeEvent;
import org.contikios.cooja.mspmote.interfaces.Msp802154Radio;


/**
 * Extension of MSPSim 802.15.4 radio wrapper with bit-level errors.
 *
 * Only errors due to signal fading are supported (as opposed to errors to interference).
 * Interesting modelling effects can obtained if this is used together with a dynamic
 * channel fading model.
 *
 * @author Atis Elsts
 */
@ClassDescription("IEEE 802.15.4 Bit Error Radio")
public class Msp802154BitErrorRadio extends Msp802154Radio {
  private static Logger logger = Logger.getLogger(Msp802154Radio.class);

  private static final double NOISE_FLOOR = AbstractRadioMedium.SS_WEAK;
  private static final double GOOD_SIGNAL = NOISE_FLOOR + 15.0;

  private Random random = null;

  public Msp802154BitErrorRadio(Mote m) {
    super(m);

    random = getMote().getSimulation().getRandomGenerator();
  }

  /* The MSK-transformed symbol-to-codeword table.
   * It's used for mapping between symbols and codeword by some popular
   * 802.15.4 radios such as CC2420 and CC2520. */
  private static final int[] mskEncodeTable = {
    1618456172,
    1309113062,
    1826650030,
    1724778362,
    778887287,
    2061946375,
    2007919840,
    125494990,
    529027475,
    838370585,
    320833617,
    422705285,
    1368596360,
    85537272,
    139563807,
    2021988657
  };

  /* Calculates the Hamming distance between two words */
  private int hammingDistance(int x1, int x2) {
    return Integer.bitCount(x1 ^ x2);
  }

  /* Send a symbol over the air with a specific bit error rate */
  private int transceiveSymbolWithErrors(int txSymbol, double bitErrorRate) {
    /* First, transmit (encode and randomly corrupt) it */
    int chipSequence = mskEncodeTable[txSymbol];
    /* Note: loop until 31, not until 32 here, as the highest bit in the codeword
     * is irrelevant for MSK encoded data, and therefore should not come into
     * the Hamming distance calculations. */
    for (int i = 0; i < 31; ++i) {
      double p = random.nextDouble();
      if (p < bitErrorRate) {
        chipSequence ^= (1 << i);
      }
    }

    /* Now receive (decode) it */
    int bestRxSymbol = 0;
    int bestHammingDistance = 32;
    for (byte i = 0; i < 16; ++i) {
      /* Resolve ties in a specific order:
       *  s7, s6, ... , s0, s15, s14 , ..., s8 */
      int rxSymbol = i < 8 ? 7 - i : 15 - i + 8;

      int hd = hammingDistance(chipSequence, mskEncodeTable[rxSymbol]);
      if (hd < bestHammingDistance) {
        bestRxSymbol = rxSymbol;
        bestHammingDistance = hd;
        if (hd == 0) {
          break;
        }
      }
    }
    return bestRxSymbol;
  }

  /* This is the probability that a bit will received incorrectly, for
   * -95.0, -94.9, -94.8, ..., -80.0 dBm signal levels.
   * It is modelled as Additive White Gaussian Noise (AWGN) channel
   * with constellation size = 4, and log2(4) = 2 bits per over-the-air symbol.
   * (See "Digital Communications" by Proakis, page 311)
   *
   * The table was generated with the following Python code:
   *
   * noise_floor = -95.0
   * good_signal = noise_floor + 15.0
   *
   * # chips per second / carrier frequency
   * # In this case:
   * #  250 kbps * symbols_per_bit / 5 MHz 802.15.4 channel bandwidth
   * spectral_efficiency = 250000 * (32 / 4) / 5000000.
   *
   * def snr_from_rssi(signal):
   *     return signal - noise_floor
   *
   * def combinations(n, k):
   *     return math.factorial(n) / (math.factorial(k) * math.factorial(n - k))
   *
   * def chip_error_rate(signal):
   *    M_sk = 4 # constellation size
   *    K_b = 2  # bits per symbol
   *    snr = snr_from_rssi(signal) * spectral_efficiency  # signal-noise ratio
   *    result = 0.0
   *    for k in range(1, M_sk):
   *       result += ((-1)**(k + 1) / (k + 1.0)) * combinations(M_sk - 1, k) * math.exp(- (k / (k + 1.0)) * K_b * snr)
   *    return result * (M_sk / 2) / (M_sk - 1)
   *
   * for signal in range(int(noise_floor), int(good_signal + 1)):
   *     for decimal_part in range(10):
   *         s = signal + decimal_part / 10.0
   *         print("{}: {:.16}".format(s, chip_error_rate(s)))
   */
  private static final double[] bitErrorRateTable = {
    0.5000000000000000,
    0.4857075690874351,
    0.4717195981917559,
    0.4580362793082289,
    0.4446572346573086,
    0.4315815642818357,
    0.4188078906959320,
    0.4063344007440545,
    0.3941588848209507,
    0.3822787735959348,
    0.3706911723779558,
    0.3593928932511307,
    0.3483804851041035,
    0.3376502616704103,
    0.3271983276911817,
    0.3170206033059785,
    0.3071128467721474,
    0.2974706756080624,
    0.2880895862507204,
    0.2789649723135144,
    0.2700921415256470,
    0.2614663314303489,
    0.2530827239151328,
    0.2449364586434322,
    0.2370226454533358,
    0.2293363757856961,
    0.2218727332005190,
    0.2146268030374602,
    0.2075936812732179,
    0.2007684826257659,
    0.1941463479526962,
    0.1877224509883067,
    0.1814920044616803,
    0.1754502656356323,
    0.1695925413041991,
    0.1639141922842669,
    0.1584106374348990,
    0.1530773572360710,
    0.1479098969566969,
    0.1429038694401250,
    0.1380549575336883,
    0.1333589161873203,
    0.1288115742448300,
    0.1244088359500270,
    0.1201466821885792,
    0.1160211714852698,
    0.1120284407751141,
    0.1081647059657158,
    0.1044262623071727,
    0.1008094845848469,
    0.0973108271493890,
    0.0939268237974889,
    0.0906540875160106,
    0.0874893101013543,
    0.0844292616651377,
    0.0814707900365929,
    0.0786108200713800,
    0.0758463528759084,
    0.0731744649556428,
    0.0705923072953072,
    0.0680971043783837,
    0.0656861531527741,
    0.0633568219490458,
    0.0611065493572223,
    0.0589328430676567,
    0.0568332786811473,
    0.0548054984930601,
    0.0528472102558939,
    0.0509561859243809,
    0.0491302603869077,
    0.0473673301867607,
    0.0456653522364111,
    0.0440223425278144,
    0.0424363748414465,
    0.0409055794565784,
    0.0394281418650815,
    0.0380023014908522,
    0.0366263504167641,
    0.0352986321208818,
    0.0340175402235017,
    0.0327815172464461,
    0.0315890533858807,
    0.0304386852998058,
    0.0293289949112408,
    0.0282586082280085,
    0.0272261941799208,
    0.0262304634740650,
    0.0252701674687994,
    0.0243440970669807,
    0.0234510816288642,
    0.0225899879050480,
    0.0217597189897583,
    0.0209592132947168,
    0.0201874435437662,
    0.0194434157883799,
    0.0187261684441326,
    0.0180347713481601,
    0.0173683248375987,
    0.0167259588489542,
    0.0161068320383143,
    0.0155101309222909,
    0.0149350690395443,
    0.0143808861327198,
    0.0138468473506013,
    0.0133322424702643,
    0.0128363851389939,
    0.0123586121357148,
    0.0118982826516654,
    0.0114547775900349,
    0.0110274988842702,
    0.0106158688347500,
    0.0102193294635131,
    0.0098373418867231,
    0.0094693857045423,
    0.0091149584080853,
    0.0087735748031163,
    0.0084447664501541,
    0.0081280811206428,
    0.0078230822688482,
    0.0075293485191375,
    0.0072464731683002,
    0.0069740637025694,
    0.0067117413290037,
    0.0064591405208906,
    0.0062159085768378,
    0.0059817051932169,
    0.0057562020496316,
    0.0055390824070828,
    0.0053300407185094,
    0.0051287822513842,
    0.0049350227220528,
    0.0047484879415055,
    0.0045689134722762,
    0.0043960442961698,
    0.0042296344925230,
    0.0040694469267080,
    0.0039152529485966,
    0.0037668321007033,
    0.0036239718357372,
    0.0034864672432910,
    0.0033541207854084,
    0.0032267420407699,
    0.0031041474572487,
    0.0029861601125883,
    0.0028726094829638,
    0.0027633312191911,
    0.0026581669303560,
    0.0025569639746388,
    0.0024595752571165,
    0.0023658590343307,
    0.0022756787254125
  };

  private double getBitErrorRate(double signal) {
    if (signal <= NOISE_FLOOR) {
      return 0.5;
    } else if (signal >= GOOD_SIGNAL) {
      return 0.0;
    } else {
      long position = Math.round((signal - NOISE_FLOOR) * 10.0);
      return bitErrorRateTable[(int)position];
    }
  }

  public void receiveCustomData(Object data) {
    if (!(data instanceof Byte)) {
      logger.fatal("Bad custom data: " + data);
      return;
    }
    lastIncomingByte = (Byte) data;

    final byte inputByte;
    if (isInterfered()) {
      inputByte = (byte)0xFF;
    } else {
      double bitErrorRate = getBitErrorRate(currentSignalStrength);
      if (bitErrorRate == 0.0) {
        inputByte = lastIncomingByte;
      } else if (bitErrorRate >= 0.5) {
        inputByte = (byte) 0xFF;
      } else {
        /* convert to an unsigned int in order to prettify subsequent operations with bits */
        int incomingByteAsInt = lastIncomingByte;
        if (incomingByteAsInt < 0) incomingByteAsInt += 256;

        /* a byte consists of 2 symbols; independently transceive each of them */
        int firstSymbol = transceiveSymbolWithErrors(incomingByteAsInt >> 4, bitErrorRate);
        int secondSymbol = transceiveSymbolWithErrors(incomingByteAsInt & 0xf, bitErrorRate);

        inputByte = (byte)((firstSymbol << 4) + secondSymbol);
      }
    }

    mote.getSimulation().scheduleEvent(new MspMoteTimeEvent(mote, 0) {
      public void execute(long t) {
        super.execute(t);
        radio.receivedByte(inputByte);
        mote.requestImmediateWakeup();
      }
    }, mote.getSimulation().getSimulationTime());

  }
}

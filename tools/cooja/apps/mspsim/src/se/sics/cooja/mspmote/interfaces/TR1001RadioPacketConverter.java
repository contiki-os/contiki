/*
 * Copyright (c) 2007, Swedish Institute of Computer Science.
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

package se.sics.cooja.mspmote.interfaces;

import java.io.ByteArrayOutputStream;
import se.sics.cooja.COOJARadioPacket;
import se.sics.cooja.RadioPacket;

/**
 * Converts radio packets between TR1001/ESB and COOJA.
 * Some functionality, such as GCR coding, is ESB TR1001 driver specific.
 *
 * @author Fredrik Osterlind
 */
public class TR1001RadioPacketConverter {

  private static GCRCoder gcrCoder = new GCRCoder();

  /* TR1001 packet: PREAMBLE(20) _ SYNCH(1+4+1) _ GCR(ESBDATA(?)) _ TRAIL(4) */
  final static int TR1001_PREAMBLE_LENGTH = 20;

  final static int TR1001_NR_SYNCHBYTES = 4;

  final static byte TR1001_SYNCH1 = 0x3c;

  final static byte TR1001_SYNCH2 = 0x03;

  final static int TR1001_HEADER_LENGTH = TR1001_PREAMBLE_LENGTH + 1
      + TR1001_NR_SYNCHBYTES + 1;

  final static int TR1001_FOOTER_LENGTH = 4;

  /* "ESB" packet: LENGTH(2) _ APPDATA(?) _ CRC(2) */
  final static int ESB_HEADER_LENGTH = 2;

  final static int ESB_FOOTER_LENGTH = 2;

  private ByteArrayOutputStream originalData = new ByteArrayOutputStream(); 

  private enum AccumulatedConversionState {
    TR1001_PREAMBLE, TR1001_SYNCH, ESB_LEN1, ESB_LEN2, ESB_DATA, ESB_CRC1, ESB_CRC2, ESB_POST,
  }

  private AccumulatedConversionState accumulatedConversionState = AccumulatedConversionState.TR1001_PREAMBLE;

  private boolean accumulatedConversionOK = true;

  private int[] accumulatedConversionDataLengthArray = new int[2];

  private int[] accumulatedConversionDataArray = null;

  private int[] accumulatedConversionCRCArray = new int[2];

  private int accumulatedConversionDataCounter = 0;

  private int accumulatedConversionDataLength = 0;

  private int accumulatedConversionFooterLength = 0;

  private GCRCoder AccumulatedConversionGCRCoder = new GCRCoder();

  /**
   * Converts radio packet data from COOJA to TR1001. This
   * implementation is ESB platform and TR1001 driver specific.
   *
   * It consists of (in sequential order):
   *  - Adding 2-byte length header
   *  - Calculating and adding 2-byte CRC16 footer
   *  - GCR encoding data
   *  - Add TR1001 header: preamble and synch bytes
   *  - Add TR1001 footer: 4 trail bytes
   *
   * The returned array typically needs to be feeded to the emulated "slowly",
   * i.e. one by one and using interrupts.
   *
   * @param coojaPacket COOJA radio packet
   * @return TR1001 radio packet
   */
  public static byte[] fromCoojaToTR1001(RadioPacket coojaPacket) {

    byte[] coojaPacketData = coojaPacket.getPacketData();

    /* Create "ESB" packet: add length header and CRC16 footer */
    byte[] esbPacket = addLengthAndCRC(coojaPacketData);

    /* GCR encode radio frame */
    byte[] encodedData = gcrCoder.gcrEncode(esbPacket, esbPacket.length);

    /* Create TR1001 header (preamble and synchbytes) */
    byte[] tr1001Frame = new byte[TR1001_HEADER_LENGTH + encodedData.length
        + TR1001_FOOTER_LENGTH];
    for (int i = 0; i < TR1001_PREAMBLE_LENGTH; i++) {
      tr1001Frame[i] = (byte) 0xaa;
    }
    tr1001Frame[TR1001_PREAMBLE_LENGTH] = (byte) 0xff;
    for (int i = 0; i < TR1001_NR_SYNCHBYTES; i++) {
      tr1001Frame[TR1001_PREAMBLE_LENGTH + 1 + i] = TR1001_SYNCH1;
    }
    tr1001Frame[TR1001_PREAMBLE_LENGTH + 1 + TR1001_NR_SYNCHBYTES] = TR1001_SYNCH2;

    /* Add encoded data */
    System.arraycopy(encodedData, 0, tr1001Frame, TR1001_HEADER_LENGTH,
        encodedData.length);

    /* Add TR1001 footer (trail bytes) */
    tr1001Frame[TR1001_HEADER_LENGTH + encodedData.length] = (byte) 0x33;
    tr1001Frame[TR1001_HEADER_LENGTH + encodedData.length + 1] = (byte) 0xcc;
    tr1001Frame[TR1001_HEADER_LENGTH + encodedData.length + 2] = (byte) 0x33;
    tr1001Frame[TR1001_HEADER_LENGTH + encodedData.length + 3] = (byte) 0xcc;

    return tr1001Frame;
  }

  /**
   * Adds length header and CRC16 footer to given data.
   *
   * @param packetData
   *          Packet data
   * @return Packet data with length header and CRC16 footer
   */
  private static byte[] addLengthAndCRC(byte[] packetData) {
    short accumulatedCRC = (short) 0xffff;
    byte[] radioFrame = new byte[ESB_HEADER_LENGTH + packetData.length
        + ESB_FOOTER_LENGTH];

    /* Add length */
    radioFrame[0] = (byte) ((packetData.length >> 8) & 0xff);
    accumulatedCRC = CRCCoder.crc16Add(radioFrame[0], accumulatedCRC);
    radioFrame[1] = (byte) (packetData.length & 0xff);
    accumulatedCRC = CRCCoder.crc16Add(radioFrame[1], accumulatedCRC);

    /* Add data */
    System.arraycopy(packetData, 0, radioFrame, ESB_HEADER_LENGTH,
        packetData.length);

    /* Calculate CRC */
    for (byte element : packetData) {
      accumulatedCRC = CRCCoder.crc16Add(element, accumulatedCRC);
    }

    /* Add CRC */
    radioFrame[ESB_HEADER_LENGTH + packetData.length] = (byte) ((accumulatedCRC >> 8) & 0xff);
    radioFrame[ESB_HEADER_LENGTH + packetData.length + 1] = (byte) (accumulatedCRC & 0xff);
    return radioFrame;
  }

  /**
   * Adds another another byte to the accumulated conversion from TR1001
   * to Cooja.
   *
   * @param tr1001Byte New TR1001 packet data byte
   * @return True if conversion finished (either successful of failed)
   */
  public boolean fromTR1001ToCoojaAccumulated(byte b) {
    originalData.write(b);

    if (accumulatedConversionState == AccumulatedConversionState.TR1001_PREAMBLE) {
      if (b == (byte) 0xaa || b == (byte) 0xff) {
        return false;
      } else if (b == (byte) 0x3c) {
        accumulatedConversionState = AccumulatedConversionState.TR1001_SYNCH;
        return false;
      } else {
        accumulatedConversionOK = false;
        return true;
      }
    } else if (accumulatedConversionState == AccumulatedConversionState.TR1001_SYNCH) {
      if (b == TR1001_SYNCH1) {
        return false;
      } else if (b == TR1001_SYNCH2) {
        accumulatedConversionState = AccumulatedConversionState.ESB_LEN1;
        AccumulatedConversionGCRCoder.gcr_init();
        return false;
      } else {
        accumulatedConversionOK = false;
        return true;
      }
    } else if (accumulatedConversionState == AccumulatedConversionState.ESB_LEN1) {
      AccumulatedConversionGCRCoder.gcr_decode(0xff & b);
      if (AccumulatedConversionGCRCoder.gcr_get_decoded(accumulatedConversionDataLengthArray, 0)) {
        accumulatedConversionState = AccumulatedConversionState.ESB_LEN2;
      }
      return false;
    } else if (accumulatedConversionState == AccumulatedConversionState.ESB_LEN2) {
      AccumulatedConversionGCRCoder.gcr_decode(0xff & b);
      if (AccumulatedConversionGCRCoder.gcr_get_decoded(accumulatedConversionDataLengthArray, 1)) {
        accumulatedConversionState = AccumulatedConversionState.ESB_DATA;
        accumulatedConversionDataLength = ((accumulatedConversionDataLengthArray[0] & 0xff) << 8) +
          (accumulatedConversionDataLengthArray[1] & 0xff);
        accumulatedConversionDataArray = new int[accumulatedConversionDataLength];
      }
      return false;
    } else if (accumulatedConversionState == AccumulatedConversionState.ESB_DATA) {
      AccumulatedConversionGCRCoder.gcr_decode(0xff & b);
      if (AccumulatedConversionGCRCoder.gcr_get_decoded(accumulatedConversionDataArray, accumulatedConversionDataCounter)) {
        accumulatedConversionDataCounter++;
        if (accumulatedConversionDataCounter >= accumulatedConversionDataArray.length) {
          accumulatedConversionState = AccumulatedConversionState.ESB_CRC1;
        }
      }
      return false;
    } else if (accumulatedConversionState == AccumulatedConversionState.ESB_CRC1) {
      AccumulatedConversionGCRCoder.gcr_decode(0xff & b);
      if (AccumulatedConversionGCRCoder.gcr_get_decoded(accumulatedConversionCRCArray, 0)) {
        accumulatedConversionState = AccumulatedConversionState.ESB_CRC2;
      }
      return false;
    } else if (accumulatedConversionState == AccumulatedConversionState.ESB_CRC2) {
      AccumulatedConversionGCRCoder.gcr_decode(0xff & b);
      if (AccumulatedConversionGCRCoder.gcr_get_decoded(accumulatedConversionCRCArray, 1)) {
        accumulatedConversionState = AccumulatedConversionState.ESB_POST;
      }
      return false;
    } else if (accumulatedConversionState == AccumulatedConversionState.ESB_POST) {
      accumulatedConversionFooterLength++;
      return accumulatedConversionFooterLength >= TR1001_FOOTER_LENGTH;
    } else {
      accumulatedConversionOK = false;
      return true;
    }
  }

  /**
   * @return Converted data (application level)
   */
  public RadioPacket getAccumulatedConvertedCoojaPacket() {
    byte[] dataArrayByte = new byte[accumulatedConversionDataArray.length];
    for (int i=0; i < accumulatedConversionDataArray.length; i++) {
      dataArrayByte[i] = (byte) accumulatedConversionDataArray[i];
    }
    return new COOJARadioPacket(dataArrayByte);
  }

  /**
   * Returns status from accumulated conversion.
   *
   * @return True if ok, false if error occurred.
   */
  public boolean accumulatedConversionIsOk() {
    return accumulatedConversionOK;
  }

}

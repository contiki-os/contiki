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
 * $Id: CC2420RadioPacketConverter.java,v 1.1 2008/03/18 13:34:29 fros4943 Exp $
 */

package se.sics.cooja.mspmote.interfaces;

import org.apache.log4j.Logger;

import se.sics.cooja.COOJARadioPacket;
import se.sics.cooja.RadioPacket;

/**
 * Converts radio packets between CC24240/Sky and COOJA.
 * Handles radio driver specifics such as length header and CRC footer.
 *
 * @author Fredrik Osterlind
 */
public class CC2420RadioPacketConverter {
  private static Logger logger = Logger.getLogger(CC2420RadioPacketConverter.class);

  /* CC2420/802.15.4 packet: PREAMBLE(4) _ SYNCH(2) _ LENGTH(1) _ PAYLOAD(<27) _ CRCFOOTER(2) */

  /**
   * Converts radio packet data from COOJA to CC2420.
   *
   * @param coojaPacket COOJA packet
   * @return CC2420 packet
   */
  public static CC2420RadioPacket fromCoojaToCC24240(RadioPacket coojaPacket) {
    return new CC2420RadioPacket(coojaPacket.getPacketData());
  }

  /**
   * Converts radio packet data from CC24240 to COOJA.
   *
   * @param cc24240Data CC24240 data
   * @param cc24240DataLength CC24240 data length
   * @return
   */
  public static COOJARadioPacket fromCC2420ToCooja(CC2420RadioPacket cc2420RadioPacket) {
    return new COOJARadioPacket(cc2420RadioPacket.getPacketData());
  }

}

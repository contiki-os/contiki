/*
 * Copyright (c) 2014, TU Braunschweig
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
 */

package org.contikios.cooja.plugins.analyzers;

/**
 * Analyzes the fragmentation header if present.
 *
 * @author Enrico Jorns
 */
public class FragHeadPacketAnalyzer extends PacketAnalyzer {
  
  public final static int SICSLOWPAN_DISPATCH_FRAG1 = 0xc0; /* 1100 0xxx */

  public final static int SICSLOWPAN_DISPATCH_FRAGN = 0xe0; /* 1110 0xxx */

  @Override
  public boolean matchPacket(Packet packet) {
    return packet.level == NETWORK_LEVEL && (packet.get(0) & 0xD8) == SICSLOWPAN_DISPATCH_FRAG1;
  }

  @Override
  public int analyzePacket(Packet packet, StringBuilder brief, StringBuilder verbose) {
    int hdr_size = 0;
    
    verbose.append("<b>Frag Header</b> ");

    if ((packet.get(0) & 0xF8) == SICSLOWPAN_DISPATCH_FRAG1) {
      hdr_size = 4;
      brief.append("FRAG1");
      verbose.append("first<br/>");
    } else if ((packet.get(0) & 0xF8) == SICSLOWPAN_DISPATCH_FRAGN) {
      hdr_size = 5;
      brief.append("FRAGN");
      verbose.append("nth<br/>");
    }
    
    int datagram_size = ((packet.get(0) & 0x07) << 8) + packet.get(1);
    int datagram_tag = packet.getInt(2, 2);
    
    verbose.append("size = ").append(datagram_size)
            .append(", tag = ").append(String.format("0x%04x", datagram_tag));
    
    if (hdr_size == 5) {
      verbose.append(", offset = ").append(packet.get(4) * 8);
    }
    
    packet.pos += hdr_size;

    return ANALYSIS_OK_CONTINUE;
  }
  
}

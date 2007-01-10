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
 * $Id: PassiveMoteInterface.java,v 1.2 2007/01/10 14:57:42 fros4943 Exp $
 */

package se.sics.cooja;

/**
 * Mote interfaces are divided into active and passive interfaces.
 * 
 * A passive mote interface is treated different than an ordinary (active) mote
 * interface; Passive interfaces are allowed to act even when the mote is
 * sleeping, while active interface only acts when the mote is in active state.
 * 
 * A typical active interface is the radio interface, since radio messages only
 * can be received when the mote is active.
 * 
 * A typical passive interface is the battery interface, since a mote consumes
 * energy even though it is sleeping.
 * 
 * All passive interface should implement this interface. All interfaces not
 * implemented this interface will be handled as active interfaces.
 * 
 * Any energy required by this interface must be available after the
 * doActionsBeforeTick method.
 * 
 * @author Fredrik Osterlind
 */
public interface PassiveMoteInterface {

}

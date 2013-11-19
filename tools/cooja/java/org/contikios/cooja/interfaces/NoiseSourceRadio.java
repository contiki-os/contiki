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
 * The noise source radio is used to simulate ambient background noise or 
 * point-sources of external interference (e.g. Wifi basestations).
 * 
 * Note that interference generated from these radios are different from 
 * transmissions; they will not appear in the radio logger but may still
 * hinder or interfere with ongoing transmissions.
 * 
 * Noise source radios require significant processing resources in comparison
 * to only transmission radios.
 * 
 * COOJA's radio mediums may or may not choose to respect noise source radios.
 * 
 * @see MRM
 * @author Fredrik Osterlind
 */
public interface NoiseSourceRadio {
	public int getNoiseLevel();
	
	public void addNoiseLevelListener(NoiseLevelListener l);
	public void removeNoiseLevelListener(NoiseLevelListener l);

	public interface NoiseLevelListener {
		public void noiseLevelChanged(NoiseSourceRadio radio, int signal);
	}
}

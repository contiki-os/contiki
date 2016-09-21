/*
 * Copyright (c) 2012, Thingsquare, http://www.thingsquare.com/.
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
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

package se.sics.mspsim.core;

public class StatusRegister {
	static final int NEGATIVE = MSP430Constants.NEGATIVE;
	static final int ZERO = MSP430Constants.ZERO;
	static final int CARRY = MSP430Constants.CARRY;
	static final int OVERFLOW = MSP430Constants.OVERFLOW;

	static final boolean DEBUG = false;

	/* Tested with ADDA_IMM, ADDA_REG */
	public static int updateSR(int sr, int src, int dstArg, int dstResult) {
		sr &= ~(NEGATIVE | ZERO | CARRY | OVERFLOW);

		int dst20 = dstResult & 0xfffff;
		boolean srcNegative = (dstArg & (1 << 19)) != 0;
		boolean oldDstNegative = (dstArg & (1 << 19)) != 0;
		boolean dstNegative = (dst20 & (1 << 19)) != 0;

		if (dstNegative) {
			/* negative */
			sr |= NEGATIVE;
			if (DEBUG) {
				System.out.println("sr: negative");
			}
		}
		if (dst20 == 0) {
			/* zero */
			sr |= ZERO;
			if (DEBUG) {
				System.out.println("sr: zero");
			}
		}
		if (dstResult > dst20) {
			/* carry */
			sr |= CARRY;
			if (DEBUG) {
				System.out.println("sr: carry");
			}
		}
		if (srcNegative && oldDstNegative && !dstNegative) {
			/* overflow */
			sr |= OVERFLOW;
			if (DEBUG) {
				System.out.println("sr: overflow (--+)");
			}
		}
		if (!srcNegative && !oldDstNegative && dstNegative) {
			sr |= OVERFLOW;
			if (DEBUG) {
				System.out.println("sr: overflow (++-)");
			}
		}

		if (DEBUG) {
			System.out.printf("sr: updated sr 0x%02x\n", sr);
		}
		return sr;
	}
}

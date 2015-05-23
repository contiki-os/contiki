/*
 * Copyright (c) 2009, Swedish Institute of Computer Science
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
 * @author Nicolas Tsiftes
 *
 */

package org.contikios.coffee;

import java.io.IOException;

public class CoffeeMicroLog extends CoffeeFile {
	private int logRecords;
	private int logRecordSize;
	private int indexStart;
	private int indexSize;
	private int recordStart;
	private int[] index;

	public CoffeeMicroLog(CoffeeFS fs, CoffeeHeader header)
			throws IOException {
		super(fs, header);

		CoffeeConfiguration conf = fs.getConfiguration();
		if (header.logRecordSize == 0) {
			logRecordSize = conf.pageSize;
		}
		if (header.logRecords == 0) {
			logRecords = conf.defaultLogSize / logRecordSize;
		} else {
			logRecords = header.logRecords;
		}

		indexStart = header.getPage() * conf.pageSize +
			     header.rawLength();
		/* An index entry uses two bytes. */
		indexSize = logRecords * 2;
		recordStart = indexStart + indexSize;

		index = new int[logRecords];
		byte[] bytes = new byte[2];

		for (int i = 0; i < logRecords; i++) {
			coffeeFS.getImage().read(bytes, bytes.length,
				indexStart + i * 2);
			index[i] = bytes[1] << 8 | bytes[0];
		}
	}

	public byte[] getRegion(int region) throws IOException {
		for (int i = logRecords - 1; i >= 0; i--) {
			if (index[i] - 1 == region) {
				byte[] bytes = new byte[logRecordSize];
				coffeeFS.getImage().read(bytes, bytes.length, 
					recordStart + i * logRecordSize);
				return bytes;
			}
		}

		return null;
	}

	public int getLogRecords() {
		return logRecords;
	}

	public int getLogRecordSize() {
		return logRecordSize;
	}
}

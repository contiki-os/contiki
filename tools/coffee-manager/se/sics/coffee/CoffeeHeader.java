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
 * This file is part of the Contiki operating system.
 *
 * $Id: CoffeeHeader.java,v 1.2 2009/08/10 12:51:52 nvt-se Exp $
 *
 * @author Nicolas Tsiftes
 *
 */

package se.sics.coffee;

class CoffeeHeader {
	private CoffeeFS coffeeFS;
	private CoffeeConfiguration conf;
	private int page;

	private static final int HDR_FLAG_VALID		= 0x1;
	private static final int HDR_FLAG_ALLOCATED	= 0x2;
	private static final int HDR_FLAG_OBSOLETE	= 0x4;
	private static final int HDR_FLAG_MODIFIED	= 0x8;
	private static final int HDR_FLAG_LOG		= 0x10;
	private static final int HDR_FLAG_ISOLATED	= 0x20;

	int logPage;
	int logRecords;
	int logRecordSize;
	int maxPages;
	String name;

	private boolean valid;
	private boolean allocated;
	private boolean obsolete;
	private boolean modified;
	private boolean log;
	private boolean isolated;

	public CoffeeHeader(CoffeeFS coffeeFS, int page) {
		this.coffeeFS = coffeeFS;
		this.page = page;
		conf = coffeeFS.getConfiguration();
	}

	public CoffeeHeader(CoffeeFS coffeeFS, int page, byte[] bytes) {
		this.coffeeFS = coffeeFS;
		this.page = page;

		processRawHeader(bytes);
	}

	private void processRawHeader(byte[] bytes) {
		int index = 0;

		logPage = getPageValue(bytes, 0);
		index += conf.pageTypeSize;

		logRecords = bytes[index] + (bytes[index + 1] << 8);
		index += 2;

		logRecordSize = bytes[index] + (bytes[index + 1] << 8);
		index += 2;

		maxPages = getPageValue(bytes, index);
		index += conf.pageTypeSize;

		index++; // Skip deprecated EOF hint field.

		processFlags((int)bytes[index]);
		index++;

		name = new String(bytes).substring(index,
				index + conf.NAME_LENGTH);
		int nullCharOffset = name.indexOf(0);
		if (nullCharOffset >= 0) {
			name = name.substring(0, nullCharOffset);
		}
	}

	private byte composeFlags() {
		byte flags = 0;

		if (valid) {
			flags |= HDR_FLAG_VALID;
		}
		if (allocated) {
			flags |= HDR_FLAG_ALLOCATED;
		}
		if (obsolete) {
			flags |= HDR_FLAG_OBSOLETE;
		}
		if (modified) {
			flags |= HDR_FLAG_MODIFIED;
		}
		if (log) {
			flags |= HDR_FLAG_LOG;
		}

		if (isolated) {
			flags |= HDR_FLAG_ISOLATED;
		}

		return flags;
	}

	private void processFlags(int flags) {
		if ((flags & HDR_FLAG_VALID) != 0) {
			valid = true;
		}
		if ((flags & HDR_FLAG_ALLOCATED) != 0) {
			allocated = true;
		}
		if ((flags & HDR_FLAG_OBSOLETE) != 0) {
			obsolete = true;
		}
		if ((flags & HDR_FLAG_MODIFIED) != 0) {
			modified = true;
		}
		if ((flags & HDR_FLAG_LOG) != 0) {
			log = true;
		}
		if ((flags & HDR_FLAG_ISOLATED) != 0) {
			isolated = true;
		}
	}

	private byte[] setPageValue(int page) {
		byte[] bytes = new byte[conf.pageTypeSize];

		for (int i = conf.pageTypeSize - 1; i >= 0; i--) {
			bytes[i] = (byte) (page >> (8 * i));
		}
		return bytes;
	}

	private int getPageValue(byte[] bytes, int offset) {
		int page = 0;

		for (int i = 0; i < conf.pageTypeSize; i++) {
			page |= bytes[offset + i] << (8 * i);
		}
		return page;
	}

	public byte[] toRawHeader() {
		byte[] bytes = new byte[2 * conf.pageTypeSize + 
					conf.NAME_LENGTH + 6];
		int index = 0;

		System.arraycopy(setPageValue(logPage), 0, bytes, 0, 
			conf.pageTypeSize);
		index += conf.pageTypeSize;

		bytes[index++] = (byte) (logRecords >> 8);
		bytes[index++] = (byte) logRecords;

		bytes[index++] = (byte) (logRecordSize >> 8);
		bytes[index++] = (byte) logRecordSize;

		System.arraycopy(setPageValue(maxPages), 0, bytes, index,
			conf.pageTypeSize);
		index += conf.pageTypeSize;

		bytes[index++] = 0; // Deprecated EOF hint field.
		bytes[index++] = composeFlags();

		byte[] nameBytes = name.getBytes();
		int copyLength = nameBytes.length > conf.NAME_LENGTH ?
					conf.NAME_LENGTH : nameBytes.length;
		System.arraycopy(nameBytes, 0, bytes, index, copyLength);

		return bytes;
	}

	public int rawLength() {
		return 2 * conf.pageTypeSize + conf.NAME_LENGTH + 6;
	}

	public int getPage() {
		return page;
	}

	public boolean isValid() {
		return valid;
	}

	public boolean isAllocated() {
		return allocated;
	}

	public boolean isObsolete() {
		return obsolete;
	}

	public boolean isModified() {
		return modified;
	}

	public boolean isIsolated() {
		return isolated;
	}

	public boolean isLog() {
		return log;
	}

	public boolean isFree() {
		return !isAllocated();
	}

	public boolean isActive() {
		return isAllocated() && !isObsolete() && !isIsolated();
	}

	public void allocate() {
		allocated = true;
	}

	public void makeObsolete() {
		obsolete = true;
	}

	public void setName(String name) {
		this.name = name;
	}

	public void setReservedSize(int pages) {
		maxPages = pages;
	}

	public int getReservedSize() {
		return maxPages;
	}
}

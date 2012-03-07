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

package se.sics.coffee;

import java.io.*;
import java.util.Properties;

import se.sics.coffee.CoffeeFS.CoffeeException;

public class CoffeeConfiguration {
	public static final int FD_SET_SIZE = 256;
	public static final int MAX_OPEN_FILES = 256;
	public static final int LOG_TABLE_LIMIT = 256;
	public final int nameLength;
	public final int fsSize, sectorSize, pageSize;
	public final int startOffset, pageTypeSize;
	public final int defaultFileSize, defaultLogSize;
	public final int pagesPerSector;
	public final boolean useMicroLogs;

	public CoffeeConfiguration(String filename)
			throws CoffeeException, IOException {
		String[] requiredParameters = {"name_length", "use_micro_logs", "fs_size",
					    "page_size", "sector_size",
					    "start_offset", "default_file_size",
					    "default_log_size", "page_type_size"};
		Properties prop = new Properties();
		InputStream stream = CoffeeConfiguration.class.getResourceAsStream("/" + filename);
		if (stream == null) {
			throw new CoffeeException("failed to load the configuration file " + filename);
		}

		prop.load(stream);
		for (int i = 0; i < requiredParameters.length; i++) {
			if (prop.getProperty(requiredParameters[i]) == null) {
				throw new CoffeeException("missing the parameter \"" + requiredParameters[i] + "\" in the configuration file " + filename);
			}
		}

		nameLength = Integer.parseInt(prop.getProperty("name_length"));
		useMicroLogs = new Boolean(prop.getProperty("use_micro_logs")).booleanValue();
		fsSize = Integer.parseInt(prop.getProperty("fs_size"));
		sectorSize = Integer.parseInt(prop.getProperty("sector_size"));
		pageSize = Integer.parseInt(prop.getProperty("page_size"));
		defaultFileSize = Integer.parseInt(prop.getProperty("default_file_size"));
		defaultLogSize = Integer.parseInt(prop.getProperty("default_log_size"));
		startOffset = Integer.parseInt(prop.getProperty("start_offset"));
		pageTypeSize = Integer.parseInt(prop.getProperty("page_type_size"));

		pagesPerSector = sectorSize / pageSize;
	}
}

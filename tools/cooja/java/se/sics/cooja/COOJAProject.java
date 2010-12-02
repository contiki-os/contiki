/*
 * Copyright (c) 2010, Swedish Institute of Computer Science. All rights
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
 * $Id: COOJAProject.java,v 1.1 2010/12/02 15:27:08 fros4943 Exp $
 */

package se.sics.cooja;

import java.io.File;
import java.util.Arrays;

import org.apache.log4j.Logger;

/**
 * COOJA Project.
 *
 * @author Fredrik Osterlind
 */
public class COOJAProject {
	private static Logger logger = Logger.getLogger(COOJAProject.class);

	public File dir = null;
	public File configFile = null;
	public ProjectConfig config = null;

	public COOJAProject(File dir) {
		try {
			this.dir = dir;
			configFile = new File(dir.getPath(), GUI.PROJECT_CONFIG_FILENAME);
			config = new ProjectConfig(false);
			config.appendConfigFile(configFile);
		} catch (Exception e) {
			logger.fatal("Error when loading COOJA project: " + e.getMessage());
		}
	}

	public boolean directoryExists() {
		return dir.exists();
	}
	public boolean configExists() {
		return configFile.exists();
	}
	public boolean configRead() {
		return config != null;
	}
	public boolean hasError() {
		if (!directoryExists() || !configExists() || !configRead()) {
			return true;
		}
		if (getConfigJARs() != null) {
			String[] jars = getConfigJARs();
			for (String jar: jars) {
				File jarFile = GUI.findJarFile(dir, jar);
				if (jarFile == null || !jarFile.exists()) {
					return true;
				}
			}
		}
		return false;
	}

	/**
	 * @return Description or null
	 */
	public String getDescription() {
		return config.getStringValue("DESCRIPTION");
	}

	private String[] getStringArray(String key) {
		String[] arr = config.getStringArrayValue(key);
		if (arr == null || arr.length == 0) {
			return null;
		}
		if (arr[0].equals("+")) {
			/* strip + */
			return Arrays.copyOfRange(arr, 1, arr.length);
		}
		return arr;
	}
	public String[] getConfigPlugins() {
		return getStringArray("se.sics.cooja.GUI.PLUGINS");
	}
	public String[] getConfigJARs() {
		return getStringArray("se.sics.cooja.GUI.JARFILES");
	}
	public String[] getConfigMoteTypes() {
		return getStringArray("se.sics.cooja.GUI.MOTETYPES");
	}
	public String[] getConfigRadioMediums() {
		return getStringArray("se.sics.cooja.GUI.RADIOMEDIUMS");
	}
	public String[] getConfigMoteInterfaces() {
		return getStringArray("se.sics.cooja.contikimote.ContikiMoteType.MOTE_INTERFACES");
	}
	public String[] getConfigCSources() {
		return getStringArray("se.sics.cooja.contikimote.ContikiMoteType.C_SOURCES");
	}

	public String toString() {
		if (getDescription() != null) {
			return getDescription();
		}
		return dir.toString();
	}
}

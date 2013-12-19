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
 */

package org.contikios.cooja;

import java.io.File;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;
import org.apache.log4j.Logger;

/**
 * COOJA Project.
 *
 * @author Fredrik Osterlind
 * @author Moritz Strübe
 */
public class COOJAProject {
	private static Logger logger = Logger.getLogger(COOJAProject.class);
	
	
	public static File[] sarchProjects(File folder){
		return sarchProjects(folder, 3);
	}
	
	public static File[] sarchProjects(File folder, int depth){
		if(depth == 0){
			return null;
		}
		depth--;
		ArrayList<File> dirs = new ArrayList<File>();
		
		if(!folder.isDirectory()){
			logger.warn("Project directorys: " + folder.getPath() + "is not a Folder" );
			return null;
		}
		File[] files = folder.listFiles();
		for(File subf : files){
			if(subf.getName().charAt(0) == '.') continue;
			if(subf.isDirectory()){
				File[] newf = sarchProjects(subf, depth);
				if(newf != null){
					Collections.addAll(dirs, newf);
				}
			}
			if(subf.getName().equals(Cooja.PROJECT_CONFIG_FILENAME)){
				try{
					dirs.add(folder);
				} catch(Exception e){
					logger.error("Somthing odd happend", e);
				}
			}
		}
		return dirs.toArray(new File[0]);
		
	}

	
	public File dir = null;
	public File configFile = null;
	public ProjectConfig config = null;
	

	public COOJAProject(File dir) {
		try {
			this.dir = dir;
			configFile = new File(dir.getPath(), Cooja.PROJECT_CONFIG_FILENAME);
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
				File jarFile = Cooja.findJarFile(dir, jar);
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
		return getStringArray("org.contikios.cooja.Cooja.PLUGINS");
	}
	public String[] getConfigJARs() {
		return getStringArray("org.contikios.cooja.Cooja.JARFILES");
	}
	public String[] getConfigMoteTypes() {
		return getStringArray("org.contikios.cooja.Cooja.MOTETYPES");
	}
	public String[] getConfigRadioMediums() {
		return getStringArray("org.contikios.cooja.Cooja.RADIOMEDIUMS");
	}
	public String[] getConfigMoteInterfaces() {
		return getStringArray("org.contikios.cooja.contikimote.ContikiMoteType.MOTE_INTERFACES");
	}
	public String[] getConfigCSources() {
		return getStringArray("org.contikios.cooja.contikimote.ContikiMoteType.C_SOURCES");
	}

	public String toString() {
		if (getDescription() != null) {
			return getDescription();
		}
		return dir.toString();
	}
}

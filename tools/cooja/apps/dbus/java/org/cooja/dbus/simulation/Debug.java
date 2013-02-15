package org.cooja.dbus.simulation;

public class Debug {

	public static void debug(String s, boolean error) {
		String message= "DBus " + s;
		if (error) {
			System.err.println(message);
		} else {
			System.out.println(message);
		}
	}

	public static void debug(String s){
		Debug.debug(s, false);
	}

}

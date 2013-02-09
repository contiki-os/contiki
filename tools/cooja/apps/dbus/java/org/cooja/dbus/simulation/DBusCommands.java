package org.cooja.dbus.simulation;
import org.freedesktop.dbus.DBusInterface;
import org.freedesktop.dbus.DBusSignal;
import org.freedesktop.dbus.exceptions.DBusException;

public interface DBusCommands extends DBusInterface
{
   public static class Start extends DBusSignal
   {
      public final String address;
      public Start(String path, String address)
                                 throws DBusException
      {
         super(path, address);
         this.address = address;
      }
   }
}
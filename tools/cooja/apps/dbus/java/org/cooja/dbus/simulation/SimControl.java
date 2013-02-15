package org.cooja.dbus.simulation;
import org.freedesktop.dbus.DBusInterface;
import org.freedesktop.dbus.DBusInterfaceName;

/**
 * Interface to be exported over dbus
 *
 * @author Salvo 'LtWorf' Tomaselli
 *
 */
@DBusInterfaceName("org.cooja.simulation.SimControl")
public interface SimControl extends DBusInterface
{
    public void Start();
    public void Stop();
    public void setSimulationTime(long time);
    public long getSimulationTime();
    public void setSpeedLimit(double limit);
    public double getSpeedLimit();
    public void setTitle(String title);
    public String getTitle();
    public void Reload();
    public void Step();
}
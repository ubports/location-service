# Tips'n'Tricks

## Mark HERE license as accepted from cmdline

    sudo LC_ALL=C gdbus call --system --dest org.freedesktop.Accounts --object-path /org/freedesktop/Accounts/User32011 --method org.freedesktop.DBus.Properties.Set com.ubuntu.location.providers.here.AccountsService LicenseAccepted '<true>'

## Force startup after ofono and NM are started

This is a *workaround* to get connectivity API to collect; mount your
system read-write and edit
/etc/init/ubuntu-location-provider-here-slpgwd.conf:

    sudo mount -o remount,rw /
    sudo vi /etc/init/ubuntu-location-provider-here-slpgwd.conf

change: `start on started dbus and (started ofono or started network-manager)`
to: `start on started dbus and started ofono and started network-manager`

    sudo mount -o remount,ro /
    sync
    sudo reboot

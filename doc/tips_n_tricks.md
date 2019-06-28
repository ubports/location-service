# Tips'n'Tricks

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

# Copyright © 2013 Canonical Ltd.
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License version 3 as
# published by the Free Software Foundation.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
#
# Authored by: Thomas Voss <thomas.voss@canonical.com>

configure_file(
  ubuntu-location-service.pc.in ubuntu-location-service.pc @ONLY
)

configure_file(
  ubuntu-location-service-connectivity.pc.in ubuntu-location-service-connectivity.pc @ONLY
)

configure_file(
  ubuntu-location-service.conf.in ubuntu-location-service.conf @ONLY
)

configure_file(
  ubuntu-location-service-trust-stored.conf.in ubuntu-location-service-trust-stored.conf @ONLY
)

install(
  FILES ${CMAKE_CURRENT_BINARY_DIR}/ubuntu-location-service.pc
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig
)

install(
  FILES ${CMAKE_CURRENT_BINARY_DIR}/ubuntu-location-service-connectivity.pc
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig
)

install(
  FILES ${CMAKE_CURRENT_SOURCE_DIR}/com.ubuntu.location.Service.conf
  DESTINATION /etc/dbus-1/system.d/
)

install(
  FILES ${CMAKE_CURRENT_BINARY_DIR}/ubuntu-location-service.conf
  DESTINATION /etc/init/
)

install(
  FILES ${CMAKE_CURRENT_BINARY_DIR}/ubuntu-location-service-trust-stored.conf
  DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/upstart/sessions/
)

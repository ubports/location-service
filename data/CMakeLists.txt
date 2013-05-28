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

install(
  FILES ${CMAKE_CURRENT_BINARY_DIR}/ubuntu-location-service.pc
  DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/pkgconfig
)

install(
  FILES ${CMAKE_CURRENT_SOURCE_DIR}/com.ubuntu.location.Service.conf
  DESTINATION /etc/dbus-1/system.d/
)

// Copyright (C) 2016 Canonical Ltd.
// 
// This library is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published
// by the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
#ifndef ICHNAEA_BLUETOOTH_BEACON_H_
#define ICHNAEA_BLUETOOTH_BEACON_H_

#include <chrono>
#include <string>

namespace ichnaea
{
/// @brief BluetoothBeacon models a visible bluetooth le device.
struct BluetoothBeacon
{
    std::string mac_address;        ///< The address of the Bluetooth Low Energy (BLE) beacon.
    std::string name;               ///< The name of the BLE beacon.
    std::chrono::milliseconds age;  ///< The number of milliseconds since this BLE beacon was last seen.
    double signal_strength;         ///< The measured signal strength of the BLE beacon in dBm.
};
}

#endif // ICHNAEA_BLUETOOTH_BEACON_H_

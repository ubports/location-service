// Copyright (C) 2016 Thomas Voss <thomas.voss.bochum@gmail.com>
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
#ifndef UBX_8_SERIAL_PORT_RECEIVER_H_
#define UBX_8_SERIAL_PORT_RECEIVER_H_

#include <location/providers/ubx/_8/receiver.h>

#include <boost/asio.hpp>
#include <boost/filesystem.hpp>

#include <array>

namespace location
{
namespace providers
{
namespace ubx
{
namespace _8
{

/// @brief SerialPortReceiver connects to a ublox 8 GNSS receiver over a serial
/// port.
class SerialPortReceiver : public Receiver, public std::enable_shared_from_this<SerialPortReceiver>
{
public:
    /// @brief create returns a new Receiver instance connected to the
    /// serial port reachable under dev.
    static std::shared_ptr<SerialPortReceiver> create(boost::asio::io_service& ios, const boost::filesystem::path& dev,
                                                      const std::shared_ptr<Receiver::Monitor>& monitor);

    void start();
    void stop();

private:
    /// @brief Receiver initializes a new instance opening the serial port
    /// located at path.
    ///
    /// Throws in case of issues.
    SerialPortReceiver(boost::asio::io_service& ios, const boost::filesystem::path& dev,
                       const std::shared_ptr<Receiver::Monitor>& monitor);

    /// @brief finalize returns a finalized reader instance reading from
    /// the serial port.
    std::shared_ptr<SerialPortReceiver> finalize();

    /// @brief start_read starts an async read operation from the configured
    /// serial port.
    void start_read();

    Receiver::Buffer buffer;
    boost::asio::io_service& ios;
    boost::asio::serial_port sp;
};
}
}
}
}

#endif // UBX_8_SERIAL_PORT_RECEIVER_H_

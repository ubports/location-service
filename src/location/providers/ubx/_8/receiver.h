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
#ifndef UBX_8_RECEIVER_H_
#define UBX_8_RECEIVER_H_

#include <location/providers/ubx/_8/nmea/scanner.h>
#include <location/providers/ubx/_8/nmea/sentence.h>

#include <memory>

namespace location
{
namespace providers
{
namespace ubx
{
namespace _8
{

/// @brief Receiver connects to a ublox 8 GNSS receiver.
class Receiver
{
public:
    using Buffer = std::array<char, 1024>;

    /// @brief Monitor provides calling code with means for monitoring
    /// receiver operation.
    class Monitor
    {
    public:
        /// @cond
        Monitor() = default;
        Monitor(const Monitor&) = delete;
        Monitor(Monitor&&) = delete;
        virtual ~Monitor() = default;
        Monitor& operator=(const Monitor&) = delete;
        Monitor& operator=(Monitor&&) = delete;
        /// @endcond

        /// @brief on_new_chunk is invoked for every incoming chunk of raw data.
        virtual void on_new_chunk(Buffer::iterator it, Buffer::iterator itE) = 0;

        /// @brief on_new_nmea_sentence is invoked for every complete and parsed
        /// nmea sentence.
        virtual void on_new_nmea_sentence(const nmea::Sentence& sentence) = 0;
    };

protected:
    /// @brief Receiver initializes a new instance with monitor
    ///
    /// Throws in case of issues.
    Receiver(const std::shared_ptr<Monitor>& monitor);

    /// @brief process_chunk iterates over the given range, updating scanners and
    /// parsers.
    ///
    /// Calls out to a configured monitor instance for announcing results.
    void process_chunk(Buffer::iterator it, Buffer::iterator itE);

private:
    std::shared_ptr<Monitor> monitor;
    nmea::Scanner nmea_scanner;
};
}
}
}
}

#endif // UBX_8_RECEIVER_H_

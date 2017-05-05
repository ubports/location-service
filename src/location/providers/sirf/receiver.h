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

#ifndef LOCATION_PROVIDERS_SIRF_RECEIVER_H_
#define LOCATION_PROVIDERS_SIRF_RECEIVER_H_

#include <location/providers/sirf/codec.h>
#include <location/providers/sirf/message.h>
#include <location/providers/sirf/scanner.h>

#include <location/nmea/scanner.h>
#include <location/nmea/sentence.h>

#include <memory>

namespace location
{
namespace providers
{
namespace sirf
{

/// @brief Receiver connects to a ublox 8 GNSS receiver.
class Receiver
{
public:
    using Buffer = std::array<std::uint8_t, 4096>;

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

        /// @brief on_new_ubx_message is invoked for every complete and parsed
        /// ubx message.
        virtual void on_new_sirf_message(const sirf::Message& message) = 0;

        /// @brief on_new_nmea_sentence is invoked for every complete and parsed
        /// nmea sentence.
        virtual void on_new_nmea_sentence(const nmea::Sentence& sentence) = 0;
    };

    /// @brief send_message encodes and sends 'message' to the receiver.
    void send_message(const Message& message);

    /// @brief send_encoded_message sends out data to the receiver.
    virtual void send_encoded_message(const std::vector<std::uint8_t> &data) = 0;

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
    sirf::Scanner sirf_scanner;
};

}  // namespace sirf
}  // namespace providers
}  // namespace location

#endif // LOCATION_PROVIDERS_SIRF_RECEIVER_H_

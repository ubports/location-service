#include <location/providers/ubx/_8/receiver.h>

#include <location/providers/ubx/_8/nmea/sentence.h>

#include <iostream>

namespace ubx = location::providers::ubx;

ubx::_8::Receiver::Receiver(const std::shared_ptr<Monitor>& monitor) : monitor{monitor} {}

void ubx::_8::Receiver::process_chunk(Buffer::iterator it, Buffer::iterator itE)
{
    monitor->on_new_chunk(it, itE);

    while (it != itE)
    {
        if (nmea::Scanner::Expect::nothing_more == nmea_scanner.update(*it))
        {
            monitor->on_new_nmea_sentence(nmea::parse_sentence(nmea_scanner.finalize()));
        }
        ++it;
    }
}

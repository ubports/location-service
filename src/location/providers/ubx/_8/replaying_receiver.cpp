#include <location/providers/ubx/_8/replaying_receiver.h>

#include <iostream>
#include <memory>

namespace ubx = location::providers::ubx;

std::shared_ptr<ubx::_8::ReplayingReceiver> ubx::_8::ReplayingReceiver::create(
    const boost::filesystem::path& trace, const std::shared_ptr<Monitor>& monitor)
{
    return std::shared_ptr<ReplayingReceiver>{new ReplayingReceiver{trace, monitor}};
}

void ubx::_8::ReplayingReceiver::run()
{
    while (in)
    {
        auto size = in.readsome(&buffer.front(), buffer.size());
        if (size > 0)
            process_chunk(buffer.begin(), buffer.begin() + size);
        else
            break;
    }
}

ubx::_8::ReplayingReceiver::ReplayingReceiver(const boost::filesystem::path& trace,
                                              const std::shared_ptr<Monitor>& monitor)
    : Receiver{monitor}, in{trace.string().c_str()}
{
    if (not in)
        throw std::runtime_error{"Failed to open " + trace.string() + " for reading."};
}

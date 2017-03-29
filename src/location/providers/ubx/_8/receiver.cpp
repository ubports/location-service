#include <location/providers/ubx/_8/receiver.h>

#include <location/providers/ubx/_8/nmea/sentence.h>

#include <iostream>

namespace ubx = location::providers::ubx;

namespace
{

struct EncodingVisitor : public boost::static_visitor<std::vector<std::uint8_t>>
{
    template<typename T>
    std::vector<std::uint8_t> operator()(const T&) const
    {
        throw std::logic_error{"Encoding not supported"};
    }

    std::vector<std::uint8_t> operator()(const ubx::_8::cfg::Gnss& gnss) const
    {
        return ubx::_8::encode_message(gnss);
    }

    std::vector<std::uint8_t> operator()(const ubx::_8::cfg::Msg& msg) const
    {
        return ubx::_8::encode_message(msg);
    }
};

}  // namespace

ubx::_8::Receiver::Receiver(const std::shared_ptr<Monitor>& monitor) : monitor{monitor} {}

void ubx::_8::Receiver::send_message(const Message& message)
{
    send_encoded_message(boost::apply_visitor(EncodingVisitor{}, message));
}

void ubx::_8::Receiver::process_chunk(Buffer::iterator it, Buffer::iterator itE)
{
    while (it != itE)
    {
        auto result = ubx_scanner.update(*it);

        if (std::get<0>(result) == Scanner::Expect::nothing_more)
            monitor->on_new_ubx_message(ubx_scanner.finalize());
        if (!std::get<1>(result))
        {
            if (nmea::Scanner::Expect::nothing_more == nmea_scanner.update(*it))
                monitor->on_new_nmea_sentence(nmea::parse_sentence(nmea_scanner.finalize()));
        }
        ++it;
    }
}

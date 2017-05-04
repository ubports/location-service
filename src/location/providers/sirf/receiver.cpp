#include <location/providers/sirf/receiver.h>

#include <location/nmea/sentence.h>
#include <location/providers/sirf/message.h>

#include <iostream>

namespace sirf = location::providers::sirf;

namespace
{

struct EncodingVisitor : public boost::static_visitor<std::vector<std::uint8_t>>
{
    template<typename T>
    std::vector<std::uint8_t> operator()(const T&) const
    {
        throw std::logic_error{"Encoding not supported"};
    }

    std::vector<std::uint8_t> operator()(const sirf::InitializeDataSource& ids) const
    {
        return sirf::encode_message(ids);
    }

    std::vector<std::uint8_t> operator()(const sirf::SetProtocol& sp) const
    {
        return sirf::encode_message(sp);
    }
};

}  // namespace

sirf::Receiver::Receiver(const std::shared_ptr<Monitor>& monitor) : monitor{monitor} {}

void sirf::Receiver::send_message(const Message& message)
{
    send_encoded_message(boost::apply_visitor(EncodingVisitor{}, message));
}

void sirf::Receiver::process_chunk(Buffer::iterator it, Buffer::iterator itE)
{
    while (it != itE)
    {
        auto result = sirf_scanner.update(*it);

        if (std::get<0>(result) == Scanner::Expect::nothing_more)
        {
            try
            {
                monitor->on_new_sirf_message(sirf_scanner.finalize());
            }
            catch (...)
            {
                // Dropping the exception as there is hardly any reasonable measure
                // we can take. Both scanners are designed to recover from issues, and we
                // we just trap the exception here to guarantee that we keep on consuming the
                // entire buffer.
            }
        }
        else if (!std::get<1>(result))
        {
            if (nmea::Scanner::Expect::nothing_more == nmea_scanner.update(*it))
            {
                try
                {
                    monitor->on_new_nmea_sentence(nmea::parse_sentence(nmea_scanner.finalize()));
                }
                catch (...)
                {
                    // Dropping the exception as there is hardly any reasonable measure
                    // we can take. Both scanners are designed to recover from issues, and we
                    // we just trap the exception here to guarantee that we keep on consuming the
                    // entire buffer.
                }
            }
        }
        ++it;
    }
}

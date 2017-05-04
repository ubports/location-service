#include <location/providers/sirf/serial_port_receiver.h>

#include <location/logging.h>

#include <iostream>
#include <system_error>

namespace sirf = location::providers::sirf;

std::shared_ptr<sirf::SerialPortReceiver> sirf::SerialPortReceiver::create(
    boost::asio::io_service& ios, const boost::filesystem::path& dev, const std::shared_ptr<Monitor>& monitor)
{
    return std::shared_ptr<SerialPortReceiver>{new SerialPortReceiver{ios, dev, monitor}};
}

sirf::SerialPortReceiver::SerialPortReceiver(boost::asio::io_service& ios, const boost::filesystem::path& dev,
                                                const std::shared_ptr<Monitor>& monitor)
    : Receiver{monitor}, ios{ios}, serial_port{ios, dev.string().c_str()}
{
    serial_port.set_option(boost::asio::serial_port::baud_rate(4800));
    serial_port.set_option(boost::asio::serial_port::stop_bits(boost::asio::serial_port::stop_bits::one));
    serial_port.set_option(boost::asio::serial_port::parity(boost::asio::serial_port::parity::none));
    serial_port.set_option(boost::asio::serial_port::flow_control(boost::asio::serial_port::flow_control::none));
}

void sirf::SerialPortReceiver::send_encoded_message(const std::vector<std::uint8_t>& data)
{
    auto thiz = shared_from_this();
    std::weak_ptr<SerialPortReceiver> wp{thiz};

    ios.dispatch([this, wp, data]()
    {
        if (auto sp = wp.lock())
            boost::asio::write(serial_port, boost::asio::buffer(data), boost::asio::transfer_all());
    });
}

void sirf::SerialPortReceiver::start()
{
    ::tcflush(serial_port.lowest_layer().native_handle(), TCIFLUSH);
    start_read();
}

void sirf::SerialPortReceiver::stop()
{
    serial_port.cancel();
}

void sirf::SerialPortReceiver::start_read()
{
    auto thiz = shared_from_this();
    std::weak_ptr<SerialPortReceiver> wp{thiz};

    boost::asio::async_read(serial_port, boost::asio::buffer(&buffer.front(), buffer.size()),
                            [this, wp](const boost::system::error_code& ec, std::size_t transferred) {
                                if (ec == boost::asio::error::operation_aborted)
                                    return;

                                if (auto sp = wp.lock())
                                {
                                    if (not ec)
                                    {
                                        try
                                        {
                                            process_chunk(buffer.begin(), buffer.begin() + transferred);
                                        }
                                        catch(const std::exception& e)
                                        {
                                            LOG(WARNING) << "Error processing data chunk: " << e.what();
                                        }
                                        catch(...)
                                        {
                                            LOG(WARNING) << "Error processing data chunk.";
                                        }

                                        start_read();
                                    }
                                }
                            });
}

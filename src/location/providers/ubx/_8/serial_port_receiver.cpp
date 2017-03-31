#include <location/providers/ubx/_8/serial_port_receiver.h>

#include <location/logging.h>

#include <iostream>
#include <system_error>

namespace ubx = location::providers::ubx;

std::shared_ptr<ubx::_8::SerialPortReceiver> ubx::_8::SerialPortReceiver::create(
    boost::asio::io_service& ios, const boost::filesystem::path& dev, const std::shared_ptr<Monitor>& monitor)
{
    return std::shared_ptr<SerialPortReceiver>{new SerialPortReceiver{ios, dev, monitor}};
}

ubx::_8::SerialPortReceiver::SerialPortReceiver(boost::asio::io_service& ios, const boost::filesystem::path& dev,
                                                const std::shared_ptr<Monitor>& monitor)
    : Receiver{monitor}, ios{ios}, serial_port{ios, dev.string().c_str()}
{
    // serial_port.set_option(boost::asio::serial_port::baud_rate(9600));
}

void ubx::_8::SerialPortReceiver::send_encoded_message(const std::vector<std::uint8_t>& data)
{
    auto thiz = shared_from_this();
    std::weak_ptr<SerialPortReceiver> wp{thiz};

    ios.dispatch([this, wp, data]()
    {
        if (auto sp = wp.lock())
            boost::asio::write(serial_port, boost::asio::buffer(data), boost::asio::transfer_all());
    });
}

void ubx::_8::SerialPortReceiver::start()
{
    ::tcflush(serial_port.lowest_layer().native_handle(), TCIFLUSH);
    start_read();
}

void ubx::_8::SerialPortReceiver::stop()
{
    serial_port.cancel();
}

void ubx::_8::SerialPortReceiver::start_read()
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

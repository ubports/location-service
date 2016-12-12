#include <location/providers/ubx/_8/serial_port_receiver.h>

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
    : Receiver{monitor}, ios{ios}, sp{ios, dev.string().c_str()}
{
    sp.set_option(boost::asio::serial_port::baud_rate(9600));
}

void ubx::_8::SerialPortReceiver::start()
{
    auto flush_rc = ::tcflush(sp.lowest_layer().native_handle(), TCIOFLUSH);
    start_read();
    if (flush_rc) throw std::system_error(errno, std::system_category());
}

void ubx::_8::SerialPortReceiver::stop() { sp.cancel(); }

void ubx::_8::SerialPortReceiver::start_read()
{
    auto thiz = shared_from_this();
    boost::asio::async_read(sp, boost::asio::buffer(&buffer.front(), buffer.size()),
                            [thiz, this](const boost::system::error_code& ec, std::size_t transferred) {
                                if (ec == boost::asio::error::operation_aborted)
                                    return;

                                std::cerr << "Received chunk: " << std::endl;
                                std::cerr << std::string(buffer.begin(), buffer.begin() + transferred) << std::endl;

                                if (not ec)
                                    process_chunk(buffer.begin(), buffer.begin() + transferred);

                                start_read();
                            });
}

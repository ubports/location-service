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
    : Receiver{monitor}, ios{ios}, sp{ios, dev.string().c_str()}
{
}

void ubx::_8::SerialPortReceiver::send_encoded_message(const std::vector<std::uint8_t>& data)
{
    ios.dispatch([this, data]()
    {
        boost::asio::write(sp, boost::asio::buffer(data), boost::asio::transfer_all());
    });
}

void ubx::_8::SerialPortReceiver::start()
{
    start_read();
}

void ubx::_8::SerialPortReceiver::stop()
{
    sp.cancel();
}

void ubx::_8::SerialPortReceiver::start_read()
{
    auto thiz = shared_from_this();
    boost::asio::async_read(sp, boost::asio::buffer(&buffer.front(), buffer.size()),
                            [thiz, this](const boost::system::error_code& ec, std::size_t transferred) {
                                if (ec == boost::asio::error::operation_aborted)
                                    return;
                                if (not ec)
                                {
                                    try
                                    {
                                        process_chunk(buffer.begin(), buffer.begin() + transferred);
                                    }
                                    catch(const std::exception& e)
                                    {
                                        LOG(WARNING) << "Error processing NMEA chunk: " << e.what();
                                    }
                                    catch(...)
                                    {
                                        LOG(WARNING) << "Error processing NMEA chunk.";
                                    }

                                    start_read();
                                }
                            });
}

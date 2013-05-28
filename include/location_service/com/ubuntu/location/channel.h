#ifndef LOCATION_SERVICE_COM_UBUNTU_LOCATION_CHANNEL_H_
#define LOCATION_SERVICE_COM_UBUNTU_LOCATION_CHANNEL_H_

#include <boost/signals2.hpp>

#include <memory>
#include <set>

namespace com
{
namespace ubuntu
{
namespace location
{
template<typename T>
using Channel = boost::signals2::signal<void(const T&)>;

typedef boost::signals2::scoped_connection ScopedChannelConnection;
typedef boost::signals2::connection ChannelConnection;
}
}
}

#endif // LOCATION_SERVICE_COM_UBUNTU_LOCATION_CHANNEL_H_

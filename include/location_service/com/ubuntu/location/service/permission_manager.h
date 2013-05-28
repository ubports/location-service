#ifndef LOCATION_SERVICE_COM_UBUNTU_LOCATION_SERVICE_PERMISSION_MANAGER_H_
#define LOCATION_SERVICE_COM_UBUNTU_LOCATION_SERVICE_PERMISSION_MANAGER_H_

#include "com/ubuntu/location/channel.h"
#include "com/ubuntu/location/provider_selection_policy.h"

#include <functional>

namespace com
{
namespace ubuntu
{
namespace location
{
namespace service
{
struct Credentials
{
    pid_t pid;
    uid_t uid;
};

class PermissionManager
{
public:
    typedef std::shared_ptr<PermissionManager> Ptr;

    enum class Result
    {
        granted,
        rejected
    };

    virtual ~PermissionManager() = default;
    PermissionManager(const PermissionManager&) = delete;
    PermissionManager& operator=(const PermissionManager&) = delete;

    virtual Result check_permission_for_credentials(
        const Criteria& criteria,
        const Credentials& credentials) = 0;
    
protected:
    PermissionManager() = default;
};
}
}
}
}
#endif // LOCATION_SERVICE_COM_UBUNTU_LOCATION_SERVICE_PERMISSION_MANAGER_H_

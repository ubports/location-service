#ifndef LOCATION_SERVICE_COM_UBUNTU_LOCATION_SERVICE_DEFAULT_PERMISSION_MANAGER_H_
#define LOCATION_SERVICE_COM_UBUNTU_LOCATION_SERVICE_DEFAULT_PERMISSION_MANAGER_H_

#include "com/ubuntu/location/service/permission_manager.h"

#include <sys/types.h>
#include <unistd.h>

namespace com
{
namespace ubuntu
{
namespace location
{
namespace service
{
class DefaultPermissionManager : public PermissionManager
{
  public:
    typedef std::shared_ptr<DefaultPermissionManager> Ptr;

    DefaultPermissionManager();
    ~DefaultPermissionManager() noexcept;
    
    Result check_permission_for_credentials(
    	const Criteria&,
        const Credentials& credentials);

  private:
    pid_t pid;
    uid_t uid;
};
}
}
}
}
#endif // LOCATION_SERVICE_COM_UBUNTU_LOCATION_SERVICE_DEFAULT_PERMISSION_MANAGER_H_

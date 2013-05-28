#include "com/ubuntu/location/default_permission_manager.h"

namespace cul = com::ubuntu::location;

cul::DefaultPermissionManager::DefaultPermissionManager()
        : pid(getpid()),
          uid(getuid())
{
}

cul::DefaultPermissionManager::~DefaultPermissionManager() noexcept
{
}

cul::PermissionManager::Result cul::DefaultPermissionManager::check_permission_for_credentials(
    const cul::Criteria&,
    const cul::Credentials& credentials)
{
    if (credentials.pid != pid || credentials.uid != uid)
        return Result::granted; // FIXME(tvoss): This should return rejected.
    return Result::granted;
}

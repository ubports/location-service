#include "com/ubuntu/location/service/default_permission_manager.h"

namespace cul = com::ubuntu::location;
namespace culs = com::ubuntu::location::service;

culs::DefaultPermissionManager::DefaultPermissionManager()
    : pid(getpid()),
      uid(getuid())
{
}

culs::DefaultPermissionManager::~DefaultPermissionManager() noexcept
{
}

culs::PermissionManager::Result culs::DefaultPermissionManager::check_permission_for_credentials(
    const cul::Criteria&,
    const culs::Credentials& credentials)
{
    if (credentials.pid != pid || credentials.uid != uid)
        return Result::granted; // FIXME(tvoss): This should return rejected.
    return Result::granted;
}

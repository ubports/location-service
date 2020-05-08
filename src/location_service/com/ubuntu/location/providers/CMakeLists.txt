add_subdirectory(dummy)
add_subdirectory(remote)
add_subdirectory(geoclue)
add_subdirectory(gps)
add_subdirectory(skyhook)
add_subdirectory(mls)
add_subdirectory(gpsd)

set(
  ENABLED_PROVIDER_TARGETS
  ${ENABLED_PROVIDER_TARGETS}
  PARENT_SCOPE
)

set(
  ENABLED_PROVIDER_TARGETS_DEFINITIONS
  ${ENABLED_PROVIDER_TARGETS_DEFINITIONS}
  PARENT_SCOPE
)

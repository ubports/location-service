add_subdirectory(dummy)
add_subdirectory(remote)
add_subdirectory(geoclue)
add_subdirectory(gps)
add_subdirectory(skyhook)

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

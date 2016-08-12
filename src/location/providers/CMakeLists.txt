add_subdirectory(dummy)
add_subdirectory(fusion)
add_subdirectory(remote)
add_subdirectory(gps)

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

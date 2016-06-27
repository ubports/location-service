option(
    LOCATION_ENABLE_GEOCLUE_PROVIDER
    "Enable location provider connecting to existing geoclue providers"
    ON
)

if (LOCATION_ENABLE_GEOCLUE_PROVIDER)

  message(STATUS "Enabling support for Geoclue location providers")

  add_library(geoclue provider.cpp)

  set(
    ENABLED_PROVIDER_TARGETS
    ${ENABLED_PROVIDER_TARGETS} geoclue
    PARENT_SCOPE
  )

  set(
    ENABLED_PROVIDER_TARGETS_DEFINITIONS
    -DLOCATION_PROVIDERS_GEOCLUE ${ENABLED_PROVIDER_TARGETS_DEFINITIONS}
    PARENT_SCOPE
  )
endif (LOCATION_ENABLE_GEOCLUE_PROVIDER)

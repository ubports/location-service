option(
    LOCATION_SERVICE_ENABLE_MLS_PROVIDER
    "Enable location provider connecting to existing mls providers"
    ON
)

if (LOCATION_SERVICE_ENABLE_MLS_PROVIDER)

  message(STATUS "Enabling support for mls location providers")

  add_library(
    mls
    provider.h
    provider.cpp)

  set(
    ENABLED_PROVIDER_TARGETS
    ${ENABLED_PROVIDER_TARGETS} mls
    PARENT_SCOPE
  )
  
  target_link_libraries(
    mls
  
    ichnaea)
  
  set(
    ENABLED_PROVIDER_TARGETS_DEFINITIONS
    -DCOM_UBUNTU_LOCATION_SERVICE_PROVIDERS_MLS ${ENABLED_PROVIDER_TARGETS_DEFINITIONS}
    PARENT_SCOPE
  )
endif (LOCATION_SERVICE_ENABLE_MLS_PROVIDER)

option(
    LOCATION_SERVICE_ENABLE_GPSD_PROVIDER
    "Enable location provider connecting to existing gpsd providers"
    ON
)

if (LOCATION_SERVICE_ENABLE_GPSD_PROVIDER)

  message(STATUS "Enabling support for gpsd location providers")

  add_library(gpsd provider.cpp)

  set(
    ENABLED_PROVIDER_TARGETS
    ${ENABLED_PROVIDER_TARGETS} gpsd
    PARENT_SCOPE
  )

  target_link_libraries(gpsd ${GPSD_LDFLAGS})

  set(
    ENABLED_PROVIDER_TARGETS_DEFINITIONS
    -DCOM_UBUNTU_LOCATION_SERVICE_PROVIDERS_GPSD ${ENABLED_PROVIDER_TARGETS_DEFINITIONS}
    PARENT_SCOPE
  )
endif (LOCATION_SERVICE_ENABLE_GPSD_PROVIDER)

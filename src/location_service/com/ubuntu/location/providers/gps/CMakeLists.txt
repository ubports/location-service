option(
    LOCATION_SERVICE_ENABLE_GPS_PROVIDER
    "Enable location providers relying on the Android HAL to connect to GPS HW"
    ON
)

if (LOCATION_SERVICE_ENABLE_GPS_PROVIDER)
  find_file(
    UBUNTU_APPLICATION_GPS_H ubuntu_application_gps.h
    NAMES ubuntu/application/ubuntu_application_gps.h
  )

  if (UBUNTU_APPLICATION_GPS_H)
    message(STATUS "Enabling GPS location provider")

    add_library(gps provider.cpp)

    target_link_libraries(gps ubuntu_application_api)
  
    set(
      ENABLED_PROVIDER_TARGETS
      ${ENABLED_PROVIDER_TARGETS} gps
      PARENT_SCOPE)

    set(
      ENABLED_PROVIDER_TARGETS_DEFINITIONS
      -DCOM_UBUNTU_LOCATION_SERVICE_PROVIDERS_GPS ${ENABLED_PROVIDER_TARGETS_DEFINITIONS}
      PARENT_SCOPE
    )
  endif (UBUNTU_APPLICATION_GPS_H)
endif (LOCATION_SERVICE_ENABLE_GPS_PROVIDER)
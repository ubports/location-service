option(
    LOCATION_SERVICE_ENABLE_GPS_PROVIDER
    "Enable location providers relying on the Android HAL to connect to GPS HW"
    ON
)

if (LOCATION_SERVICE_ENABLE_GPS_PROVIDER)
  find_file(
    UBUNTU_HARDWARE_GPS_H gps.h
    NAMES ubuntu/hardware/gps.h
  )

  if (UBUNTU_HARDWARE_GPS_H)
    message(STATUS "Enabling GPS location provider")

    add_library(
        gps

        gps.conf

        android_hardware_abstraction_layer.h
        android_hardware_abstraction_layer.cpp

        hardware_abstraction_layer.h
        # hardware_abstraction_layer.cpp

        provider.h
        provider.cpp)

    target_link_libraries(gps ubuntu_platform_hardware_api.so)
  
    set(
      ENABLED_PROVIDER_TARGETS
      ${ENABLED_PROVIDER_TARGETS} gps
      PARENT_SCOPE)

    set(
      ENABLED_PROVIDER_TARGETS_DEFINITIONS
      -DCOM_UBUNTU_LOCATION_SERVICE_PROVIDERS_GPS ${ENABLED_PROVIDER_TARGETS_DEFINITIONS}
      PARENT_SCOPE
    )

    install(FILES gps.conf DESTINATION /etc/)
  endif (UBUNTU_HARDWARE_GPS_H)
endif (LOCATION_SERVICE_ENABLE_GPS_PROVIDER)

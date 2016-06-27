option(
    LOCATION_ENABLE_GPS_PROVIDER
    "Enable location providers relying on the Android HAL to connect to GPS HW"
    ON
)

if ((LOCATION_ENABLE_GPS_PROVIDER) AND (UBUNTU_PLATFORM_HARDWARE_API_FOUND))
    message(STATUS "Enabling GPS location provider")

    include_directories(${UBUNTU_PLATFORM_HARDWARE_API_INCLUDE_DIRS})

    add_library(
        gps

        gps.conf

        hardware_abstraction_layer.cpp

        android_hardware_abstraction_layer.h
        android_hardware_abstraction_layer.cpp

        sntp_reference_time_source.h
        sntp_reference_time_source.cpp

        sntp_client.h
        sntp_client.cpp

        hardware_abstraction_layer.h
        # hardware_abstraction_layer.cpp

        provider.h
        provider.cpp)

    target_link_libraries(gps ${UBUNTU_PLATFORM_HARDWARE_API_LDFLAGS})
  
    set(
      ENABLED_PROVIDER_TARGETS
      ${ENABLED_PROVIDER_TARGETS} gps
      PARENT_SCOPE)

    set(
      ENABLED_PROVIDER_TARGETS_DEFINITIONS
      -DLOCATION_PROVIDERS_GPS ${ENABLED_PROVIDER_TARGETS_DEFINITIONS}
      PARENT_SCOPE
    )

    install(FILES gps.conf DESTINATION /etc/)
endif ()

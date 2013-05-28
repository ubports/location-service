option(
    LOCATION_SERVICE_ENABLE_SKYHOOK_PROVIDER
    "Enable location provider relying on the Skyhook SDK"
    ON
)

if (LOCATION_SERVICE_ENABLE_SKYHOOK_PROVIDER)

  pkg_check_modules(WPSAPI wpsapi)

  if (WPSAPI_FOUND)
    message(STATUS "Enabling support for Skyhook location providers")
    
    add_definitions(-DCOM_UBUNTU_LOCATION_SERVICE_PROVIDERS_SKYHOOK)

    include_directories(${WPSAPI_INCLUDE_DIRS})
    link_directories(${WPSAPI_LIBRARY_DIRS})

    add_library(skyhook provider.cpp)
  
    target_link_libraries(
      skyhook 

      ${WPSAPI_LIBRARIES}
      iw
    )

    set(
      ENABLED_PROVIDER_TARGETS
      skyhook ${ENABLED_PROVIDER_TARGETS}
      PARENT_SCOPE
    )

    set(
      ENABLED_PROVIDER_TARGETS_DEFINITIONS
      -DCOM_UBUNTU_LOCATION_SERVICE_PROVIDERS_SKYHOOK ${ENABLED_PROVIDER_TARGETS_DEFINITIONS}
      PARENT_SCOPE
    )
  endif (WPSAPI_FOUND)
endif (LOCATION_SERVICE_ENABLE_SKYHOOK_PROVIDER)

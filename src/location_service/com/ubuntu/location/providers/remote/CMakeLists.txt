option(
    LOCATION_SERVICE_ENABLE_REMOTE_PROVIDER
    "Enable location provider relying on the remote provider SDK"
    ON
)

if (LOCATION_SERVICE_ENABLE_REMOTE_PROVIDER)

    message(STATUS "Enabling support for the remote location providers")

    set(REMOTE_SOURCES
    	provider.cpp
    )

    set(REMOTE_HEADERS
    	remote_interface.h
    	provider.h
    )

    add_library(remote
    	${REMOTE_HEADERS}
    	${REMOTE_SOURCES}
    )

    set(
            ENABLED_PROVIDER_TARGETS
            ${ENABLED_PROVIDER_TARGETS} remote
            PARENT_SCOPE
    )

   set(
            ENABLED_PROVIDER_TARGETS_DEFINITIONS
            -DCOM_UBUNTU_LOCATION_SERVICE_PROVIDERS_REMOTE ${ENABLED_PROVIDER_TARGETS_DEFINITIONS}
            PARENT_SCOPE
   )

endif (LOCATION_SERVICE_ENABLE_REMOTE_PROVIDER)

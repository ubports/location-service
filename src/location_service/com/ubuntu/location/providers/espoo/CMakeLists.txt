option(
    LOCATION_SERVICE_ENABLE_ESPOO_PROVIDER
    "Enable location provider relying on the Espoo SDK"
    ON
)

if (LOCATION_SERVICE_ENABLE_ESPOO_PROVIDER)

        message(STATUS "Enabling support for Espoo location providers")

        set(ESPOO_SOURCES
        	provider.cpp
        )

        set(ESPOO_HEADERS
        	espoo.h
        	macros.h
        	provider.h
        )

        add_library(espoo
        	${ESPOO_HEADERS}
        	${ESPOO_SOURCES}
          )

        set(
          ENABLED_PROVIDER_TARGETS
          ${ENABLED_PROVIDER_TARGETS} espoo
          PARENT_SCOPE)

       set(
         ENABLED_PROVIDER_TARGETS_DEFINITIONS
         -DCOM_UBUNTU_LOCATION_SERVICE_PROVIDERS_ESPOO ${ENABLED_PROVIDER_TARGETS_DEFINITIONS}
         PARENT_SCOPE
       )

endif (LOCATION_SERVICE_ENABLE_ESPOO_PROVIDER)

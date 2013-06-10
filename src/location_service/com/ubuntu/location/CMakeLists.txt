add_subdirectory(providers)

add_library(
  ubuntu-location-service SHARED

  default_provider_selection_policy.cpp
  
  engine.cpp
  init_and_shutdown.cpp
  position.cpp
  provider.cpp
  provider_factory.cpp
  proxy_provider.cpp

  service/default_configuration.cpp
  service/default_permission_manager.cpp
  service/implementation.cpp
  service/skeleton.cpp
  service/stub.cpp

  service/session/implementation.cpp
  service/session/interface.cpp
  service/session/skeleton.cpp
  service/session/stub.cpp

  providers/config.cpp
)

set_target_properties(
  ubuntu-location-service

  PROPERTIES
  VERSION ${UBUNTU_LOCATION_SERVICE_VERSION_MAJOR}.${UBUNTU_LOCATION_SERVICE_VERSION_MINOR}.${UBUNTU_LOCATION_SERVICE_VERSION_PATCH}
  SOVERSION ${UBUNTU_LOCATION_SERVICE_VERSION_MAJOR}
)

add_definitions(${ENABLED_PROVIDER_TARGETS_DEFINITIONS})

target_link_libraries(
  ubuntu-location-service

  ${ENABLED_PROVIDER_TARGETS}

  ${Boost_LIBRARIES}
  ${DBUS_LIBRARIES}
  ${GLog_LIBRARY}
)

install(
  TARGETS ubuntu-location-service
  DESTINATION lib
)

add_subdirectory(providers)

add_library(
  ubuntu-location-service SHARED

  default_provider_selection_policy.cpp
  
  criteria.cpp
  engine.cpp
  init_and_shutdown.cpp
  position.cpp
  provider.cpp
  provider_factory.cpp
  proxy_provider.cpp
  set_name_for_thread.cpp

  connectivity/ofono_nm_connectivity_manager.cpp

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
  ${DBUS_CPP_LIBRARIES}
  ${GLog_LIBRARY}
  ${GFlags_LIBRARY}
)

install(
  TARGETS ubuntu-location-service
  DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

add_library(
  ubuntu-location-service-daemon

  service/daemon
)

add_executable(
  ubuntu-location-serviced

  service/daemon_main.cpp
)

add_executable(
  ubuntu-location-serviced-cli

  service/daemon_cli_main.cpp
)

target_link_libraries(
  ubuntu-location-service-daemon

  ubuntu-location-service
)

target_link_libraries(
  ubuntu-location-serviced

  ubuntu-location-service-daemon

  ${ENABLED_PROVIDER_TARGETS}

  ${Boost_LIBRARIES}
  ${DBUS_LIBRARIES}
  ${DBUS_CPP_LIBRARIES}
  ${GLog_LIBRARY}
  ${GFlags_LIBRARY}
)

target_link_libraries(
  ubuntu-location-serviced-cli

  ubuntu-location-service-daemon

  ${ENABLED_PROVIDER_TARGETS}

  ${Boost_LIBRARIES}
  ${DBUS_LIBRARIES}
  ${DBUS_CPP_LIBRARIES}
  ${GLog_LIBRARY}
  ${GFlags_LIBRARY}
)

install(
  TARGETS ubuntu-location-serviced
  DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(
  TARGETS ubuntu-location-serviced-cli
  DESTINATION ${CMAKE_INSTALL_BINDIR}
)


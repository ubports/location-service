if (UBUNTU_PLATFORM_HARDWARE_API_FOUND)
  add_definitions(-DCOM_UBUNTU_LOCATION_SERVICE_HAVE_UBUNTU_PLATFORM_HARDWARE_API)
endif()

if (NET_CPP_FOUND)
  add_definitions(-DCOM_UBUNTU_LOCATION_SERVICE_HAVE_NET_CPP=1)

  set(ICHNAEA_REPORTER_SRCS service/ichnaea_reporter.cpp)
endif()

add_subdirectory(providers)

configure_file(
  service/config.h.in ${CMAKE_CURRENT_BINARY_DIR}/service/config.h @ONLY
)

include_directories(${UBUNTU_PLATFORM_HARDWARE_API_INCLUDE_DIRS})

add_library(
  ubuntu-location-service SHARED

  ${UBUNTU_LOCATION_SERVICE_PUBLIC_HEADERS}

  default_provider_selection_policy.cpp
  fusion_provider_selection_policy.cpp
  non_selecting_provider_selection_policy.cpp

  criteria.cpp
  engine.cpp
  fusion_provider.cpp
  init_and_shutdown.cpp
  position.cpp
  provider.cpp
  provider_factory.cpp
  proxy_provider.cpp
  satellite_based_positioning_state.cpp
  settings.cpp
  time_based_update_policy.cpp
  set_name_for_thread.cpp
  time_since_boot.cpp
  wifi_and_cell_reporting_state.cpp

  boost_ptree_settings.cpp

  service/default_configuration.cpp
  service/default_permission_manager.cpp
  service/harvester.cpp
  service/demultiplexing_reporter.h
  service/demultiplexing_reporter.cpp
  service/runtime.cpp
  service/runtime_tests.h
  service/runtime_tests.cpp
  service/state.cpp
  service/trust_store_permission_manager.cpp

  service/implementation.cpp
  service/skeleton.cpp
  service/stub.cpp

  service/session/implementation.cpp
  service/session/interface.cpp
  service/session/skeleton.cpp
  service/session/stub.cpp

  providers/config.cpp

  providers/remote/provider.cpp
  providers/remote/skeleton.cpp
  providers/remote/stub.cpp

  ${ICHNAEA_REPORTER_SRCS}
)

add_library(
  ubuntu-location-service-connectivity SHARED

  set_name_for_thread.cpp

  connectivity/cached_radio_cell.cpp
  connectivity/cached_wireless_network.cpp
  connectivity/manager.cpp
  connectivity/radio_cell.cpp
  connectivity/wireless_network.cpp

  connectivity/ofono_nm_connectivity_manager.cpp
)

set(symbol_map "${CMAKE_SOURCE_DIR}/symbols.map")

set_target_properties(
  ubuntu-location-service

  PROPERTIES
  LINK_FLAGS "${ldflags} -Wl,--version-script,${symbol_map}"
  LINK_DEPENDS ${symbol_map}
  VERSION ${UBUNTU_LOCATION_SERVICE_VERSION_MAJOR}.${UBUNTU_LOCATION_SERVICE_VERSION_MINOR}.${UBUNTU_LOCATION_SERVICE_VERSION_PATCH}
  SOVERSION ${UBUNTU_LOCATION_SERVICE_VERSION_MAJOR}
)

set_target_properties(
  ubuntu-location-service-connectivity

  PROPERTIES
  LINK_FLAGS "${ldflags} -Wl,--version-script,${symbol_map}"
  LINK_DEPENDS ${symbol_map}
  VERSION ${UBUNTU_LOCATION_SERVICE_VERSION_MAJOR}.${UBUNTU_LOCATION_SERVICE_VERSION_MINOR}.${UBUNTU_LOCATION_SERVICE_VERSION_PATCH}
  SOVERSION ${UBUNTU_LOCATION_SERVICE_VERSION_MAJOR}
)

add_definitions(${ENABLED_PROVIDER_TARGETS_DEFINITIONS})

target_link_libraries(
  ubuntu-location-service-connectivity

  ${CMAKE_THREAD_LIBS_INIT}
  ${Boost_LIBRARIES}
  ${DBUS_LIBRARIES}
  ${DBUS_CPP_LDFLAGS}
  ${PROCESS_CPP_LDFLAGS}
  ${GLog_LIBRARY}
  ${GFlags_LIBRARY}
)

target_link_libraries(
  ubuntu-location-service

  ubuntu-location-service-connectivity

  ${ENABLED_PROVIDER_TARGETS}

  ${CMAKE_THREAD_LIBS_INIT}
  ${Boost_LIBRARIES}
  ${DBUS_LIBRARIES}
  ${DBUS_CPP_LDFLAGS}
  ${JSON_C_LDFLAGS}
  ${LIBAPPARMOR_LDFLAGS}
  ${NET_CPP_LDFLAGS}
  ${TRUST_STORE_LDFLAGS}
  ${UBUNTU_PLATFORM_HARDWARE_API_LDFLAGS}
  ${GLog_LIBRARY}
  ${GFlags_LIBRARY}
)

install(
  TARGETS ubuntu-location-service
  DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

install(
  TARGETS ubuntu-location-service-connectivity
  DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

add_library(
  ubuntu-location-service-provider-daemon

  service/provider_daemon.cpp
)

add_library(
  ubuntu-location-service-daemon

  service/daemon.cpp
)

add_executable(
  ubuntu-location-service-providerd

  service/provider_daemon_main.cpp
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
  ubuntu-location-service-provider-daemon

  ubuntu-location-service
)

target_link_libraries(
  ubuntu-location-service-daemon

  ubuntu-location-service
)

target_link_libraries(
  ubuntu-location-service-providerd

  ubuntu-location-service-provider-daemon

  ${ENABLED_PROVIDER_TARGETS}

  ${Boost_LIBRARIES}
  ${DBUS_LIBRARIES}
  ${DBUS_CPP_LIBRARIES}
  ${GLog_LIBRARY}
  ${GFlags_LIBRARY}
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
  TARGETS ubuntu-location-service-providerd
  DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(
  TARGETS ubuntu-location-serviced
  DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(
  TARGETS ubuntu-location-serviced-cli
  DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if (UBUNTU_PLATFORM_HARDWARE_API_FOUND)
  add_definitions(-DLOCATION_HAVE_UBUNTU_PLATFORM_HARDWARE_API)
endif()

add_subdirectory(providers)

configure_file(
  config.h.in ${CMAKE_CURRENT_BINARY_DIR}/config.h @ONLY
)

include_directories(${UBUNTU_PLATFORM_HARDWARE_API_INCLUDE_DIRS})

add_library(
  ubuntu-location-service SHARED

  ${UBUNTU_LOCATION_SERVICE_PUBLIC_HEADERS}

  fusion_provider_selection_policy.cpp
  non_selecting_provider_selection_policy.cpp

  engine.cpp
  event.cpp
  position.cpp
  provider.cpp
  provider_factory.cpp
  satellite_based_positioning_state.cpp
  settings.cpp
  time_based_update_policy.cpp
  set_name_for_thread.cpp
  time_since_boot.cpp
  wifi_and_cell_reporting_state.cpp

  boost_ptree_settings.cpp

  daemon.h
  daemon.cpp
  service.cpp

  permission_manager.h
  runtime.h
  runtime.cpp
  runtime_tests.h
  runtime_tests.cpp
  serializing_bus.h
  serializing_bus.cpp
  service_with_engine.h
  service_with_engine.cpp
  session_with_provider.h
  session_with_provider.cpp

  trust_store_permission_manager.h
  trust_store_permission_manager.cpp

  cmds/list.h
  cmds/list.cpp
  cmds/monitor.h
  cmds/monitor.cpp
  cmds/provider.h
  cmds/provider.cpp
  cmds/run.h
  cmds/run.cpp
  cmds/status.h
  cmds/status.cpp
  cmds/test.h
  cmds/test.cpp

  util/cli.h
  util/cli.cpp
  util/do_not_copy_or_move.h
  util/well_known_bus.h
  util/well_known_bus.cpp

  dbus/skeleton/service.h
  dbus/skeleton/service.cpp
  dbus/skeleton/session.h
  dbus/skeleton/session.cpp

  dbus/stub/service.h
  dbus/stub/service.cpp
  dbus/stub/session.h
  dbus/stub/session.cpp

  events/all.h
  events/registry.h
  events/registry.cpp
  events/reference_position_updated.cpp
  events/wifi_and_cell_id_reporting_state_changed.cpp

  providers/config.cpp

  providers/fusion/provider.h
  providers/fusion/provider.cpp
  providers/remote/provider.cpp
  providers/remote/skeleton.cpp
  providers/remote/stub.cpp
  providers/proxy.h
  providers/proxy.cpp
  providers/state_tracking_provider.h
  providers/state_tracking_provider.cpp
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

add_executable(
  locationd

  daemon_main.cpp
)

target_link_libraries(
  locationd

  ubuntu-location-service

  ${ENABLED_PROVIDER_TARGETS}
)

install(
  TARGETS locationd
  DESTINATION ${CMAKE_INSTALL_BINDIR}
)

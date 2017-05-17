find_package(Threads)

add_definitions(-DCORE_DBUS_ENABLE_GOOGLE_TEST_FIXTURE -DBOOST_ASIO_DISABLE_MOVE)

# Build mongoose as static library to leverage in location reporter testing
add_library(mongoose mongoose.c)
# Explicitly select c99 when compiling mongoose
set_target_properties(
    mongoose
    PROPERTIES COMPILE_FLAGS "-std=c99")

# Build with system gmock and embedded gtest
find_package(GMock)

macro(LOCATION_SERVICE_ADD_TEST test_name src)
  add_executable(
    ${test_name}
    ${src})

  target_link_libraries(
    ${test_name}

    ubuntu-location-service
    ubuntu-location-service-connectivity

    mongoose

    ${Boost_LIBRARIES}
    ${CMAKE_THREAD_LIBS_INIT}

    ${PROCESS_CPP_LIBRARIES}

    ${ARGN}

    ${GMOCK_LIBRARIES}
    ${GTEST_BOTH_LIBRARIES})

  add_test(${test_name} ${CMAKE_CURRENT_BINARY_DIR}/${test_name} --gtest_filter=*-*requires*)

  # Address android's limit on cmdline length and distill ubuntu-location-service into uls
  install(
    TARGETS ${test_name}
    DESTINATION ${CMAKE_INSTALL_BINDIR}/uls-tests
  )
endmacro(LOCATION_SERVICE_ADD_TEST)

include_directories (
  ${CMAKE_SOURCE_DIR}/src/location_service

  ${GMOCK_INCLUDE_DIR}
  ${GTEST_INCLUDE_DIR}
)

LOCATION_SERVICE_ADD_TEST(acceptance_tests acceptance_tests.cpp)
LOCATION_SERVICE_ADD_TEST(boost_ptree_settings_test boost_ptree_settings_test.cpp)
LOCATION_SERVICE_ADD_TEST(daemon_and_cli_tests daemon_and_cli_tests.cpp)
LOCATION_SERVICE_ADD_TEST(engine_test engine_test.cpp)
LOCATION_SERVICE_ADD_TEST(event_test event_test.cpp)
LOCATION_SERVICE_ADD_TEST(time_based_update_policy_test time_based_update_policy_test.cpp)

LOCATION_SERVICE_ADD_TEST(position_test position_test.cpp)
LOCATION_SERVICE_ADD_TEST(provider_registry_test provider_registry_test.cpp)
LOCATION_SERVICE_ADD_TEST(provider_test provider_test.cpp)
LOCATION_SERVICE_ADD_TEST(reference_position_updated_test reference_position_updated_test.cpp)
LOCATION_SERVICE_ADD_TEST(runtime_test runtime_test.cpp)
LOCATION_SERVICE_ADD_TEST(state_tracking_provider_test state_tracking_provider_test.cpp)
LOCATION_SERVICE_ADD_TEST(wifi_and_cell_id_reporting_state_changed_test wifi_and_cell_id_reporting_state_changed_test.cpp)
# Provider-specific test-cases go here.
if (LOCATION_SERVICE_ENABLE_GPS_PROVIDER)
  include_directories(${CMAKE_SOURCE_DIR}/src/location_service)
  LOCATION_SERVICE_ADD_TEST(gps_provider_test gps_provider_test.cpp)
  LOCATION_SERVICE_ADD_TEST(sntp_client_test sntp_client_test.cpp)
endif(LOCATION_SERVICE_ENABLE_GPS_PROVIDER)

if (LOCATION_SERVICE_ENABLE_GEOCLUE_PROVIDERS)
  add_executable(geoclue_provider_test geoclue_provider_test.cpp)
  target_link_libraries(
    geoclue_provider_test
    
    ubuntu-location-service

    ${CMAKE_THREAD_LIBS_INIT}
    ${Boost_LIBRARIES}
    ${PROCESS_CPP_LIBRARIES}

    ${GTEST_BOTH_LIBRARIES}
  )
  if (LOCATION_SERVICE_ENABLE_DBUS_TEST_RUNNER)
    add_test(geoclue_provider_test ${DBUS_TEST_RUNNER_EXECUTABLE} --task=${CMAKE_CURRENT_BINARY_DIR}/geoclue_provider_test)
  else (LOCATION_SERVICE_ENABLE_DBUS_TEST_RUNNER)
    add_test(geoclue_provider_test ${CMAKE_CURRENT_BINARY_DIR}/geoclue_provider_test)
  endif (LOCATION_SERVICE_ENABLE_DBUS_TEST_RUNNER)
endif (LOCATION_SERVICE_ENABLE_GEOCLUE_PROVIDERS)

LOCATION_SERVICE_ADD_TEST(serializing_bus_test serializing_bus_test.cpp)

# LOCATION_SERVICE_ADD_TEST(remote_provider_test remote_provider_test.cpp)
# LOCATION_SERVICE_ADD_TEST(espoo_provider_test espoo_provider_test.cpp)
LOCATION_SERVICE_ADD_TEST(bug_1447110 bug_1447110.cpp)

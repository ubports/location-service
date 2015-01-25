find_package(Threads)

add_definitions(-DCORE_DBUS_ENABLE_GOOGLE_TEST_FIXTURE -DBOOST_ASIO_DISABLE_MOVE)

option(
    LOCATION_SERVICE_ENABLE_DBUS_TEST_RUNNER 
    "Rely on dbus test runner to start a private session for testing purposes" 
    ON
)

if (LOCATION_SERVICE_ENABLE_DBUS_TEST_RUNNER)
    find_program(DBUS_TEST_RUNNER_EXECUTABLE dbus-test-runner)
    message(STATUS "Executing test suite under dbus-test-runner")
endif (LOCATION_SERVICE_ENABLE_DBUS_TEST_RUNNER)

# Build mongoose as static library to leverage in location reporter testing
add_library(mongoose mongoose.c)
# Explicitly select c99 when compiling mongoose
set_target_properties(
    mongoose
    PROPERTIES COMPILE_FLAGS "-std=c99")

# Build with system gmock and embedded gtest
set (GMOCK_INCLUDE_DIR "/usr/include/gmock/include" CACHE PATH "gmock source include directory")
set (GMOCK_SOURCE_DIR "/usr/src/gmock" CACHE PATH "gmock source directory")
set (GTEST_INCLUDE_DIR "${GMOCK_SOURCE_DIR}/gtest/include" CACHE PATH "gtest source include directory")

add_subdirectory(${GMOCK_SOURCE_DIR} "${CMAKE_CURRENT_BINARY_DIR}/gmock")

macro(LOCATION_SERVICE_ADD_TEST test_name src)
  add_executable(
    ${test_name}
    ${src})

  target_link_libraries(
    ${test_name}

    ubuntu-location-service
    ubuntu-location-service-connectivity
    ubuntu-location-service-provider-daemon
    ubuntu-location-service-daemon

    mongoose

    ${Boost_LIBRARIES}
    ${CMAKE_THREAD_LIBS_INIT}

    ${PROCESS_CPP_LIBRARIES}

    ${ARGN}

    gmock

    gtest
    gtest_main)

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
LOCATION_SERVICE_ADD_TEST(connectivity_manager_test connectivity_manager_test.cpp)
LOCATION_SERVICE_ADD_TEST(controller_test controller_test.cpp)
LOCATION_SERVICE_ADD_TEST(criteria_test criteria_test.cpp)
LOCATION_SERVICE_ADD_TEST(daemon_and_cli_tests daemon_and_cli_tests.cpp)
LOCATION_SERVICE_ADD_TEST(default_permission_manager_test default_permission_manager_test.cpp)
LOCATION_SERVICE_ADD_TEST(engine_test engine_test.cpp)
LOCATION_SERVICE_ADD_TEST(harvester_test harvester_test.cpp)
LOCATION_SERVICE_ADD_TEST(demultiplexing_reporter_test demultiplexing_reporter_test.cpp)

if (NET_CPP_FOUND)
  LOCATION_SERVICE_ADD_TEST(ichnaea_reporter_test ichnaea_reporter_test.cpp)
endif()

LOCATION_SERVICE_ADD_TEST(position_test position_test.cpp)
LOCATION_SERVICE_ADD_TEST(provider_selection_policy_test provider_selection_policy_test.cpp)
LOCATION_SERVICE_ADD_TEST(provider_factory_test provider_factory_test.cpp)
LOCATION_SERVICE_ADD_TEST(provider_test provider_test.cpp)
LOCATION_SERVICE_ADD_TEST(wgs84_test wgs84_test.cpp)
LOCATION_SERVICE_ADD_TEST(trust_store_permission_manager_test trust_store_permission_manager_test.cpp)

# Provider-specific test-cases go here.
if (LOCATION_SERVICE_ENABLE_GPS_PROVIDER)
  include_directories(${CMAKE_SOURCE_DIR}/src/location_service)
  LOCATION_SERVICE_ADD_TEST(gps_provider_test gps_provider_test.cpp)
endif(LOCATION_SERVICE_ENABLE_GPS_PROVIDER)

if (LOCATION_SERVICE_ENABLE_GEOCLUE_PROVIDERS)
  add_executable(geoclue_provider_test geoclue_provider_test.cpp)
  target_link_libraries(
    geoclue_provider_test
    
    ubuntu-location-service

    ${CMAKE_THREAD_LIBS_INIT}
    ${Boost_LIBRARIES}
    ${PROCESS_CPP_LIBRARIES}

    gtest
    gtest_main
  )
  if (LOCATION_SERVICE_ENABLE_DBUS_TEST_RUNNER)
    add_test(geoclue_provider_test ${DBUS_TEST_RUNNER_EXECUTABLE} --task=${CMAKE_CURRENT_BINARY_DIR}/geoclue_provider_test)
  else (LOCATION_SERVICE_ENABLE_DBUS_TEST_RUNNER)
    add_test(geoclue_provider_test ${CMAKE_CURRENT_BINARY_DIR}/geoclue_provider_test)
  endif (LOCATION_SERVICE_ENABLE_DBUS_TEST_RUNNER)
endif (LOCATION_SERVICE_ENABLE_GEOCLUE_PROVIDERS)

LOCATION_SERVICE_ADD_TEST(remote_providerd_test remote_providerd_test.cpp)

LOCATION_SERVICE_ADD_TEST(remote_provider_test remote_provider_test.cpp)
LOCATION_SERVICE_ADD_TEST(espoo_provider_test espoo_provider_test.cpp)

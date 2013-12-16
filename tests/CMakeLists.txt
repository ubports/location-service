find_package(Threads)

option(
    LOCATION_SERVICE_ENABLE_DBUS_TEST_RUNNER 
    "Rely on dbus test runner to start a private session for testing purposes" 
    ON
)

if (LOCATION_SERVICE_ENABLE_DBUS_TEST_RUNNER)
    find_program(DBUS_TEST_RUNNER_EXECUTABLE dbus-test-runner)
    message(STATUS "Executing test suite under dbus-test-runner")
endif (LOCATION_SERVICE_ENABLE_DBUS_TEST_RUNNER)

# Build with system gmock and embedded gtest
set (GMOCK_INCLUDE_DIR "/usr/include/gmock/include" CACHE PATH "gmock source include directory")
set (GMOCK_SOURCE_DIR "/usr/src/gmock" CACHE PATH "gmock source directory")
set (GTEST_INCLUDE_DIR "${GMOCK_SOURCE_DIR}/gtest/include" CACHE PATH "gtest source include directory")

add_subdirectory(${GMOCK_SOURCE_DIR} "${CMAKE_CURRENT_BINARY_DIR}/gmock")

include_directories (
  ${CMAKE_SOURCE_DIR}/src/location_service

  ${GMOCK_INCLUDE_DIR}
  ${GTEST_INCLUDE_DIR}
)

add_executable(
    acceptance_tests
    acceptance_tests.cpp
    test_main.cpp
)

add_executable(
    controller_test
    controller_test.cpp
    test_main.cpp
)

add_executable(
    daemon_and_cli_tests
    daemon_and_cli_tests.cpp
    test_main.cpp
)

add_executable(
    default_permission_manager_test
    default_permission_manager_test.cpp
    test_main.cpp
)

add_executable(
    engine_test
    engine_test.cpp
    test_main.cpp
)

add_executable(
    position_test
    position_test.cpp
    test_main.cpp
)

add_executable(
    provider_selection_policy_test
    provider_selection_policy_test.cpp
    test_main.cpp
)

add_executable(
    wgs84_test
    wgs84_test.cpp
    test_main.cpp
)

add_executable(
    provider_test
    provider_test.cpp
    test_main.cpp
)

add_executable(
    provider_factory_test
    provider_factory_test.cpp
    test_main.cpp
)

target_link_libraries(
    default_permission_manager_test
    
    ubuntu-location-service

    ${CMAKE_THREAD_LIBS_INIT}
    ${Boost_LIBRARIES}
    ${DBUS_LIBRARIES}
    
    gtest
)

target_link_libraries(
    daemon_and_cli_tests

    ubuntu-location-service-daemon

    ${CMAKE_THREAD_LIBS_INIT}
    ${Boost_LIBRARIES}
    ${DBUS_LIBRARIES}

    gtest
)

target_link_libraries(
    engine_test

    ubuntu-location-service

    ${CMAKE_THREAD_LIBS_INIT}
    ${Boost_LIBRARIES}
    ${DBUS_LIBRARIES}
    
    gmock
    gtest
)

target_link_libraries(
    position_test

    ubuntu-location-service
    
    ${CMAKE_THREAD_LIBS_INIT}
    ${Boost_LIBRARIES}
    ${DBUS_LIBRARIES}
    
    gtest
)

target_link_libraries(
    provider_selection_policy_test

    ubuntu-location-service
    
    ${CMAKE_THREAD_LIBS_INIT}
    ${Boost_LIBRARIES}
    ${DBUS_LIBRARIES}
    
    gmock
    gtest
)

target_link_libraries(
    wgs84_test

    ubuntu-location-service

    ${CMAKE_THREAD_LIBS_INIT}
    ${Boost_LIBRARIES}
    ${DBUS_LIBRARIES}
    
    gtest
)

target_link_libraries(
    provider_test

    ubuntu-location-service

    ${CMAKE_THREAD_LIBS_INIT}
    ${Boost_LIBRARIES}
    ${DBUS_LIBRARIES}
    
    gmock
    gtest
)

target_link_libraries(
    provider_factory_test

    ubuntu-location-service

    ${CMAKE_THREAD_LIBS_INIT}
    ${Boost_LIBRARIES}
    ${DBUS_LIBRARIES}
    
    gmock
    gtest
)

target_link_libraries(
    acceptance_tests

    ubuntu-location-service

    ${CMAKE_THREAD_LIBS_INIT}
    ${Boost_LIBRARIES}
    ${DBUS_LIBRARIES}
    gtest
)

target_link_libraries(
    controller_test

    ubuntu-location-service

    ${CMAKE_THREAD_LIBS_INIT}
    ${Boost_LIBRARIES}
    ${DBUS_LIBRARIES}

    gmock
    gtest
)

if (LOCATION_SERVICE_ENABLE_GPS_PROVIDERS)
  add_executable(gps_provider_test gps_provider_test.cpp)
  target_link_libraries(
    gps_provider_test

    ubuntu-location-service

    ${CMAKE_THREAD_LIBS_INIT}
    ${Boost_LIBRARIES}
    ${DBUS_LIBRARIES}
    ubuntu_application_api
    gtest
    gtest_main
  )
  add_test(gps_provider_test gps_provider_test)
endif(LOCATION_SERVICE_ENABLE_GPS_PROVIDERS)

if (LOCATION_SERVICE_ENABLE_GEOCLUE_PROVIDERS)
  add_executable(geoclue_provider_test geoclue_provider_test.cpp)
  target_link_libraries(
    geoclue_provider_test
    
    ubuntu-location-service

    ${CMAKE_THREAD_LIBS_INIT}
    ${Boost_LIBRARIES}
    ${DBUS_LIBRARIES}
    gtest
    gtest_main
  )
  if (LOCATION_SERVICE_ENABLE_DBUS_TEST_RUNNER)
    add_test(geoclue_provider_test ${DBUS_TEST_RUNNER_EXECUTABLE} --task=${CMAKE_CURRENT_BINARY_DIR}/geoclue_provider_test)
  else (LOCATION_SERVICE_ENABLE_DBUS_TEST_RUNNER)
    add_test(geoclue_provider_test ${CMAKE_CURRENT_BINARY_DIR}/geoclue_provider_test)
  endif (LOCATION_SERVICE_ENABLE_DBUS_TEST_RUNNER)
endif (LOCATION_SERVICE_ENABLE_GEOCLUE_PROVIDERS)

if (LOCATION_SERVICE_ENABLE_DBUS_TEST_RUNNER)
  add_test(acceptance_tests ${DBUS_TEST_RUNNER_EXECUTABLE} --task=${CMAKE_CURRENT_BINARY_DIR}/acceptance_tests)
  add_test(daemon_and_cli_tests ${DBUS_TEST_RUNNER_EXECUTABLE} --task=${CMAKE_CURRENT_BINARY_DIR}/daemon_and_cli_tests)
else (LOCATION_SERVICE_ENABLE_DBUS_TEST_RUNNER)
  add_test(acceptance_tests ${CMAKE_CURRENT_BINARY_DIR}/acceptance_tests)
  add_test(daemon_and_cli_tests ${CMAKE_CURRENT_BINARY_DIR}/daemon_and_cli_tests)
endif (LOCATION_SERVICE_ENABLE_DBUS_TEST_RUNNER)

add_test(provider_test provider_test)
add_test(controller_test controller_test)
add_test(default_permission_manager_test default_permission_manager_test)
add_test(engine_test engine_test)
add_test(position_test position_test)
add_test(provider_factory_test provider_factory_test)
add_test(provider_selection_policy_test provider_selection_policy_test)
add_test(wgs84_test wgs84_test)

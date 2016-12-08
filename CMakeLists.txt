cmake_minimum_required(VERSION 2.8)

project(ubuntu-location-service)

if (DEFINED SNAPPY_UBUNTU_CORE)
  add_definitions(-DSNAPPY_UBUNTU_CORE)
endif()

# We haven't received version information via the packaging setup.
# For that, we try to determine sensible values on our own, ensuring
# plain old invocations to cmake still work as expected.
if (NOT DEFINED UBUNTU_LOCATION_SERVICE_VERSION_MAJOR)
  find_program(LSB_RELEASE lsb_release)
  execute_process(
    COMMAND ${LSB_RELEASE} -c -s
    OUTPUT_VARIABLE DISTRO_CODENAME
    OUTPUT_STRIP_TRAILING_WHITESPACE)

  # We explicitly ignore errors and only check if we are building for vivid.
  # For all other cases:
  #   - releases other than vivid
  #   - other distros
  #   - errors
  # we define the version to be 2.0.0
  if (${DISTRO_CODENAME} STREQUAL "vivid")
    set(UBUNTU_LOCATION_SERVICE_VERSION_MAJOR 2)
    set(UBUNTU_LOCATION_SERVICE_VERSION_MINOR 0)
    set(UBUNTU_LOCATION_SERVICE_VERSION_PATCH 0)
  else ()
    set(UBUNTU_LOCATION_SERVICE_VERSION_MAJOR 3)
    set(UBUNTU_LOCATION_SERVICE_VERSION_MINOR 0)
    set(UBUNTU_LOCATION_SERVICE_VERSION_PATCH 0)
  endif()
endif()

message(STATUS "${CMAKE_PROJECT_NAME} ${UBUNTU_LOCATION_SERVICE_VERSION_MAJOR}.${UBUNTU_LOCATION_SERVICE_VERSION_MINOR}.${UBUNTU_LOCATION_SERVICE_VERSION_PATCH}")

set(UBUNTU_LOCATION_SERVICE_TRUST_STORE_SERVICE_NAME "UbuntuLocationService")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -pedantic -Wextra -fPIC")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall -fno-strict-aliasing -fvisibility=hidden -fvisibility-inlines-hidden -pedantic -Wextra -fPIC -pthread")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,--no-undefined")

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

include(cmake/EnableCoverageReport.cmake)
include(cmake/FindGFlags.cmake)
include(cmake/FindGLog.cmake)
include(cmake/PrePush.cmake)
include(CTest)

include(GNUInstallDirs)

find_package(PkgConfig)
find_package(Boost COMPONENTS filesystem system program_options)
find_package(GLog)
find_package(GFlags)
find_package(Threads)
pkg_check_modules(DBUS dbus-1 REQUIRED)
pkg_check_modules(DBUS_CPP dbus-cpp REQUIRED)
pkg_check_modules(JSON_C json-c REQUIRED)
pkg_check_modules(LIBAPPARMOR libapparmor REQUIRED)
pkg_check_modules(NET_CPP net-cpp REQUIRED)
pkg_check_modules(PROCESS_CPP process-cpp REQUIRED)
pkg_check_modules(PROPERTIES_CPP properties-cpp REQUIRED)
pkg_check_modules(TRUST_STORE trust-store REQUIRED)
pkg_check_modules(UBUNTU_PLATFORM_HARDWARE_API ubuntu-platform-hardware-api)
#####################################################################
# Enable code coverage calculation with gcov/gcovr/lcov
# Usage:
#  * Switch build type to coverage (use ccmake or cmake-gui)
#  * Invoke make, make test, make coverage
#  * Find html report in subdir coveragereport
#  * Find xml report feasible for jenkins in coverage.xml
#####################################################################
IF(CMAKE_BUILD_TYPE MATCHES [cC][oO][vV][eE][rR][aA][gG][eE])
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ftest-coverage -fprofile-arcs" )
  SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -ftest-coverage -fprofile-arcs" )
  SET(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} -ftest-coverage -fprofile-arcs" )
  SET(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -ftest-coverage -fprofile-arcs" )
ENDIF(CMAKE_BUILD_TYPE MATCHES [cC][oO][vV][eE][rR][aA][gG][eE])

option (DISABLE_ERROR_ON_LOCAL_TYPEDEFS_WARNINGS "Disable errors when local typedefs are unused" ON)
if (DISABLE_ERROR_ON_LOCAL_TYPEDEFS_WARNINGS)
    SET (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-error=unused-local-typedefs")
endif (DISABLE_ERROR_ON_LOCAL_TYPEDEFS_WARNINGS)

#
# Code style fixer. We rely on clang-format to adjust the formatting of source code.
#
find_program(CLANG_FORMAT_COMMAND NAMES clang-format-3.6 clang-format-3.5)
if (NOT CLANG_FORMAT_COMMAND)
  message(WARNING "Cannot find clang-format >= clang-format-3.5: formatcode target will not be available")
endif()

if (CLANG_FORMAT_COMMAND)
  file(GLOB_RECURSE UBUNTU_LOCATION_SERVICE_INTERFACE_HEADER_FILES ${CMAKE_SOURCE_DIR}/include/*.h)
  file(GLOB_RECURSE UBUNTU_LOCATION_SERVICE_IMPLEMENTATION_FILES ${CMAKE_SOURCE_DIR}/src/*.h ${CMAKE_SOURCE_DIR}/src/*.cpp)
  add_custom_target(formatcode ${CLANG_FORMAT_COMMAND} -i ${UBUNTU_LOCATION_SERVICE_INTERFACE_HEADER_FILES} ${UBUNTU_LOCATION_SERVICE_IMPLEMENTATION_FILES} SOURCES _clang-format)
endif()

include_directories(
  3rd-party/
  3rd-party/ichnaea/include

  ${Boost_INCLUDE_DIRS}
  ${DBUS_INCLUDE_DIRS}
  ${DBUS_CPP_INCLUDE_DIRS}
  ${JSON_C_INCLUDE_DIRS}
  ${LIBAPPARMOR_INCLUDE_DIRS}
  ${NET_CPP_INCLUDE_DIRS}
  ${PROPERTIES_CPP_INCLUDE_DIRS}
  ${PROCESS_CPP_INCLUDE_DIRS}
  ${TRUST_STORE_INCLUDE_DIRS}
  ${GLog_INCLUDE_DIR}
  ${GFlags_INCLUDE_DIR}

  ${CMAKE_SOURCE_DIR}/include
  ${CMAKE_SOURCE_DIR}/src
  ${CMAKE_BINARY_DIR}/src
)

file(GLOB_RECURSE UBUNTU_LOCATION_SERVICE_PUBLIC_HEADERS ${CMAKE_CURRENT_SOURCE_DIR} *.h)

add_subdirectory(3rd-party/ichnaea)

add_subdirectory(data)
add_subdirectory(doc)
add_subdirectory(examples)
add_subdirectory(include)
add_subdirectory(po)
add_subdirectory(src)
add_subdirectory(tests)

enable_coverage_report(service)

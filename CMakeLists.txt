cmake_minimum_required(VERSION 2.8)

project(ubuntu-location-service)

set(UBUNTU_LOCATION_SERVICE_VERSION_MAJOR 0)
set(UBUNTU_LOCATION_SERVICE_VERSION_MINOR 0)
set(UBUNTU_LOCATION_SERVICE_VERSION_PATCH 1)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Werror -Wall -pedantic -Wextra -fPIC -pthread")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall -fno-strict-aliasing -pedantic -Wextra -fPIC -pthread")

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

include(cmake/EnableCoverageReport.cmake)
include(cmake/FindGFlags.cmake)
include(cmake/FindGLog.cmake)
include(cmake/PrePush.cmake)
include(CTest)

find_package(PkgConfig)
find_package(Boost 1.49 COMPONENTS system program_options)
find_package(GLog)
pkg_check_modules(DBUS dbus-1)
pkg_check_modules(DBUS_CPP dbus-cpp)

include_directories(
  ${Boost_INCLUDE_DIRS}
  ${DBUS_INCLUDE_DIRS}
  ${DBUS_CPP_INCLUDE_DIRS}/dbus
  ${GLog_INCLUDE_DIR}
  
  include/location_service
)

add_subdirectory(data)
add_subdirectory(doc)
add_subdirectory(examples)
add_subdirectory(include)
add_subdirectory(src)
add_subdirectory(tests)


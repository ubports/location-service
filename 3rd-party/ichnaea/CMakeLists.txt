cmake_minimum_required(VERSION 2.8)

project(ichnaea)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

include(CTest)

find_package(PkgConfig)
find_package(Boost)
find_package(Threads)

pkg_check_modules(NET_CPP net-cpp REQUIRED)
pkg_check_modules(UBUNTU_LOCATION_SERVICE_CONNECTIVITY ubuntu-location-service-connectivity REQUIRED)

include_directories(
  include
  src

  ${NET_CPP_INCLUDE_DIRS}
  ${UBUNTU_LOCATION_SERVICE_CONNECTIVITY_INCLUDE_DIRS})

add_library(
  ichnaea
  
  include/ichnaea/ichnaea.h

  include/ichnaea/bluetooth_beacon.h
  include/ichnaea/client.h
  include/ichnaea/error.h
  include/ichnaea/radio_cell.h
  include/ichnaea/response.h
  include/ichnaea/wifi_access_point.h
  include/ichnaea/geolocate/fallback.h
  include/ichnaea/geolocate/parameters.h
  include/ichnaea/geolocate/result.h
  include/ichnaea/geosubmit/parameters.h
  include/ichnaea/geosubmit/report.h
  include/ichnaea/region/parameters.h
  include/ichnaea/region/result.h
  
  src/ichnaea/geolocate/fallback.cpp
  src/ichnaea/geosubmit/report.cpp

  src/ichnaea/codec.h
  src/ichnaea/client.cpp
  src/ichnaea/error.cpp
  src/ichnaea/radio_cell.cpp
  src/ichnaea/wifi_access_point.cpp)

target_link_libraries(
  ichnaea

  ${CMAKE_THREAD_LIBS_INIT}
  ${NET_CPP_LDFLAGS}
  ${UBUNTU_LOCATION_SERVICE_CONNECTIVITY_LDFLAGS})

add_executable(
  ichnaea-client

  examples/client.cpp)

target_link_libraries(
  ichnaea-client

  ichnaea)

add_subdirectory(tests)

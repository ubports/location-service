find_package(PkgConfig)

find_package(Qt5Core REQUIRED)
find_package(Qt5Gui REQUIRED)
find_package(Qt5Location REQUIRED)

pkg_check_modules(UBUNTU_PLATFORM_API ubuntu-platform-api REQUIRED)

qt5_add_resources(POSITION_RESOURCES position.qrc)

add_library(
  qtposition_ubuntu SHARED

  core_geo_position_info_source.h
  core_geo_position_info_source_factory.h

  core_geo_position_info_source.cpp
  core_geo_position_info_source_factory.cpp

  ${POSITION_RESOURCES}
)

# Ideally, we would read the plugin installation location from cmake
# but this does not work currently.
set(PLUGIN_INSTALL_LOCATION "${CMAKE_INSTALL_LIBDIR}/qt5/plugins/position")
# get_target_property(PLUGIN_LOCATION Qt5::Location PLUGIN_LOCATION)

message(STATUS "Installing Qt5 location plugin to: ${PLUGIN_INSTALL_LOCATION}")

qt5_use_modules(qtposition_ubuntu Core Gui Location)
qt5_use_modules(qtposition_ubuntu Gui Location)

target_link_libraries(
  qtposition_ubuntu

  ${CMAKE_THREAD_LIBS_INIT}

  Qt5::Core
  Qt5::Gui
  Qt5::Location

  ${UBUNTU_PLATFORM_API_LIBRARIES}
)

install(
  TARGETS qtposition_ubuntu
  LIBRARY DESTINATION ${PLUGIN_INSTALL_LOCATION})

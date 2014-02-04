find_package(PkgConfig)
find_package(Boost COMPONENTS system)
find_package(Threads)

pkg_check_modules(DBUS dbus-1)
pkg_check_modules(DBUS_CPP dbus-cpp)
include_directories(
    ${Boost_INCLUDE_DIRS}
    ${DBUS_INCLUDE_DIRS}
    ${DBUS_CPP_INCLUDE_DIRS}/dbus

)

link_directories(
  /tmp/wpsapi-4.7.6_05-linux-x86_64/lib
)

add_executable(
    client
    client.cpp
)

add_executable(
    service
    service.cpp
)

target_link_libraries(
    client

    ubuntu-location-service

    ${CMAKE_THREAD_LIBS_INIT}
    ${Boost_LIBRARIES}
    ${DBUS_LIBRARIES}
    ${DBUS_CPP_LDFLAGS}
)

target_link_libraries(
    service

    ubuntu-location-service

    ${CMAKE_THREAD_LIBS_INIT}
    ${Boost_LIBRARIES}
    ${DBUS_LIBRARIES}
    ${DBUS_CPP_LDFLAGS}
)

add_custom_command(
    OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/ubuntu-location-service.1"
    COMMAND ronn --roff --pipe --organization=ubuntu ${CMAKE_CURRENT_SOURCE_DIR}/service.1.ronn > ${CMAKE_CURRENT_BINARY_DIR}/ubuntu-location-service.1
)

add_custom_target(
    ubuntu-location-service-man-page
    DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/ubuntu-location-service.1"
)

install(
    TARGETS service client
    DESTINATION ${CMAKE_INSTALL_LIBEXECDIR}/examples/
)

install(
    FILES ${CMAKE_CURRENT_SOURCE_DIR}/ubuntu-location-service.1
    DESTINATION share/doc/${CMAKE_PROJECT_NAME}/man
)

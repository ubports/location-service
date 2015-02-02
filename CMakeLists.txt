cmake_minimum_required(VERSION 2.8)

project(ubuntu-location-service)

set(UBUNTU_LOCATION_SERVICE_VERSION_MAJOR 2)
set(UBUNTU_LOCATION_SERVICE_VERSION_MINOR 0)
set(UBUNTU_LOCATION_SERVICE_VERSION_PATCH 0)

set(UBUNTU_LOCATION_SERVICE_TRUST_STORE_SERVICE_NAME "UbuntuLocationService")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -pedantic -Wextra -fPIC")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall -fno-strict-aliasing -pedantic -Wextra -fPIC")

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
# Code style fixer. We put the code through astyle first because it makes some fixes that
# clang-format won't do (such as "char *p" -> "char* p"). But astyle messes up other things
# (particularly lambdas and assembly-style comments), which clang-format does right.
# So, we run clang-format after running astyle, which undoes the damage done by astyle
# without reverting desirable astyle fixes.
#

find_program(ASTYLE_COMMAND NAMES astyle)
if (NOT ASTYLE_COMMAND)
    message(WARNING "Cannot find astyle: formatcode target will not be available")
else()
    # astyle 2.03 creates DOS line endings, so we need to fix its output
    find_program(DOS2UNIX_COMMAND NAMES dos2unix)
    if (NOT DOS2UNIX_COMMAND)
        message(WARNING "Cannot find dos2unix: formatcode target will not be available")
    else()
        find_program(CLANG_FORMAT_COMMAND NAMES clang-format-3.6 clang-format-3.5)
        if (NOT CLANG_FORMAT_COMMAND)
            message(WARNING "Cannot find clang-format >= clang-format-3.5: formatcode target will not be available")
        endif()
    endif()
endif()

if (ASTYLE_COMMAND AND DOS2UNIX_COMMAND AND CLANG_FORMAT_COMMAND)
set(UNITY_SCOPES_LIB_HDRS ${UNITY_SCOPES_LIB_HDRS} ${fmt_h})
    add_custom_target(formatcode
                      ${PROJECT_SOURCE_DIR}/tools/format-files.sh ${PROJECT_SOURCE_DIR} ${ASTYLE_COMMAND} ${CLANG_FORMAT_COMMAND})
endif()

include_directories(
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

  ${CMAKE_SOURCE_DIR}/include/location_service
  ${CMAKE_SOURCE_DIR}/src/location_service
  ${CMAKE_BINARY_DIR}/src/location_service
)

file(GLOB_RECURSE UBUNTU_LOCATION_SERVICE_PUBLIC_HEADERS ${CMAKE_CURRENT_SOURCE_DIR} *.h)

add_subdirectory(data)
add_subdirectory(doc)
add_subdirectory(examples)
add_subdirectory(include)
add_subdirectory(po)
add_subdirectory(src)
add_subdirectory(tests)

# There's no nice way to format this. Thanks CMake.
add_test(LGPL-required
  /bin/sh -c "! grep -rl 'GNU General' ${PROJECT_SOURCE_DIR}/src ${PROJECT_SOURCE_DIR}/include"
)

enable_coverage_report(service)

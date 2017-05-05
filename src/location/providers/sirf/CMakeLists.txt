file(GLOB_RECURSE SIRF_HEADERS*.h)

add_library(
  sirf

  ${SIRF_HEADERS}

  bits.h

  checksum.h
  checksum.cpp
  reader.h
  reader.cpp
  receiver.h
  receiver.cpp
  scanner.h
  scanner.cpp
  serial_port_receiver.h
  serial_port_receiver.cpp
  writer.h
  writer.cpp

  provider.h provider.cpp)

target_link_libraries(
  sirf

  nmea)

set(
  ENABLED_PROVIDER_TARGETS
  ${ENABLED_PROVIDER_TARGETS} sirf
  PARENT_SCOPE)

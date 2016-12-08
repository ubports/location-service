file(GLOB_RECURSE UBX_HEADERS*.h)

add_library(
  ubx

  ${UBX_HEADERS}

  _8/receiver.cpp
  _8/serial_port_receiver.cpp
  _8/nmea/scanner.cpp
  _8/nmea/sentence.cpp

  provider.h provider.cpp)

set(
  ENABLED_PROVIDER_TARGETS
  ${ENABLED_PROVIDER_TARGETS} ubx
  PARENT_SCOPE)

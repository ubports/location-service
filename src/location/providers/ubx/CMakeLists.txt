file(GLOB_RECURSE UBX_HEADERS*.h)

add_library(
  ubx

  ${UBX_HEADERS}

  bits.h

  _8/checksum.h
  _8/checksum.cpp
  _8/gnss_id.h
  _8/gnss_id.cpp
  _8/reader.h
  _8/reader.cpp
  _8/receiver.h
  _8/receiver.cpp
  _8/scanner.h
  _8/scanner.cpp
  _8/serial_port_receiver.h
  _8/serial_port_receiver.cpp
  _8/writer.h
  _8/writer.cpp
  _8/ack/ack.h
  _8/ack/ack.cpp
  _8/ack/nak.h
  _8/ack/nak.cpp
  _8/cfg/cfg.h
  _8/cfg/gnss.h
  _8/cfg/gnss.cpp
  _8/cfg/msg.h
  _8/cfg/msg.cpp
  _8/nav/pvt.h
  _8/nav/pvt.cpp
  _8/nav/sat.h
  _8/nav/sat.cpp
  _8/nmea/scanner.cpp
  _8/nmea/sentence.cpp

  provider.h provider.cpp)

set(
  ENABLED_PROVIDER_TARGETS
  ${ENABLED_PROVIDER_TARGETS} ubx
  PARENT_SCOPE)

add_library(
  mls
  provider.h
  provider.cpp)

target_link_libraries(
  mls

  ichnaea)

set(
  ENABLED_PROVIDER_TARGETS
  ${ENABLED_PROVIDER_TARGETS} mls
  PARENT_SCOPE)

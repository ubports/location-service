add_library(
  dummy
  
  provider.h
  provider.cpp
  delayed_provider.h
  delayed_provider.cpp)

set(
  ENABLED_PROVIDER_TARGETS
  ${ENABLED_PROVIDER_TARGETS} dummy
  PARENT_SCOPE)

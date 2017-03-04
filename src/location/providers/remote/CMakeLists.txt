file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/generated")

find_program(GDBUS_CODEGEN_EXECUTABLE NAMES gdbus-codegen)
execute_process(
    COMMAND ${GDBUS_CODEGEN_EXECUTABLE}
            --c-generate-object-manager
            --generate-c-code ${CMAKE_CURRENT_BINARY_DIR}/generated/provider
            ${CMAKE_CURRENT_SOURCE_DIR}/provider.xml)
	  
add_library(
  remote
  "${CMAKE_CURRENT_BINARY_DIR}/generated/provider.h"
  "${CMAKE_CURRENT_BINARY_DIR}/generated/provider.c"
  provider.h provider.cpp
  stub.cpp skeleton.cpp
)

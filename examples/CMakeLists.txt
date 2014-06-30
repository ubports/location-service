add_subdirectory(service)

install(
  DIRECTORY standalone
  DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/${CMAKE_PROJECT_NAME}/examples)
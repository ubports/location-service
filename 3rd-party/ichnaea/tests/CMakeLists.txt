add_executable(fallback_test fallback_test.cpp)
target_link_libraries(fallback_test ichnaea gtest gtest_main)
add_test(fallback_test fallback_test)

add_executable(error_test error_test.cpp)
target_link_libraries(error_test ichnaea gtest gtest_main)
add_test(error_test error_test)

add_executable(radio_cell_test radio_cell_test.cpp)
target_link_libraries(radio_cell_test ichnaea gtest gtest_main)
add_test(radio_cell_test radio_cell_test)

add_executable(response_test response_test.cpp)
target_link_libraries(response_test ichnaea gtest gtest_main)
add_test(response_test response_test)

add_executable(wifi_access_point_test wifi_access_point_test.cpp)
target_link_libraries(wifi_access_point_test ichnaea gtest gtest_main)
add_test(wifi_access_point_test wifi_access_point_test)

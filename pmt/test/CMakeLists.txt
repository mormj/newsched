

add_executable(test_pmt test_pmt.cpp)

target_include_directories(test_pmt
PUBLIC
$<INSTALL_INTERFACE:include>
$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>
)

target_link_libraries(test_pmt 
    pmt
)



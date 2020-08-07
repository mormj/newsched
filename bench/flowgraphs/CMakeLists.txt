add_executable(test_basic basic.cpp)

target_include_directories(test_basic
PUBLIC
$<INSTALL_INTERFACE:include>
$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>
PRIVATE
${PROJECT_SOURCE_DIR}/schedulers/simplestream/include
${PROJECT_SOURCE_DIR}/runtime/include 
)

target_link_libraries(test_basic 
    gnuradio-blocklib-common
    gnuradio-blocklib-blocks
    gnuradio-runtime
    gnuradio-scheduler-simplestream
)

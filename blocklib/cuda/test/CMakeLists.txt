########################################################
#   QA Tests
########################################################

list(APPEND TEST_SOURCES
qa_fft.cpp
)

add_executable(qa_blocklib_cuda ${TEST_SOURCES})
target_include_directories(qa_blocklib_cuda
    PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../schedulers/st/include>
    $<INSTALL_INTERFACE:include>
    PRIVATE
    ${CMAKE_SOURCE_DIR}/deps/catch/include
    ${CMAKE_SOURCE_DIR}/schedulers/st/include>
    
)
target_link_libraries(qa_blocklib_cuda  
    gnuradio-runtime
    gnuradio-blocklib-blocks
    gnuradio-blocklib-cuda
    gnuradio-scheduler-st)

add_test(NAME blocklib_cuda_tests
    COMMAND qa_blocklib_cuda)
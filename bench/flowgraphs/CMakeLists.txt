add_executable(bm_basic basic.cpp)
add_executable(bm_domain_adapt domain_adapt.cpp)
add_executable(bm_cuda_fft cuda_fft.cpp)

target_include_directories(bm_basic
PUBLIC
$<INSTALL_INTERFACE:include>
$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>
PRIVATE
${PROJECT_SOURCE_DIR}/schedulers/st/include
${PROJECT_SOURCE_DIR}/runtime/include 
)

target_include_directories(bm_domain_adapt
PUBLIC
$<INSTALL_INTERFACE:include>
$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>
PRIVATE
${PROJECT_SOURCE_DIR}/schedulers/st/include
${PROJECT_SOURCE_DIR}/runtime/include 
)

target_include_directories(bm_cuda_fft
PUBLIC
$<INSTALL_INTERFACE:include>
$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>
PRIVATE
${PROJECT_SOURCE_DIR}/schedulers/st/include
${PROJECT_SOURCE_DIR}/runtime/include 
)


target_link_libraries(bm_basic 
    gnuradio-runtime
    gnuradio-blocklib-blocks
    gnuradio-scheduler-st
    Boost::program_options
)

target_link_libraries(bm_domain_adapt 
    gnuradio-runtime
    gnuradio-blocklib-blocks
    gnuradio-scheduler-st
    Boost::program_options
)

target_link_libraries(bm_cuda_fft 
    gnuradio-runtime
    gnuradio-blocklib-blocks
    Boost::program_options
    gnuradio-scheduler-st
    gnuradio-blocklib-cuda-cu
    gnuradio-blocklib-cuda
)

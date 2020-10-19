add_executable(trt_demo trt_demo.cpp)
add_executable(fft_demo fft_demo.cpp)

# target_include_directories(
#     trt_demo
#     ${CUDA_INCLUDE_DIRS} 
#     ${TensorRT_INCLUDE_DIR}
# )

target_link_libraries(trt_demo 
    gnuradio-runtime
    gnuradio-blocklib-blocks
    gnuradio-scheduler-st
    gnuradio-blocklib-cuda-cu
    gnuradio-blocklib-cuda
)    

target_compile_options(trt_demo PRIVATE -Wno-deprecated-declarations)

target_link_libraries(fft_demo 
    gnuradio-runtime
    gnuradio-blocklib-blocks
    gnuradio-scheduler-st
    gnuradio-blocklib-cuda-cu
    gnuradio-blocklib-cuda
)    

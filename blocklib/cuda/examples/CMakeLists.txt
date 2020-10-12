add_executable(trt_demo trt_demo.cpp)

target_link_libraries(trt_demo 
    gnuradio-runtime
    gnuradio-blocklib-blocks
    gnuradio-scheduler-st
    gnuradio-blocklib-cuda-cu
    gnuradio-blocklib-cuda
)    

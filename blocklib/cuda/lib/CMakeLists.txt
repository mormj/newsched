find_package(CUDA REQUIRED)
find_package(TensorRT REQUIRED)

add_library(gnuradio-blocklib-cuda-cu SHARED
  fft.cu
  multiply_const.cu
  multiply_const_doublecopy.cu
  )

target_compile_options(gnuradio-blocklib-cuda-cu PRIVATE $<$<COMPILE_LANGUAGE:CUDA>:-gencode arch=compute_50,code=sm_50>)
set_target_properties(gnuradio-blocklib-cuda-cu PROPERTIES POSITION_INDEPENDENT_CODE ON)
set_target_properties(gnuradio-blocklib-cuda-cu PROPERTIES CUDA_SEPARABLE_COMPILATION ON)

add_library(gnuradio-blocklib-cuda SHARED
  fft.cpp
  logger.cpp
  infer.cpp
  multiply_const.cpp
  multiply_const_doublecopy.cpp
  )
target_compile_options(gnuradio-blocklib-cuda PRIVATE -Wno-deprecated-declarations)

target_include_directories(gnuradio-blocklib-cuda
  PUBLIC
  $<INSTALL_INTERFACE:include>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>
  ${CMAKE_CURRENT_SOURCE_DIR}/../include/gnuradio/blocklib/cuda
  ${CUDA_INCLUDE_DIRS} 
  ${TensorRT_INCLUDE_DIR}
  PRIVATE
  ${PROJECT_SOURCE_DIR}/blocklib/common/include
  ${PROJECT_SOURCE_DIR}/runtime/include
  )

  # TODO - learn how to include against runtime with INSTALL_INTERFACE

  target_link_libraries(gnuradio-blocklib-cuda PUBLIC
  gnuradio-runtime gnuradio-blocklib-cuda-cu 
  ${TensorRT_LIBRARY}
${CUDA_CUDA_LIBRARY}   # CUDA Driver API
${CUDA_CUDART_LIBRARY} # CUDA Runtime API
${CUDA_CUFFT_LIBRARIES}
${nvonnxparser_LIBRARY}
)


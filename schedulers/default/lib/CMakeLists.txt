add_library(gnuradio-scheduler-default SHARED
  inner.cpp   
  )
set_target_properties(gnuradio-runtime PROPERTIES
            CXX_STANDARD 17
            )

message(STATUS ${PROJECT_SOURCE_DIR})

target_include_directories(gnuradio-scheduler-default
  PUBLIC
  $<INSTALL_INTERFACE:include>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>
  PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/../include/gnuradio/schedulers/default
  ${CMAKE_CURRENT_SOURCE_DIR}/../../../runtime/include 
  )

  # TODO - learn how to include against runtime with INSTALL_INTERFACE

  target_link_libraries(gnuradio-scheduler-default PUBLIC
    spdlog::spdlog
    gnuradio-runtime
    Boost::boost
    pthread)

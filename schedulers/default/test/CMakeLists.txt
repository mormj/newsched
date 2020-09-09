

add_executable(test_inner test_inner.cpp)

target_include_directories(gnuradio-scheduler-default
PUBLIC
$<INSTALL_INTERFACE:include>
$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>
PRIVATE
${PROJECT_SOURCE_DIR}/schedulers/default/include
${PROJECT_SOURCE_DIR}/runtime/include 
)

target_link_libraries(test_inner 
    gnuradio-runtime
    gnuradio-blocklib-blocks
    gnuradio-scheduler-default
)



########################################################
#   QA Tests
########################################################

# list(APPEND TEST_SOURCES
# qa_scheduler_default.cpp
# qa_tags.cpp
# )

# add_executable(qa_default ${TEST_SOURCES})
# target_include_directories(qa_default
#     PUBLIC
#     $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>
#     $<INSTALL_INTERFACE:include>
#     PRIVATE
#     ${CMAKE_SOURCE_DIR}/deps/catch/include
    
# )
# target_link_libraries(qa_default  
#     gnuradio-runtime
#     gnuradio-blocklib-blocks
#     gnuradio-scheduler-default)

# add_test(NAME default_tests
#     COMMAND qa_default)

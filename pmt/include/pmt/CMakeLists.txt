
message(STATUS "%@#$%@#$%@#$%@#$%@#$%")
message(STATUS ${CMAKE_CURRENT_BINARY_DIR}/..)
include_directories(${CMAKE_CURRENT_BINARY_DIR})


install(FILES
    pmt.hpp
    DESTINATION ${GR_INCLUDE_DIR}/pmt
)

set(FBS_FILE ${CMAKE_CURRENT_SOURCE_DIR}/pmt.fbs)

get_filename_component(generated_output_name ${FBS_FILE} NAME_WE)
add_custom_command(OUTPUT ${OUTPUT_LOCATION}/${generated_output_name}_generated.h
    # COMMAND flatc -c --reflect-types --reflect-names --gen-object-api -o "${OUTPUT_LOCATION}/" "${FBS_FILE}"
    COMMAND flatc -c "${FBS_FILE}"
    COMMENT "Building C++ header for flatbuffers definition ${FBS_FILE}"
    WORKING_DIRECTORY .
    DEPENDS "${FBS_FILE}"
)

add_custom_target(pmt-flatc
        DEPENDS ${OUTPUT_LOCATION}/${generated_output_name}_generated.h
        )


include(BuildFlatBuffers)


list(APPEND PMT_SCHEMA_FILES pmt.fbs
    )
build_flatbuffers (${PMT_SCHEMA_FILES}
                           ""
                           pmt_flatc
                           ""
                           ${CMAKE_BINARY_DIR}/pmt/include/pmt/generated
                           ""
                           "")


# flatbuffers_generate_headers (TARGET pmt-flatc
#                               SCHEMAS ${PMT_SCHEMA_FILES}
#                               )
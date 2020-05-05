add_executable("drace.detector.tracebinary.decoder" "BinaryDecoder")

target_link_libraries("drace.detector.tracebinary.decoder" "drace-common" "spdlog" "clipp")

set_target_properties ("drace.detector.tracebinary.decoder" PROPERTIES
                        CXX_STANDARD 17
                        CXX_STANDARD_REQUIRED ON)

if(UNIX)
    target_link_libraries("drace.detector.tracebinary.decoder" "-ldl")
endif()

install(TARGETS "drace.detector.tracebinary.decoder" DESTINATION ${DRACE_RUNTIME_DEST})

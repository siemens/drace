set(SOURCES
    "src/globals"
    "src/drace-client"
    "src/ShadowThreadState"
    "src/function-wrapper.cpp"
    "src/function-wrapper/internal"
    "src/function-wrapper/event"
    "src/memory-tracker"
    "src/instr/instr-mem-fast"
    "src/instr/instr-mem-full"
    "src/module/Metadata"
    "src/module/Tracker"
    "src/symbol/Symbols"
    "src/symbol/SymbolLocation"
    "src/util.cpp"
    "src/InstrumentationConfig"
    "src/RuntimeConfig"
    "src/race-filter"
    "src/race-collector")
if(WIN32)
    list(APPEND SOURCES "src/MSR")
endif()

find_package(DynamoRIO 7.91 REQUIRED)

# Bind detectors prior to first "configure_DynamoRIO_client"
# as DR changes global vars
add_subdirectory("detectors")

if(BUILD_TESTING)
    add_subdirectory(test)
endif()

add_library("drace-client" SHARED ${SOURCES})
target_include_directories("drace-client" PRIVATE "include")

# Setup as dynamorio client (not-standalone)
# See https://github.com/DynamoRIO/dynamorio/blob/master/make/DynamoRIOConfig.cmake.in
# for details
configure_DynamoRIO_client("drace-client")
use_DynamoRIO_extension("drace-client" drmgr)
use_DynamoRIO_extension("drace-client" drutil)
use_DynamoRIO_extension("drace-client" drreg)
use_DynamoRIO_extension("drace-client" drwrap)
use_DynamoRIO_extension("drace-client" drsyms)

target_link_libraries(
	"drace-client"   # main drace-libs
	"drace-common"   # drace and msr common libs
	"jtilly-inih"    # ini parser
	"hh-date"        # C++ date lib
  "clipp"          # option parser
  "$<$<CXX_COMPILER_ID:Clang>:atomic>"
)

# Set loglevel
target_compile_definitions("drace-client" PRIVATE "LOGLEVEL=${DRACE_LOGLEVEL}")
# we need a specific base addr, hence disable this warning
if(WIN32)
    target_link_libraries("drace-client" "drace.detector.tsan" -ignore:4281)
    target_compile_options("drace-client" PRIVATE -EHsc)
endif()

# TinyXML2
if(DRACE_XML_EXPORTER)
	target_link_libraries("drace-client" "tinyxml2")
	target_compile_definitions("drace-client" PRIVATE "DRACE_XML_EXPORTER")
endif()

if(DRACE_ENABLE_CPPCHECK)
    set_target_properties("drace-client" PROPERTIES
        CXX_CPPCHECK ${DRACE_CPPCHECK_CALL})
endif()

# Add licensing information on LCM
file(READ "include/lcm/LICENSE" LIC_FILE_LCM)
file(APPEND ${LIC_FILE} "${LIC_SEP}fmoessbauer/LossyCountingModel\n\n${LIC_FILE_LCM}")

# copy configs to build tree
add_custom_target("drace-copy-config" ALL
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${PROJECT_SOURCE_DIR}/drace.ini"
    "${PROJECT_SOURCE_DIR}/race_suppressions.txt"
    "$<TARGET_FILE_DIR:drace-client>")
# copy configs to install location
install(FILES
    "${PROJECT_SOURCE_DIR}/drace.ini"
    "${PROJECT_SOURCE_DIR}/race_suppressions.txt"
    DESTINATION "${DRACE_RUNTIME_DEST}")

install(TARGETS "drace-client"
    RUNTIME DESTINATION ${DRACE_RUNTIME_DEST} COMPONENT RUNTIME
    LIBRARY DESTINATION ${DRACE_ARCHIVE_DEST} COMPONENT ARCHIVE)

# DRace, a dynamic data race detector
#
# Copyright (c) Siemens AG, 2019
#
# Authors:
#   Felix Moessbauer <felix.moessbauer@siemens.com>
#
# This work is licensed under the terms of the MIT license.  See
# the LICENSE file in the top-level directory.

set(FT_SOURCES
    "src/stacktrace"
    "src/varstate"
    "src/threadstate")

set(FT_TEST_SOURCES
    "test/fasttrack_test"
    "src/stacktrace"
    "src/varstate"
    "src/threadstate")

include(GenerateExportHeader)
# we just need BGL which is header only, hence avoid errors by not importing as a target
find_package(Boost 1.65.0)

if(Boost_FOUND)
    message(STATUS "Build detector fasttrack (standalone)")

    add_library("drace.detector.fasttrack.generic" STATIC ${FT_SOURCES})
    set_target_properties(
        "drace.detector.fasttrack.generic" PROPERTIES
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED OFF
        POSITION_INDEPENDENT_CODE ON)

    if(WIN32)
        target_compile_options("drace.detector.fasttrack.generic" PUBLIC "/MT$<$<CONFIG:Debug>:d>")
    endif()

    target_include_directories("drace.detector.fasttrack.generic"
        PUBLIC "include"
        PUBLIC "${Boost_INCLUDE_DIR}")

    target_link_libraries("drace.detector.fasttrack.generic" "drace-common" "parallel-hashmap")

    ###########standalone Version#####################
    add_library("drace.detector.fasttrack.standalone" SHARED "src/fasttrack_st.cpp")

    generate_export_header("drace.detector.fasttrack.standalone" BASE_NAME fasttrack_st)

    # include exports header
    target_include_directories("drace.detector.fasttrack.standalone" PUBLIC ${CMAKE_CURRENT_BINARY_DIR})
    target_link_libraries("drace.detector.fasttrack.standalone" PRIVATE "drace.detector.fasttrack.generic" Threads::Threads)
    set_target_properties(
        "drace.detector.fasttrack.standalone" PROPERTIES
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED OFF
        CXX_VISIBILITY_PRESET hidden
        VISIBILITY_INLINES_HIDDEN ON)

    if(POLICY CMP0091)
        set_target_properties(
            "drace.detector.fasttrack.standalone" PROPERTIES
            # use static runtime (required by dynamorio)
            MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
    elseif(WIN32)
        target_compile_options("drace.detector.fasttrack.standalone" PRIVATE "/MT$<$<CONFIG:Debug>:d>")
    endif()

    install(TARGETS "drace.detector.fasttrack.standalone"
        RUNTIME DESTINATION ${DRACE_RUNTIME_DEST}/fasttrack COMPONENT Runtime
        LIBRARY DESTINATION ${DRACE_RUNTIME_DEST}/fasttrack COMPONENT Runtime
        ARCHIVE DESTINATION ${DRACE_ARCHIVE_DEST}/fasttrack COMPONENT Development)
    # TODO add target for drace-detector with INSTALL_INTERFACE
    install(FILES ${PROJECT_SOURCE_DIR}/common/detector/Detector.h DESTINATION ${DRACE_INCLUDE_DEST}/fasttrack)

    ################ configure test module ################
    if(BUILD_TESTING)
        # create library which is linked in global testing module
        add_executable(fasttrack_test ${FT_TEST_SOURCES})
        target_link_libraries(fasttrack_test PRIVATE gtest gtest_main "drace-common" "parallel-hashmap")
        target_include_directories(fasttrack_test
        PUBLIC "include"
        PUBLIC "${Boost_INCLUDE_DIR}")
        set_target_properties(
            fasttrack_test PROPERTIES
            CXX_STANDARD 17
            CXX_STANDARD_REQUIRED ON)


        if(COVERAGE_TESTING)
          add_coverage_flags(fasttrack_test)
        endif()

        gtest_discover_tests(fasttrack_test WORKING_DIRECTORY ${PROJECT_BINARY_DIR})
    endif()
endif()

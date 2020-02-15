 # DRace, a dynamic data race detector
 #
 # Copyright 2018 Siemens AG
 #
 # Authors:
 #   Philip Harr <philip.harr@siemens.com>
 #
 # SPDX-License-Identifier: MIT

add_executable(drace_rt_test
    "race-filter-test.cpp" "../src/race-filter.cpp"
    "shadow-stack-test.cpp")
target_compile_definitions(drace_rt_test PRIVATE TESTING)
target_link_libraries(drace_rt_test gmock gtest gtest_main "drace-common")
target_include_directories(drace_rt_test PRIVATE "../include")

if(COVERAGE_TESTING)
  add_coverage_flags(drace_rt_test)
endif()

add_custom_target("drace-copy-testdata" ALL
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${PROJECT_SOURCE_DIR}/drace.ini"
    "${PROJECT_SOURCE_DIR}/race_suppressions.txt"
    "$<TARGET_FILE_DIR:drace_rt_test>")

gtest_discover_tests(drace_rt_test WORKING_DIRECTORY ${PROJECT_BINARY_DIR})

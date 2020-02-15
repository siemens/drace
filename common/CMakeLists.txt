# burn-in git commit hash
set(PRE_CONFIGURE_FILE "${CMAKE_CURRENT_SOURCE_DIR}/version/version.h.in")
set(POST_CONFIGURE_FILE "${CMAKE_CURRENT_SOURCE_DIR}/version/version.h")
include(${PROJECT_SOURCE_DIR}/CMakeExt/git_watcher.cmake)

# check os + hw support
try_compile(HAVE_SSE2 "${CMAKE_BINARY_DIR}/temp" "${CMAKE_CURRENT_SOURCE_DIR}/test/have_mm_pause.c")
if(NOT HAVE_SSE2)
    message(WARNING "Compiler does not support SSE2")
endif()

# Common header-only target
add_library("drace-common" INTERFACE)
target_include_directories("drace-common" INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>)
target_compile_definitions("drace-common" INTERFACE "DETECTOR_EXPORTS")
if(HAVE_SSE2)
    target_compile_definitions("drace-common" INTERFACE "HAVE_SSE2")
endif()
add_dependencies("drace-common" check_git_repository)

################ configure test module ################
if(BUILD_TESTING)
    set(TEST_SOURCES "test/spinlock")
    if(WIN32)
        list(APPEND TEST_SOURCES "test/ShmDriver")
    endif()

    # create library which is linked in global testing module
    add_executable(common_test ${TEST_SOURCES})
    target_link_libraries(common_test PRIVATE gtest gtest_main "drace-common")
    if(COVERAGE_TESTING)
      add_coverage_flags(common_test)
    endif()
    # SHMDriver requires serial execution
    gtest_discover_tests(common_test WORKING_DIRECTORY ${PROJECT_BINARY_DIR})
endif()

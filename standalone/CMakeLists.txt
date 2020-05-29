# DRace, a dynamic data race detector
#
# Copyright (c) Siemens AG, 2019
#
# Authors:
#   Felix Moessbauer <felix.moessbauer@siemens.com>
#
# This work is licensed under the terms of the MIT license.  See
# the LICENSE file in the top-level directory.

# NOTE: we build all non-Dynamorio libs and tools here, as DR changes global flags

if(OPTIMIZE_FOR_NATIVE)
    # for standalone builds, try to use widest possible vector features
    include(CheckCXXCompilerFlag)
    CHECK_CXX_COMPILER_FLAG("-march=native" COMPILER_SUPPORTS_MARCH_NATIVE)
    if(COMPILER_SUPPORTS_MARCH_NATIVE)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=native")
    endif()
endif()

add_subdirectory("detectors/fasttrack")
add_subdirectory("binarydecoder")
add_subdirectory("helper")

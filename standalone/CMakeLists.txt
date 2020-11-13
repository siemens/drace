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

add_subdirectory("binarydecoder")
add_subdirectory("detectors/fasttrack")

set(Boost_USE_STATIC_LIBS OFF)
set(Boost_USE_MULTITHREADED ON)
find_package(Python 3 COMPONENTS Interpreter Development)
if(UNIX)
  find_package(Boost 1.65.0 COMPONENTS python${Python_VERSION_MAJOR})
else()
  find_package(Boost 1.65.0 COMPONENTS python${Python_VERSION_MAJOR}${Python_VERSION_MINOR})
endif()

if(Python_FOUND AND Boost_FOUND)
  message(STATUS "enable detector python bindings")
  add_subdirectory("dracepy")
endif()

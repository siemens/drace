# DRace, a dynamic data race detector
#
# Copyright (c) Siemens AG, 2018
#
# Authors:
#   Felix Moessbauer <felix.moessbauer@siemens.com>
#
# This work is licensed under the terms of the MIT license.  See
# the LICENSE file in the top-level directory.

cmake_minimum_required (VERSION 3.8)

if(POLICY CMP0091)
	# MSVC runtime library selected by abstraction
	cmake_policy(SET CMP0091 NEW)
endif()

project ("drace-client" LANGUAGES "C" "CXX")

# prepare default values
if(WIN32)
	set(DEFAULT_DRACE_ENABLE_RUNTIME ON)
	set(DEFAULT_DRACE_CPPCHECK_BIN "C:/Program Files/Cppcheck/cppcheck.exe")
else()
	set(DEFAULT_DRACE_ENABLE_RUNTIME OFF)
	set(DEFAULT_DRACE_CPPCHECK_BIN "/usr/bin/cppcheck")
endif()

set(BUILD_TESTING ON CACHE BOOL "Build Tests")
set(DRACE_ENABLE_BENCH OFF CACHE BOOL "Build Benchmarks")
set(DRACE_ENABLE_CPPCHECK OFF CACHE BOOL "Enable CPPCheck")
set(BUILD_CSHARP_EXAMPLES OFF CACHE BOOL "Build C# Examples")
set(BUILD_TOOLS ON CACHE BOOL "Build Tooling Support (e.g. Qt5 GUI)")
set(DRACE_ENABLE_RUNTIME ${DEFAULT_DRACE_ENABLE_RUNTIME} CACHE BOOL "Build Drace Runtime")
set(DRACE_CPPCHECK_BIN ${DEFAULT_DRACE_CPPCHECK_BIN} CACHE STRING "absolute path to cppcheck binary")
set(DRACE_XML_EXPORTER ON CACHE BOOL "Build with Valkyrie compatible XML exporter")
set(DRACE_LOGLEVEL "2" CACHE STRING "Set loglevel of DRace (0: error, 1: warning, ... 4: trace)")
set(OPTIMIZE_FOR_NATIVE OFF CACHE BOOL "Build standalone components with -march=native")

set(BOOST_LIBRARYDIR ${BOOST_ROOT}/libs)

# install targets
set(DRACE_INSTALL_TESTS ON CACHE BOOL "Install test target")
set(DRACE_INSTALL_BENCH OFF CACHE BOOL "Install benchmarks")
set(DRACE_INSTALL_DOCS  ON  CACHE BOOL "Install documentation")

set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(DRACE_RUNTIME_DEST "bin"     CACHE STRING "Runtime install destination")
set(DRACE_LIBRARY_DEST "lib"     CACHE STRING "Library install destination")
set(DRACE_ARCHIVE_DEST "lib"     CACHE STRING "Archive install destination")
set(DRACE_INCLUDE_DEST "include" CACHE STRING "Header install destination")
set(DRACE_DOC_DEST     "doc"     CACHE STRING "Documentation install destination")

# must be set to fixes issue in DR configure scripts on unix
# also useful on windows as dlls are placed next to binaries
if(NOT CMAKE_ARCHIVE_OUTPUT_DIRECTORY)
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/lib")
endif()
if(NOT CMAKE_LIBRARY_OUTPUT_DIRECTORY)
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/lib")
endif()
if(NOT CMAKE_RUNTIME_OUTPUT_DIRECTORY)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/bin")
endif()

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/CMakeExt/modules")

include(GenerateExportHeader)

if(DRACE_ENABLE_CPPCHECK)
    if(CMAKE_VERSION VERSION_GREATER 3.10.0)
        if(EXISTS "${DRACE_CPPCHECK_BIN}")
            message(STATUS "Enable support for cppcheck")
            set(DRACE_CPPCHECK_CALL "${DRACE_CPPCHECK_BIN}\;--suppressions-list=${PROJECT_SOURCE_DIR}/contrib/suppressions.txt")
        else()
            message(STATUS "No CPPCheck in ${DRACE_CPPCHECK_BIN}")
        endif()
    else()
        message(STATUS "Requested CPPCheck, but this requires at least CMake 3.10.0")
    endif()
endif()
configure_file(contrib/suppressions.txt suppressions.txt)

# DR performance tuning
set(DynamoRIO_FAST_IR ON CACHE BOOL "DynamoRIO_FAST_IR")

# CSharp can only be build using MSVC generators
message(STATUS "Generator: ${CMAKE_GENERATOR}")
if(${CMAKE_GENERATOR} MATCHES "Visual Studio")
	message(STATUS "Build CSharp Examples")
	set(BUILD_CSHARP_EXAMPLES ON)
endif()

find_package(Threads REQUIRED)

# prepare license
# whenever a external library is added, append its license to this file
set(LIC_FILE "${CMAKE_CURRENT_BINARY_DIR}/LICENSE.txt")
set(LIC_SEP "\n\n===========================================================================\n")

# generate and install license of DRace itself
file(WRITE ${LIC_FILE} "Primary DRace License: MIT\n\n")
file(READ "LICENSE" LIC_FILE_DRACE)
file(APPEND ${LIC_FILE} ${LIC_FILE_DRACE})

# setup global defines
if(WIN32)
	# avoid name collisions with std::min / std::max
	add_definitions(-DNOMINMAX)
endif()
if(CMAKE_SIZEOF_VOID_P EQUAL 4)
    message(STATUS "Building for x86 Plattform")
    add_definitions(-DCOMPILE_X86)
endif()

if(BUILD_TESTING)
	message(STATUS "Build Testsuite")
	include(CTest)
	include(GoogleTest)
endif()

# Common Files
add_subdirectory("common")
# external vendor targets
add_subdirectory("vendor")
# standalone tools
add_subdirectory("standalone")
# Tutorial
add_subdirectory("HowTo")

if(DRACE_ENABLE_RUNTIME)
	# DRACE
	add_subdirectory("drace-client")
	if(WIN32)
		# Managed Stack Resolver
		add_subdirectory("ManagedResolver")
	endif()
endif()

if(BUILD_TESTING)
message(STATUS "Build Testing")
	add_subdirectory("test")
endif()

if(DRACE_ENABLE_RUNTIME)
	if(DRACE_ENABLE_BENCH)
		message(STATUS "Build Benchmarks")
		add_subdirectory("bench")
	endif()

	if(BUILD_TOOLS)
		message(STATUS "Build Tools")
		add_subdirectory("tools")
	endif()
endif()

# Build documentation
find_package(Doxygen)
if(Doxygen_FOUND)
	message(STATUS "Build Documentation")
	set(DOXYGEN_GENERATE_HTML YES)
	set(DOXYGEN_GENERATE_MAN NO)
	set(DOXYGEN_OUTPUT_DIRECTORY doc)
	set(DOXYGEN_EXCLUDE_PATTERNS
		"*DynamoRIO*"
		"build"
		"install"
		"${PROJECT_BINARY_DIR}"
		"${CMAKE_INSTALL_PREFIX}"
		"*/test/*"
		"${PROJECT_SOURCE_DIR}/ManagedResolver/include/prebuild/*"
		"${PROJECT_SOURCE_DIR}/test/*"
		"${PROJECT_SOURCE_DIR}/bench/*"
		"${PROJECT_SOURCE_DIR}/vendor/*"
		"${PROJECT_SOURCE_DIR}/drace-client/detectors/extsan"
		"${PROJECT_SOURCE_DIR}/tools/ReportConverter/ReportConverterUnitTest.py"
		"${PROJECT_SOURCE_DIR}/node_modules/*")
	set(DOXYGEN_USE_MDFILE_AS_MAINPAGE "${PROJECT_SOURCE_DIR}/README.md")

	# work around https://gitlab.kitware.com/cmake/cmake/issues/18738
	set(DOXYGEN_LATEX_MAKEINDEX_CMD makeindex)

	doxygen_add_docs(
		doc
		COMMENT "Generate Code Documentation")

	if(DRACE_INSTALL_DOCS)
		install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/docs/html DESTINATION ${DRACE_DOC_DEST} OPTIONAL)
	endif()
endif()

# install config readme
install(FILES "README.md" DESTINATION ${DRACE_DOC_DEST})
install(FILES ${LIC_FILE} DESTINATION ${DRACE_DOC_DEST})

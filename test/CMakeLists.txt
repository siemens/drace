# DRace, a dynamic data race detector
#
# Copyright (c) Siemens AG, 2018
#
# Authors:
#   Felix Moessbauer <felix.moessbauer@siemens.com>
#
# This work is licensed under the terms of the MIT license.  See
# the LICENSE file in the top-level directory.

set(TEST_SOURCES_UNIT
	"src/DetectorTest.cpp")

set(TEST_UNIT_TARGET "drace-unit-tests")
set(TEST_SYSTEM_TARGET "drace-system-tests")

set(Boost_USE_STATIC_LIBS ON)
set(Boost_USE_MULTITHREADED ON)

##### SETUP DRace SYSTEM TESTS #####

# get path to drrun and race and hard-code into test binary
# for easier integration in CI
if(DRACE_ENABLE_RUNTIME)
	find_package(Boost 1.65.0 REQUIRED COMPONENTS filesystem)

	get_filename_component(DR_NORMALIZED_PATH "${DynamoRIO_DIR}/../bin64/drrun${CMAKE_EXECUTABLE_SUFFIX}" ABSOLUTE)
	file(TO_NATIVE_PATH ${DR_NORMALIZED_PATH} PATH_TO_DRRUN)
	message(STATUS "Test (integration): Use DynamoRIO in ${PATH_TO_DRRUN}")

	# we cannot use generator expressions here, hence hardcode
	if(WIN32)
		set(DRACE_REL_PATH "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/drace-client.dll")
	else()
		set(DRACE_REL_PATH "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/libdrace-client.so")
	endif()
	get_filename_component(DRACE_NORMALIZED_PATH ${DRACE_REL_PATH} ABSOLUTE)
	file(TO_NATIVE_PATH ${DRACE_NORMALIZED_PATH} PATH_TO_DRACE)
	message(STATUS "Test (integration): Use DRace in     ${PATH_TO_DRACE}")

	configure_file(src/IntegrationTestEnv.cpp.cmake generated/IntegrationTestEnv.cpp @ONLY)

	add_executable(${TEST_SYSTEM_TARGET}
		"src/main.cpp"
		"src/IntegrationTest.cpp"
		${CMAKE_CURRENT_BINARY_DIR}/generated/IntegrationTestEnv.cpp)
	target_link_libraries(${TEST_SYSTEM_TARGET} PRIVATE
		gtest
		Boost::filesystem)

	target_include_directories(${TEST_SYSTEM_TARGET} PRIVATE "include")

	if(DRACE_XML_EXPORTER)
		target_compile_definitions(${TEST_SYSTEM_TARGET} PRIVATE DRACE_XML_EXPORTER)
		target_link_libraries(${TEST_SYSTEM_TARGET} PRIVATE "tinyxml2")
	endif()

	if(WIN32)
		target_compile_definitions(${TEST_SYSTEM_TARGET} PRIVATE WINDOWS)
		# manually build C# tests as CMake support is not available in ninja
		find_program(CSC csc.exe DOC "Path to CSharp compiler csc.exe")
		if(CSC)
			target_compile_definitions(${TEST_SYSTEM_TARGET} PRIVATE DRACE_TESTING_DOTNET)
		else()
		message(WARNING "csc not found: .NET Windows tests will be disabled")
		endif()
	endif()
endif()

##### SETUP DRace UNIT TESTS #####
add_executable(${TEST_UNIT_TARGET} ${TEST_SOURCES_UNIT})

target_include_directories(${TEST_UNIT_TARGET} PRIVATE "include")
target_link_libraries(${TEST_UNIT_TARGET} PRIVATE gtest gtest_main "drace-common")
if(UNIX)
  target_link_libraries(${TEST_UNIT_TARGET} PRIVATE "-ldl")
  set_property(TARGET ${TEST_UNIT_TARGET}
	  APPEND PROPERTY BUILD_RPATH "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}")
endif()

# enable dr mocks
target_compile_definitions(${TEST_UNIT_TARGET} PRIVATE TESTING)

if(DRACE_ENABLE_RUNTIME)
	add_dependencies(${TEST_UNIT_TARGET} "drace-client")
endif()

if(COVERAGE_TESTING)
  add_coverage_flags(${TEST_UNIT_TARGET})
endif()

gtest_discover_tests(${TEST_UNIT_TARGET}
	WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
	PROPERTIES RUN_SERIAL ON)

if(TARGET ${TEST_UNIT_TARGET})
	add_custom_command(TARGET ${TEST_UNIT_TARGET} POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy_directory
			"${PROJECT_SOURCE_DIR}/test/data/"
			"$<TARGET_FILE_DIR:${TEST_UNIT_TARGET}>/data/")
endif()

# Prepare Guinea pigs
add_subdirectory("mini-apps")

if(DRACE_INSTALL_TESTS AND DRACE_ENABLE_RUNTIME)
	install(TARGETS ${TEST_UNIT_TARGET} RUNTIME DESTINATION ${DRACE_RUNTIME_DEST} COMPONENT RUNTIME)
	install(TARGETS ${TEST_SYSTEM_TARGET} RUNTIME DESTINATION ${DRACE_RUNTIME_DEST} COMPONENT RUNTIME)
    install(FILES ${PROJECT_SOURCE_DIR}/test/data/drace_excl.ini DESTINATION ${DRACE_RUNTIME_DEST})
endif()


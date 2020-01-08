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

find_package(Boost 1.65.0 REQUIRED COMPONENTS
	filesystem)

##### SETUP DRace SYSTEM TESTS #####

# get path to drrun and race and hard-code into test binary
# for easier integration in CI
if(DRACE_ENABLE_RUNTIME)
	get_filename_component(DR_NORMALIZED_PATH "${DynamoRIO_DIR}/../bin64/drrun.exe" ABSOLUTE)
	file(TO_NATIVE_PATH ${DR_NORMALIZED_PATH} PATH_TO_DRRUN)
	message(STATUS "Test (integration): Use DynamoRIO in ${PATH_TO_DRRUN}")

	get_filename_component(DRACE_NORMALIZED_PATH "${CMAKE_CURRENT_BINARY_DIR}/../drace-client/drace-client.dll" ABSOLUTE)
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
		target_compile_definitions(${TEST_SYSTEM_TARGET} PRIVATE -DDRACE_XML_EXPORTER)
		target_link_libraries(${TEST_SYSTEM_TARGET} PRIVATE "tinyxml2")
	endif()
	# This introduces a circular dependency as shared libraries
	# are copied after the target is build
	#gtest_discover_tests(${TEST_TARGET})
	
	# manually build C# tests as CMake support is not available in ninja
	find_program(CSC csc.exe DOC "Path to CSharp compiler csc.exe")
	if(CSC)
	target_compile_definitions(${TEST_SYSTEM_TARGET} PRIVATE -DDRACE_TESTING_DOTNET)
	else()
	message(STATUS "csc not found: .NET Windows tests will be disabled")
	endif()
endif()
	

##### SETUP DRace UNIT TESTS #####
add_executable(${TEST_UNIT_TARGET} ${TEST_SOURCES_UNIT})

target_include_directories(${TEST_UNIT_TARGET} PRIVATE "include")
target_link_libraries(${TEST_UNIT_TARGET} PRIVATE gtest gtest_main "drace-common")
if(UNIX)
	target_link_libraries(${TEST_UNIT_TARGET} PRIVATE "-ldl")
endif()

# get and register all test suites
get_property(TEST_SUITES GLOBAL PROPERTY DRACE_TEST_SUITES)
if(TEST_SUITES)
	target_link_libraries(${TEST_UNIT_TARGET} PRIVATE ${TEST_SUITES})
endif()

# enable dr mocks
target_compile_definitions(${TEST_UNIT_TARGET} PRIVATE TESTING)

if(DRACE_ENABLE_RUNTIME)
	add_dependencies(${TEST_UNIT_TARGET} "drace-client")
endif()

gtest_discover_tests(${TEST_UNIT_TARGET}
	WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
	PROPERTIES RUN_SERIAL ON)

add_custom_target(copy_files_testing ALL)
#DEPENDS "${PROJECT_SOURCE_DIR}/drace.ini"
#DEPENDS "$<TARGET_FILE:drace.detector.tsan>"
#)

if(DRACE_ENABLE_RUNTIME)
	if(DRACE_XML_EXPORTER)
		add_custom_command(TARGET copy_files_testing POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy_if_different
			"$<TARGET_FILE:tinyxml2>"
			"$<TARGET_FILE_DIR:${TEST_SYSTEM_TARGET}>")
	endif()

	add_custom_command(TARGET copy_files_testing POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy_if_different
			"${PROJECT_SOURCE_DIR}/test/data/drace_excl.ini"
			"$<TARGET_FILE_DIR:${TEST_UNIT_TARGET}>/data/drace_excl.ini")

	# copy tsan dll
	add_custom_command(TARGET copy_files_testing POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy_if_different
			"$<TARGET_FILE:drace.detector.tsan>"
			"$<TARGET_FILE_DIR:${TEST_UNIT_TARGET}>")

	# Copy tsan-common
	add_custom_command(TARGET copy_files_testing POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy_if_different
			"${TSAN_BINARY_DIR}/race_windows_amd64.dll"
			"$<TARGET_FILE_DIR:${TEST_UNIT_TARGET}>")

	add_custom_command(TARGET copy_files_testing POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy_if_different
			"$<TARGET_FILE:drace.detector.dummy>"
			"$<TARGET_FILE_DIR:${TEST_UNIT_TARGET}>")

			# copy gtest shared lib
	add_custom_command(TARGET copy_files_testing POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy_if_different
		"$<TARGET_FILE:gtest>"
		"$<TARGET_FILE_DIR:${TEST_UNIT_TARGET}>")
			
	add_custom_command(TARGET copy_files_testing POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy_if_different
		"$<TARGET_FILE:drace.detector.fasttrack>"
		"$<TARGET_FILE_DIR:${TEST_UNIT_TARGET}>")
endif()

add_custom_command(TARGET copy_files_testing POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy_if_different
		"$<TARGET_FILE:drace.detector.fasttrack.standalone>"
		"$<TARGET_FILE_DIR:${TEST_UNIT_TARGET}>")

# Prepare Guinea pigs
add_subdirectory("mini-apps")
		
if(${DRACE_INSTALL_TESTS} AND DRACE_ENABLE_RUNTIME)
	install(TARGETS ${TEST_UNIT_TARGET} RUNTIME DESTINATION bin COMPONENT RUNTIME)
	install(TARGETS ${TEST_SYSTEM_TARGET} RUNTIME DESTINATION bin COMPONENT RUNTIME)
    install(FILES ${PROJECT_SOURCE_DIR}/test/data/drace_excl.ini DESTINATION bin)
endif()


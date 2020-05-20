# DRace, a dynamic data race detector
#
# Copyright (c) Siemens AG, 2019
#
# Authors:
#   Felix Moessbauer <felix.moessbauer@siemens.com>
#
# This work is licensed under the terms of the MIT license.  See
# the LICENSE file in the top-level directory.

message(STATUS "Build detector printer")

add_library("drace.detector.printer" SHARED "printer")
target_link_libraries("drace.detector.printer" "drace-common")
install(TARGETS "drace.detector.printer"
	RUNTIME DESTINATION ${DRACE_RUNTIME_DEST} COMPONENT Runtime
	LIBRARY DESTINATION ${DRACE_ARCHIVE_DEST} COMPONENT ARCHIVE)

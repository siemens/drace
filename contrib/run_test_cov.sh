#!/bin/bash
#
# DRace, a dynamic data race detector
# Copyright 2018 Siemens AG
# Authors:
#   Felix Moessbauer <felix.moessbauer@siemens.com>
# SPDX-License-Identifier: MIT

# run this file from build directory

# generate baseline
lcov -c -i -d . -o baseline.info
# execute tests
ctest -j
# gather test coverage data
lcov -c -d . -o coverage_run.info
# merge cov data
lcov -a baseline.info -a coverage_run.info -o coverage_comb.info
# filter non drace sources
lcov --remove coverage_comb.info '/usr/*' '*/vendor/*' '*/test/mini-apps/*' '*/HowTo/*' -o coverage.info
# generate html report
genhtml coverage.info --output-directory coverage
# print hints
echo "Report is in coverage/index.html"

#pragma once
/*
 * DRace, a dynamic data race detector
 *
 * Copyright (c) Siemens AG, 2018
 *
 * Authors:
 *   Felix Moessbauer <felix.moessbauer@siemens.com>
 *
 * This work is licensed under the terms of the MIT license.  See
 * the LICENSE file in the top-level directory.
 */

#include <detector/detector_if.h>
#include <string>
#include <iostream>

static unsigned num_races = 0;

class DetectorTest : public ::testing::Test {
public:
	uint64_t stack[1];

public:
	// each callback-call increases num_races by two
	static void callback(const detector::Race*) {
		++num_races;
	}

	DetectorTest() {
		num_races = 0;
		if (detector::name() != "TSAN") {
			const char * _argv = "drace-tests.exe";
			detector::init(1, &_argv, callback);
		}
	}

	~DetectorTest() {
		if (detector::name() != "TSAN") {
			detector::finalize();
		}
	}

protected:
	// TSAN can only be initialized once, even after a finalize
	static void SetUpTestCase() {
		std::cout << "Detector: " << detector::name() << std::endl;
		if (detector::name() == "TSAN") {
			const char * _argv = "drace-tests-tsan.exe";
			detector::init(1, &_argv, callback);
		}
	}

	static void TearDownTestCase() {
		if (detector::name() == "TSAN") {
			detector::finalize();
		}
	}
};

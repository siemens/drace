/*
 * DRace, a dynamic data race detector
 *
 * Copyright 2018 Siemens AG
 *
 * Authors:
 *   Felix Moessbauer <felix.moessbauer@siemens.com>
 *
 * SPDX-License-Identifier: MIT
 */

#include "gtest/gtest.h"
#include "detectorTest.h"

TEST_F(DetectorTest, WR_Race) {
	detector::tls_t tls10;
	detector::tls_t tls11;

	detector::fork(1, 10, &tls10);
	detector::fork(1, 11, &tls11);

	detector::write(tls10, (void*)0x0010, (void*)0x0010, 8);
	detector::read(tls11, (void*)0x0011, (void*)0x0010, 8);

	EXPECT_EQ(num_races, 1);
}

TEST_F(DetectorTest, Mutex) {
	detector::tls_t tls20;
	detector::tls_t tls21;

	detector::fork(1, 20, &tls20);
	detector::fork(1, 21, &tls21);
	// First Thread
	detector::acquire(tls20, (void*)0x0100, 1, true);
	detector::write(tls20, (void*)0x0021, (void*)0x0020, 8);
	detector::read(tls20, (void*)0x0022, (void*)0x0020, 8);
	detector::release(tls20, (void*)0x0100, true);

	// Second Thread
	detector::acquire(tls21, (void*)0x0100, 1, true);
	detector::read(tls21, (void*)0x0024, (void*)0x0020, 8);
	detector::write(tls21, (void*)0x0025, (void*)0x0020, 8);
	detector::release(tls21, (void*)0x0100, true);

	EXPECT_EQ(num_races, 0);
}

TEST_F(DetectorTest, ThreadExit) {
	detector::tls_t tls30;
	detector::tls_t tls31;

	detector::fork(1, 30, &tls30);
	detector::write(tls30, (void*)0x0031, (void*)0x0032, 8);

	detector::fork(1, 31, &tls31);
	detector::write(tls31, (void*)0x0032, (void*)0x0032, 8);
	detector::join(1, 31);

	detector::read(tls30, (void*)0x0031, (void*)0x0032, 8);

	EXPECT_EQ(num_races, 0);
}

TEST_F(DetectorTest, MultiFork) {
	detector::tls_t tls40;
	detector::tls_t tls41;
	detector::tls_t tls42;

	detector::fork(1, 40, &tls40);
	detector::fork(1, 41, &tls41);
	detector::fork(1, 42, &tls42);

	EXPECT_EQ(num_races, 0);
}

TEST_F(DetectorTest, HappensBefore) {
	detector::tls_t tls50;
	detector::tls_t tls51;

	detector::fork(1, 50, &tls50);
	detector::fork(1, 51, &tls51);

	detector::write(tls50, (void*)0x0050, (void*)0x0050, 8);
	detector::happens_before(tls50, (void*)5051);
	detector::happens_after(tls51, (void*)5051);
	detector::write(tls51, (void*)0x0051, (void*)0x0050, 8);

	EXPECT_EQ(num_races, 0);
}

TEST_F(DetectorTest, ForkInitialize) {
	detector::tls_t tls60;
	detector::tls_t tls61;

	detector::fork(1, 60, &tls60);
	detector::write(tls60, (void*)0x0060, (void*)0x0060, 8);
	detector::fork(1, 61, &tls61);
	detector::write(tls61, (void*)0x0060, (void*)0x0060, 8);

	EXPECT_EQ(num_races, 0);
}

TEST_F(DetectorTest, Barrier) {
	detector::tls_t tls70;
	detector::tls_t tls71;
	detector::tls_t tls72;

	detector::fork(1, 70, &tls70);
	detector::fork(1, 71, &tls71);
	detector::fork(1, 72, &tls72);

	detector::write(tls70, (void*)0x0070, (void*)0x0070, 8);
	detector::write(tls70, (void*)0x0070, (void*)0x0071, 8);
	detector::write(tls71, (void*)0x0070, (void*)0x0171, 8);

	// Barrier
	{
		void* barrier_id = (void*)0x0700;
		// barrier enter
		detector::happens_before(tls70, barrier_id);
		// each thread enters individually
		detector::write(tls71, (void*)0x0070, (void*)0x0072, 8);
		detector::happens_before(tls71, barrier_id);

		// sufficient threads have arrived => barrier leave
		detector::happens_after(tls71, barrier_id);
		detector::write(tls71, (void*)0x0070, (void*)0x0071, 8);
		detector::happens_after(tls70, barrier_id);
	}

	detector::write(tls71, (void*)0x0071, (void*)0x0070, 8);
	detector::write(tls70, (void*)0x0072, (void*)0x0072, 8);

	EXPECT_EQ(num_races, 0);
	// This thread did not paricipate in barrier, hence expect race
	detector::write(tls72, (void*)0x0072, (void*)0x0071, 8);
	EXPECT_EQ(num_races, 1);
}

TEST_F(DetectorTest, ResetRange) {
	detector::tls_t tls80;
	detector::tls_t tls81;

	detector::fork(1, 80, &tls80);
	detector::fork(1, 81, &tls81);

	detector::allocate(tls80, (void*)0x0080, (void*)0x0080, 0xF);
	detector::write(tls80, (void*)0x0080, (void*)0x0082, 8);
	detector::deallocate(tls80, (void*)0x0080);
	detector::happens_before(tls80, (void*)0x0080);

	detector::happens_after(tls81, (void*)0x0080);
	detector::allocate(tls81, (void*)0x0080, (void*)0x0080, 0x2);
	detector::write(tls81, (void*)0x0080, (void*)0x0082, 8);
	detector::deallocate(tls81, (void*)0x0080);
	EXPECT_EQ(num_races, 0);
}
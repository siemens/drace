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

std::unordered_map<std::string, std::shared_ptr<util::LibraryLoader>> DetectorTest::_libs;
std::unordered_map<std::string, Detector*> DetectorTest::_detectors;

TEST_P(DetectorTest, WR_Race) {
	Detector::tls_t tls10;
	Detector::tls_t tls11;

	detector->fork(1, 10, &tls10);
	detector->fork(1, 11, &tls11);

	detector->write(tls10, (void*)0x0010, (void*)0x00100000, 8);
	detector->read( tls11, (void*)0x0011, (void*)0x00100000, 8);

	EXPECT_EQ(num_races, 1);
}

TEST_P(DetectorTest, WR2_Race) {
    Detector::tls_t tls13;
    Detector::tls_t tls14;
    Detector::tls_t tls15;

    detector->fork(1, 13, &tls13);
    detector->fork(1, 14, &tls14);
    detector->fork(1, 15, &tls15);

    detector->acquire(tls13, (void*)0x01000000, 1, true);
    detector->write(tls13, (void*)0x0010, (void*)0x00110000, 8);
    detector->read(tls13, (void*)0x0011, (void*)0x00110000, 8);
    detector->release(tls13, (void*)0x01000000, true);
    EXPECT_EQ(num_races, 0);

    detector->acquire(tls15, (void*)0x01000000, 1, true);
    detector->read(tls15, (void*)0x0011, (void*)0x00110000, 8);
    detector->release(tls15, (void*)0x01000000, true);
    EXPECT_EQ(num_races, 0);

    detector->acquire(tls14, (void*)0x01000000, 1, true);
    detector->read(tls14, (void*)0x0011, (void*)0x00110000, 8);
    detector->release(tls14, (void*)0x01000000, true);
    EXPECT_EQ(num_races, 0);
    
    detector->write(tls15, (void*)0x0011, (void*)0x00110000, 8);
    EXPECT_EQ(num_races, 1);
}

TEST_P(DetectorTest, WW_Race) {
    Detector::tls_t tls16;
    Detector::tls_t tls17;

    detector->fork(1, 16, &tls16);
    detector->fork(1, 17, &tls17);

    detector->write(tls16, (void*)0x0010, (void*)0x00120000, 8);
    detector->write(tls17, (void*)0x0011, (void*)0x00120000, 8);

    EXPECT_EQ(num_races, 1);
}

TEST_P(DetectorTest, RW_Race) {
    Detector::tls_t tls18;
    Detector::tls_t tls19;

    detector->fork(1, 18, &tls18);
    detector->fork(1, 19, &tls19);

    detector->acquire(tls18, (void*)0x01000000, 1, true);
    detector->write(tls18, (void*)0x0010, (void*)0x00130000, 8);
    detector->release(tls18, (void*)0x01000000, true);

    detector->acquire(tls19, (void*)0x01000000, 1, true);
    detector->read(tls19, (void*)0x0011, (void*)0x00130000, 8);
    detector->release(tls19, (void*)0x01000000, true);
    EXPECT_EQ(num_races, 0);

    detector->write(tls18, (void*)0x0010, (void*)0x00130000, 8);
    EXPECT_EQ(num_races, 1);
}

TEST_P(DetectorTest, Mutex) {
    Detector::tls_t tls20;
    Detector::tls_t tls21;

	detector->fork(1, 20, &tls20);
	detector->fork(1, 21, &tls21);
	// First Thread
	detector->acquire(tls20, (void*)0x01000000, 1, true);
	detector->write(tls20, (void*)0x0021, (void*)0x00200000, 8);
	detector->read(tls20, (void*)0x0022, (void*)0x00200000, 8);
	detector->release(tls20, (void*)0x01000000, true);

	// Second Thread
	detector->acquire(tls21, (void*)0x01000000, 1, true);
	detector->read(tls21, (void*)0x0024, (void*)0x00200000, 8);
	detector->write(tls21, (void*)0x0025, (void*)0x00200000, 8);
	detector->release(tls21, (void*)0x01000000, true);

	EXPECT_EQ(num_races, 0);
}

TEST_P(DetectorTest, VarLength){
    Detector::tls_t tls22;
    Detector::tls_t tls23;

    detector->fork(1, 22, &tls22);
    detector->fork(1, 23, &tls23);

    
    detector->write(tls22, (void*)0x0010, (void*)0x14000000, 8);
    detector->write(tls23, (void*)0x0011, (void*)0x14000002, 8);
    
    /// \todo set test 'online' when a working handling of variable lengths is implemented 
    //EXPECT_EQ(num_races, 1);
}

TEST_P(DetectorTest, ThreadExit) {
	Detector::tls_t tls30;
	Detector::tls_t tls31;

	detector->fork(1, 30u, &tls30);
	detector->write(tls30, (void*)0x0031, (void*)0x00320000, 8);

	detector->fork(1, 31u, &tls31);
	detector->write(tls31, (void*)0x0032, (void*)0x00320000, 8);
	detector->join(30u, 31u);

	detector->read(tls30, (void*)0x0031, (void*)0x00320000, 8);

    if (std::string(detector->name()) != "FASTTRACK") {
        EXPECT_EQ(num_races, 0);
    }
    else {
        // fasttrack is detecting a race here
        // because the two child threads write unsynchronised to the same var
        EXPECT_EQ(num_races, 1);
    }
}

TEST_P(DetectorTest, MultiFork) {
	Detector::tls_t tls40;
	Detector::tls_t tls41;
	Detector::tls_t tls42;

	detector->fork(1, 40, &tls40);
	detector->fork(1, 41, &tls41);
	detector->fork(1, 42, &tls42);

	EXPECT_EQ(num_races, 0);
}

TEST_P(DetectorTest, HappensBefore) {
	Detector::tls_t tls50;
	Detector::tls_t tls51;

	detector->fork(1, 50, &tls50);
	detector->fork(1, 51, &tls51);

	detector->write(tls50, (void*)0x0050, (void*)0x00500000, 8);
	detector->happens_before(tls50, (void*)50510000);
	detector->happens_after(tls51, (void*)50510000);
	detector->write(tls51, (void*)0x0051, (void*)0x00500000, 8);

	EXPECT_EQ(num_races, 0);
}




TEST_P(DetectorTest, ForkInitialize) {
	Detector::tls_t tls60;
	Detector::tls_t tls61;

	detector->fork(1, 60, &tls60);
	detector->write(tls60, (void*)0x0060, (void*)0x00600000, 8);
	detector->fork(1, 61, &tls61);
	detector->write(tls61, (void*)0x0060, (void*)0x00600000, 8);

    if (std::string(detector->name()) != "FASTTRACK") {
        EXPECT_EQ(num_races, 0);
    }
    else {
        // fasttrack is detecting a race here
        // because the two child threads write unsynchronised to the same var
        EXPECT_EQ(num_races, 1);
    }
}

TEST_P(DetectorTest, Barrier) {
	Detector::tls_t tls70;
	Detector::tls_t tls71;
	Detector::tls_t tls72;

	detector->fork(1, 70, &tls70);
	detector->fork(1, 71, &tls71);
	detector->fork(1, 72, &tls72);

	detector->write(tls70, (void*)0x0070, (void*)0x00700000, 8);
	detector->write(tls70, (void*)0x0070, (void*)0x00710000, 8);
	detector->write(tls71, (void*)0x0070, (void*)0x01710000, 8);

	// Barrier
	{
		void* barrier_id = (void*)0x0700;
		// barrier enter
		detector->happens_before(tls70, barrier_id);
		// each thread enters individually
		detector->write(tls71, (void*)0x0070, (void*)0x00720000, 8);
		detector->happens_before(tls71, barrier_id);

		// sufficient threads have arrived => barrier leave
		detector->happens_after(tls71, barrier_id);
		detector->write(tls71, (void*)0x0070, (void*)0x00710000, 8);
		detector->happens_after(tls70, barrier_id);
	}

	detector->write(tls71, (void*)0x0071, (void*)0x00700000, 8);
	detector->write(tls70, (void*)0x0072, (void*)0x00720000, 8);

	EXPECT_EQ(num_races, 0);
	// This thread did not paricipate in barrier, hence expect race
	detector->write(tls72, (void*)0x0072, (void*)0x00710000, 8);
	EXPECT_EQ(num_races, 1);
}

TEST_P(DetectorTest, ResetRange) {
	Detector::tls_t tls80;
	Detector::tls_t tls81;

	detector->fork(1, 80, &tls80);
	detector->fork(1, 81, &tls81);

	detector->allocate(tls80, (void*)0x0080, (void*)0x00800000, 0xF);
	detector->write(tls80, (void*)0x0080, (void*)0x00820000, 8);
	detector->deallocate(tls80, (void*)0x00800000);
	detector->happens_before(tls80, (void*)0x00800000);

	detector->happens_after(tls81, (void*)0x00800000);
	detector->allocate(tls81, (void*)0x0080, (void*)0x00800000, 0x2);
	detector->write(tls81, (void*)0x0080, (void*)0x00820000, 8);
	detector->deallocate(tls81, (void*)0x00800000);
	EXPECT_EQ(num_races, 0);
}


void callstack_funA() {};
void callstack_funB() {};

TEST_P(DetectorTest, RaceInspection) {
    Detector::tls_t tls90, tls91;

    detector->fork(1, 90, &tls90);
    detector->fork(1, 91, &tls91);

    EXPECT_NE(tls90, tls91);

    // Thread A
    detector->func_enter(tls90, (void*)&callstack_funA);
    detector->func_enter(tls90, (void*)&callstack_funB);

    detector->write(tls90, (void*)0x0090, (void*)0x00920000, 8);

    detector->func_exit(tls90);
    detector->func_exit(tls90);

    // Thread B
    detector->func_enter(tls91, (void*)&callstack_funB);
    detector->read(tls91, (void*)0x0091, (void*)0x00920000, 8);
    detector->func_exit(tls91);

    EXPECT_EQ(num_races, 1);

    // races are not ordered, but we need an order for
    // the assertions. Hence, order by tid.
    auto a1 = last_race.first.thread_id < last_race.second.thread_id ? last_race.first : last_race.second;
    auto a2 = last_race.first.thread_id < last_race.second.thread_id ? last_race.second : last_race.first;

    EXPECT_NE(a1.thread_id, a2.thread_id);
    EXPECT_EQ(a1.thread_id, 90);
    EXPECT_EQ(a2.thread_id, 91);

    EXPECT_EQ(a1.accessed_memory, 0x00920000ull);
    EXPECT_EQ(a2.accessed_memory, 0x00920000ull);

    EXPECT_EQ(a1.write, true);
    EXPECT_EQ(a2.write, false);

    ASSERT_EQ(a1.stack_size, 3);
    ASSERT_EQ(a2.stack_size, 2);

    EXPECT_EQ(a1.stack_trace[0], (uint64_t)&callstack_funA);
    EXPECT_EQ(a1.stack_trace[1], (uint64_t)&callstack_funB);
    EXPECT_EQ(a1.stack_trace[2], 0x0090ull);

    EXPECT_EQ(a2.stack_trace[0], (uint64_t)&callstack_funB);
    EXPECT_EQ(a2.stack_trace[1], 0x0091ull);
}

TEST_P(DetectorTest, Recursive_Lock){
	Detector::tls_t tls100;
    Detector::tls_t tls101;

	detector->fork(1, 100, &tls100);
	detector->fork(1, 101, &tls101);

    detector->acquire(tls100, (void*)0x01000000, 1, true);
    detector->acquire(tls100, (void*)0x01000000, 2, true);

	detector->write(tls100, (void*)0x0010, (void*)0x00100002, 8);
    
    detector->release(tls100, (void*)0x01000000, true);
    detector->release(tls100, (void*)0x01000000, true);

    detector->acquire(tls101, (void*)0x01000000, 1, true);
    detector->acquire(tls101, (void*)0x01000000, 2, true);
	detector->read( tls101, (void*)0x0011, (void*)0x00100001, 8);
    detector->release(tls100, (void*)0x01000000, true);
    detector->release(tls100, (void*)0x01000000, true);

	EXPECT_EQ(num_races, 0);
}

TEST_P(DetectorTest, Reader_Writer_Lock){
	Detector::tls_t tls110;
    Detector::tls_t tls111;

	detector->fork(1, 110, &tls110);
	detector->fork(1, 111, &tls111);

    detector->acquire(tls110, (void*)0x01000000, 1, false);
    detector->acquire(tls111, (void*)0x01000000, 1, false);
	detector->read(tls111, (void*)0x0011, (void*)0x00100003, 8);
	detector->read(tls110, (void*)0x0011, (void*)0x00100003, 8);
    detector->release(tls111, (void*)0x01000000, false);
    detector->release(tls110, (void*)0x01000000, false);

    detector->acquire(tls110, (void*)0x01000000, 1, true);
	detector->write(tls110, (void*)0x0010, (void*)0x00100003, 8);
    detector->release(tls110, (void*)0x01000000, true);
    

	EXPECT_EQ(num_races, 0);
}

TEST_P(DetectorTest, HA_before_HB) {
    Detector::tls_t tls120;
    Detector::tls_t tls121;

    detector->fork(1, 120, &tls120);
    detector->fork(1, 121, &tls121);

    detector->happens_after(tls121, (void*)50510001);
    detector->happens_before(tls120, (void*)50510001);
    //a detector must not crash on something like this
}


// TODO: i#11
#if 0
TEST_F(DetectorTest, ShadowMemory) {
    detector->tls_t tls100;
    detector->fork(1, 100, &tls100);

    // DLLs are loaded at 07ff8 xxxx xxxx most times
    uint64_t shadow_beg = 0x7ff7'0000'0000ull;

    detector->map_shadow((void*)(shadow_beg), (size_t)(0xFFFF'FFFF - 4096));
    detector->write(tls100, (void*)0x0100, (void*)(shadow_beg + 0xF), 8);
}
#endif

// Setup value-parameterized tests
#ifdef WIN32
INSTANTIATE_TEST_CASE_P(Interface,
    DetectorTest, ::testing::Values("fasttrack.standalone", "tsan"));
#else
INSTANTIATE_TEST_CASE_P(Interface,
    DetectorTest, ::testing::Values("fasttrack.standalone"));
#endif

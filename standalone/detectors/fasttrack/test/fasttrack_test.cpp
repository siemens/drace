/*
 * DRace, a dynamic data race detector
 *
 * Copyright 2018 Siemens AG
 *
 * Authors:
 *   Philip Harr <philip.harr@siemens.com>
 *   Mihai Robescu <mihai-gabriel.robescu@siemens.com>
 *
 * SPDX-License-Identifier: MIT
 */

#include <random>
#include "fasttrack_wrapper.h"
#include "gtest/gtest.h"
#include "stacktrace.h"

using ::testing::UnitTest;

TEST(FasttrackTest, BasicStackTrace) {
  using namespace drace::detector;
  auto ft = std::make_unique<FasttrackWrapper<std::mutex>>();

  auto rc_clb = [](const Detector::Race* r, void*) {};
  const char* argv_mock[] = {"ft_test"};
  void* tls[1];  // storage for TLS data

  ft->init(1, argv_mock, rc_clb, nullptr);
  ft->fork(0, 1, &tls[0]);  // t0

  ft->func_enter(tls[0], (void*)1);
  ft->read(tls[0], (void*)1000, (void*)100, 8);
  ft->read(tls[0], (void*)1001, (void*)101, 8);

  ft->func_enter(tls[0], (void*)2);
  ft->read(tls[0], (void*)1002, (void*)102, 8);

  ft->func_enter(tls[0], (void*)3);
  ft->read(tls[0], (void*)1004, (void*)103, 8);

  ft->func_exit(tls[0]);

  ft->func_enter(tls[0], (void*)1);
  ft->func_enter(tls[0], (void*)2);
  ft->func_enter(tls[0], (void*)6);
  ft->read(tls[0], (void*)1004, (void*)104, 8);
  ft->func_enter(tls[0], (void*)7);

  ThreadState* thr = reinterpret_cast<ThreadState*>(tls[0]);
  auto list = thr->return_stack_trace(104);
  std::vector<size_t> vec(list.begin(), list.end());

  ft->finish(tls[0], 1);

  ASSERT_EQ(vec[0], 1);
  ASSERT_EQ(vec[1], 2);
  ASSERT_EQ(vec[2], 1);
  ASSERT_EQ(vec[3], 2);
  ASSERT_EQ(vec[4], 6);
  ASSERT_EQ(vec[5], 1004);
  ASSERT_EQ(vec.size(), 6);
}

TEST(FasttrackTest, ComplexStackTrace) {
  using namespace drace::detector;
  auto ft = std::make_unique<FasttrackWrapper<std::mutex>>();

  auto rc_clb = [](const Detector::Race* r, void*) {};
  const char* argv_mock[] = {"ft_test"};
  void* tls[2];  // storage for TLS data

  ft->init(1, argv_mock, rc_clb, nullptr);
  ft->fork(0, 1, &tls[0]);  // t0
  ft->fork(0, 2, &tls[1]);  // t1

  ft->func_enter(tls[0], (void*)1);
  ft->func_enter(tls[0], (void*)3);
  ft->func_enter(tls[0], (void*)5);
  ft->func_exit(tls[0]);
  ft->func_enter(tls[0], (void*)7);
  ft->read(tls[0], (void*)1000, (void*)100, 8);
  ft->read(tls[0], (void*)1001, (void*)101, 8);
  ft->read(tls[0], (void*)1004, (void*)104, 8);

  ft->func_enter(tls[1], (void*)1);
  ft->func_enter(tls[1], (void*)2);
  ft->func_enter(tls[1], (void*)4);
  ft->func_exit(tls[1]);
  ft->func_enter(tls[1], (void*)6);
  ft->func_enter(tls[1], (void*)8);
  ft->read(tls[1], (void*)1002, (void*)102, 8);
  ft->read(tls[1], (void*)1004, (void*)103, 8);
  ft->read(tls[1], (void*)1004, (void*)104, 8);
  ft->func_exit(tls[1]);
  ft->func_enter(tls[1], (void*)10);

  ThreadState* thr0 = reinterpret_cast<ThreadState*>(tls[0]);
  auto list0 = thr0->return_stack_trace(104);
  std::vector<size_t> vec0(list0.begin(), list0.end());

  ThreadState* thr1 = reinterpret_cast<ThreadState*>(tls[1]);
  auto list1 = thr1->return_stack_trace(104);
  std::vector<size_t> vec1(list1.begin(), list1.end());

  ft->finish(tls[0], 1);
  ft->finish(tls[1], 2);

  ASSERT_EQ(vec0.size(), 4);
  ASSERT_EQ(vec1.size(), 5);
  ASSERT_EQ(vec0[0], 1);
  ASSERT_EQ(vec0[1], 3);
  ASSERT_EQ(vec0[2], 7);
  ASSERT_EQ(vec0[3], 1004);
  ASSERT_EQ(vec1[0], 1);
  ASSERT_EQ(vec1[1], 2);
  ASSERT_EQ(vec1[2], 6);
  ASSERT_EQ(vec1[3], 8);
  ASSERT_EQ(vec1[4], 1004);
}

TEST(FasttrackTest, MoreComplexStackTrace) {
  using namespace drace::detector;
  auto ft = std::make_unique<FasttrackWrapper<std::mutex>>();

  auto rc_clb = [](const Detector::Race* r, void*) {};
  const char* argv_mock[] = {"ft_test"};
  void* tls[2];  // storage for TLS data

  ft->init(1, argv_mock, rc_clb, nullptr);
  ft->fork(0, 1, &tls[0]);  // t0
  ft->fork(0, 2, &tls[1]);  // t1

  ft->func_enter(tls[0], (void*)0x1ull);
  ft->func_enter(tls[0], (void*)0x2ull);
  ft->func_enter(tls[0], (void*)0x6ull);
  ft->func_enter(tls[0], (void*)0x3ull);
  ft->func_enter(tls[0], (void*)0x2ull);
  ft->func_exit(tls[0]);
  ft->func_enter(tls[0], (void*)0x6ull);
  ft->func_exit(tls[0]);
  ft->func_enter(tls[0], (void*)0x2ull);
  ft->func_enter(tls[0], (void*)0x7ull);
  ft->read(tls[0], (void*)0x1001ull, (void*)0x101ull, 8);
  ft->func_exit(tls[0]);
  ft->func_exit(tls[0]);
  ft->func_exit(tls[0]);
  ft->func_enter(tls[0], (void*)0x2ull);
  ft->func_enter(tls[0], (void*)0x6ull);
  ft->read(tls[0], (void*)0x1002ull, (void*)0x102ull, 8);
  ft->func_enter(tls[0], (void*)0x3ull);
  ft->func_enter(tls[0], (void*)0x2ull);
  ft->func_enter(tls[0], (void*)0x6ull);
  ft->func_enter(tls[0], (void*)0x3ull);
  ft->read(tls[0], (void*)0x1003ull, (void*)0x103ull, 8);

  ThreadState* thr0 = reinterpret_cast<ThreadState*>(tls[0]);
  auto trace_101 = thr0->return_stack_trace(0x101ull);
  std::vector<size_t> vec_101(trace_101.begin(), trace_101.end());

  auto trace_102 = thr0->return_stack_trace(0x102ull);
  std::vector<size_t> vec_102(trace_102.begin(), trace_102.end());

  auto trace_103 = thr0->return_stack_trace(0x103ull);
  std::vector<size_t> vec_103(trace_103.begin(), trace_103.end());

  ASSERT_EQ(vec_101.size(), 7);
  ASSERT_EQ(vec_101[0], 0x1ull);
  ASSERT_EQ(vec_101[1], 0x2ull);
  ASSERT_EQ(vec_101[2], 0x6ull);
  ASSERT_EQ(vec_101[3], 0x3ull);
  ASSERT_EQ(vec_101[4], 0x2ull);
  ASSERT_EQ(vec_101[5], 0x7ull);
  ASSERT_EQ(vec_101[6], 0x1001ull);
  ASSERT_EQ(vec_102.size(), 6);
  ASSERT_EQ(vec_102[0], 0x1ull);
  ASSERT_EQ(vec_102[1], 0x2ull);
  ASSERT_EQ(vec_102[2], 0x6ull);
  ASSERT_EQ(vec_102[3], 0x2ull);
  ASSERT_EQ(vec_102[4], 0x6ull);
  ASSERT_EQ(vec_102[5], 0x1002ull);
  ASSERT_EQ(vec_103.size(), 10);
  ASSERT_EQ(vec_103[0], 0x1ull);
  ASSERT_EQ(vec_103[1], 0x2ull);
  ASSERT_EQ(vec_103[2], 0x6ull);
  ASSERT_EQ(vec_103[3], 0x2ull);
  ASSERT_EQ(vec_103[4], 0x6ull);
  ASSERT_EQ(vec_103[5], 0x3ull);
  ASSERT_EQ(vec_103[6], 0x2ull);
  ASSERT_EQ(vec_103[7], 0x6ull);
  ASSERT_EQ(vec_103[8], 0x3ull);
  ASSERT_EQ(vec_103[9], 0x1003ull);
}

TEST(FasttrackTest, Indicate_Write_Write_Race) {
  std::size_t addr = 0x43EFull;
  using namespace drace::detector;
  auto ft = std::make_unique<FasttrackWrapper<std::mutex>>();

  auto rc_clb = [](const Detector::Race* r, void*) {};
  const char* argv_mock[] = {"ft_test"};
  void* tls[2];  // storage for TLS data

  ft->init(1, argv_mock, rc_clb, nullptr);
  ft->fork(0, 1, &tls[0]);  // t0
  ft->fork(0, 2, &tls[1]);  // t1

  ft->write(tls[0], (void*)0x78Eull, (void*)addr, 16);
  ft->write(tls[1], (void*)0x3A4Dull, (void*)addr, 16);

  EXPECT_EQ(ft->getData().wr_race, 0);
  EXPECT_EQ(ft->getData().rw_ex_race, 0);
  EXPECT_EQ(ft->getData().ww_race, 1);
  EXPECT_EQ(ft->getData().rw_sh_race, 0);
  ft->finalize();
}

TEST(FasttrackTest, Indicate_Exclusive_Read_Write_Race) {
  std::size_t addr = 0x65BEull;
  using namespace drace::detector;
  auto ft = std::make_unique<FasttrackWrapper<std::mutex>>();

  auto rc_clb = [](const Detector::Race* r, void*) {};
  const char* argv_mock[] = {"ft_test"};
  void* tls[2];  // storage for TLS data

  ft->init(1, argv_mock, rc_clb, nullptr);
  ft->fork(0, 1, &tls[0]);  // t0
  ft->fork(0, 2, &tls[1]);  // t1

  ft->read(tls[0], (void*)0x687CDull, (void*)addr, 16);
  ft->write(tls[1], (void*)0x9765Dull, (void*)addr, 16);

  EXPECT_EQ(ft->getData().wr_race, 0);
  EXPECT_EQ(ft->getData().rw_ex_race, 1);
  EXPECT_EQ(ft->getData().ww_race, 0);
  EXPECT_EQ(ft->getData().rw_sh_race, 0);
  ft->finalize();
}

TEST(FasttrackTest, Indicate_Write_Read_Race) {
  std::size_t addr = 0x342BEull;
  using namespace drace::detector;
  auto ft = std::make_unique<FasttrackWrapper<std::mutex>>();

  auto rc_clb = [](const Detector::Race* r, void*) {};
  const char* argv_mock[] = {"ft_test"};
  void* tls[2];  // storage for TLS data

  ft->init(1, argv_mock, rc_clb, nullptr);
  ft->fork(0, 1, &tls[0]);  // t0
  ft->fork(0, 2, &tls[1]);  // t1

  ft->write(tls[1], (void*)0x9868Dull, (void*)addr, 16);
  ft->read(tls[0], (void*)0x434CDull, (void*)addr, 16);

  EXPECT_EQ(ft->getData().wr_race, 1);
  EXPECT_EQ(ft->getData().rw_ex_race, 0);
  EXPECT_EQ(ft->getData().ww_race, 0);
  EXPECT_EQ(ft->getData().rw_sh_race, 0);
  ft->finalize();
}

TEST(FasttrackTest, Indicate_No_Race_Read_Exclusive) {
  std::size_t addr = 0x42Full;
  using namespace drace::detector;
  auto ft = std::make_unique<FasttrackWrapper<std::mutex>>();

  auto rc_clb = [](const Detector::Race* r, void*) {};
  const char* argv_mock[] = {"ft_test"};
  void* tls[3];  // storage for TLS data

  ft->init(1, argv_mock, rc_clb, nullptr);
  ft->fork(0, 1, &tls[0]);  // t0
  ft->fork(0, 2, &tls[1]);  // t1

  ft->acquire(tls[1], (void*)0x01000000, 1, true);
  ft->write(tls[1], (void*)0x5FFull, (void*)addr, 16);
  ft->release(tls[1], (void*)0x01000000, true);

  ft->acquire(tls[0], (void*)0x01000000, 1, false);
  ft->read(tls[0], (void*)0x5F3ull, (void*)addr, 16);
  ft->release(tls[0], (void*)0x01000000, true);

  EXPECT_EQ(ft->getData().wr_race, 0);
  EXPECT_EQ(ft->getData().rw_ex_race, 0);
  EXPECT_EQ(ft->getData().ww_race, 0);
  EXPECT_EQ(ft->getData().rw_sh_race, 0);
  EXPECT_EQ(ft->getData().read_exclusive, 1);
  ft->finalize();
}

TEST(FasttrackTest, Indicate_No_Race_Write_Exclusive) {
  std::size_t addr = 0x42Full;
  using namespace drace::detector;
  auto ft = std::make_unique<FasttrackWrapper<std::mutex>>();

  auto rc_clb = [](const Detector::Race* r, void*) {};
  const char* argv_mock[] = {"ft_test"};
  void* tls[3];  // storage for TLS data

  ft->init(1, argv_mock, rc_clb, nullptr);
  ft->fork(0, 1, &tls[0]);  // t0
  ft->fork(0, 2, &tls[1]);  // t1

  ft->acquire(tls[1], (void*)0x01000000, 1, true);
  ft->read(tls[1], (void*)0x5F3ull, (void*)addr, 16);
  ft->release(tls[1], (void*)0x01000000, true);

  ft->acquire(tls[0], (void*)0x01000000, 1, false);
  ft->write(tls[0], (void*)0x5FFull, (void*)addr, 16);
  ft->release(tls[0], (void*)0x01000000, true);

  EXPECT_EQ(ft->getData().wr_race, 0);
  EXPECT_EQ(ft->getData().rw_ex_race, 0);
  EXPECT_EQ(ft->getData().ww_race, 0);
  EXPECT_EQ(ft->getData().rw_sh_race, 0);
  EXPECT_EQ(ft->getData().read_exclusive, 1);
  EXPECT_EQ(ft->getData().write_exclusive, 1);
  ft->finalize();
}

// this would have failed with the former condition in write():
// write exclusive with VAR_NOT_INIT
TEST(FasttrackTest, Indicate_Shared_Read_Write_Race) {
  std::size_t addr = 0x42Full;
  using namespace drace::detector;
  auto ft = std::make_unique<FasttrackWrapper<std::mutex>>();

  auto rc_clb = [](const Detector::Race* r, void*) {
    ASSERT_EQ(r->first.stack_size, 1);
    ASSERT_EQ(r->second.stack_size, 1);
    // first stack
    EXPECT_EQ(r->first.stack_trace[0], 0x5F3ull);
    // second stack
    EXPECT_EQ(r->second.stack_trace[0], 0x78Eull);
  };
  const char* argv_mock[] = {"ft_test"};
  void* tls[3];  // storage for TLS data

  ft->init(1, argv_mock, rc_clb, nullptr);
  ft->fork(0, 1, &tls[0]);  // t0
  ft->fork(0, 2, &tls[1]);  // t1
  ft->fork(0, 3, &tls[2]);  // t2

  // t1 and t2 read addr
  // t3 tries to write to addr
  ft->read(tls[0], (void*)0x5F3ull, (void*)addr, 16);
  ft->read(tls[1], (void*)0x5FFull, (void*)addr, 16);
  ft->write(tls[2], (void*)0x78Eull, (void*)addr, 16);

  EXPECT_EQ(ft->getData().wr_race, 0);
  EXPECT_EQ(ft->getData().rw_ex_race, 0);
  EXPECT_EQ(ft->getData().ww_race, 0);
  EXPECT_EQ(ft->getData().rw_sh_race, 1);
  ft->finalize();
}

// check that read_shared variables can be dropped
TEST(FasttrackTest, Drop_State_Indicate_Shared_Read_Write_Race) {
  std::size_t addr[] = {0x5678Full, 0x678Full, 0x78Full};

  using namespace drace::detector;
  auto ft = std::make_unique<FasttrackWrapper<std::mutex>>();

  auto rc_clb = [](const Detector::Race* r, void*) {};
  const char* argv_mock[] = {"ft_test", "--size", "2"};
  void* tls[3];  // storage for TLS data

  ft->init(3, argv_mock, rc_clb, nullptr);
  ft->fork(0, 1, &tls[0]);  // t0
  ft->fork(0, 2, &tls[1]);  // t1
  ft->fork(0, 3, &tls[2]);  // t2

  ft->read(tls[0], (void*)0x5F3ull, (void*)addr[0], 16);
  ft->read(tls[1], (void*)0x5FFull, (void*)addr[0], 16);

  ft->clear_var_state_helper(addr[0]);

  ft->write(tls[2], (void*)0x78Eull, (void*)addr[0], 16);

  EXPECT_EQ(ft->getData().wr_race, 0);
  EXPECT_EQ(ft->getData().rw_ex_race, 0);
  EXPECT_EQ(ft->getData().ww_race, 0);
  EXPECT_EQ(ft->getData().rw_sh_race, 1);
  ft->finalize();
}

TEST(FasttrackTest, Write_Write_Race) {
  std::size_t addr0 = 0x700ull;
  std::size_t addr1 = 0x701ull;
  std::size_t addr2 = 0x702ull;
  std::size_t addr3 = 0x703ull;
  std::size_t addr4 = 0x704ull;
  std::size_t addr5 = 0x705ull;

  using namespace drace::detector;
  auto ft = std::make_unique<FasttrackWrapper<std::mutex>>();

  auto rc_clb = [](const Detector::Race* r, void*) {};
  const char* argv_mock[] = {"ft_test", "--size",
                             "10"};  // making size bigger so the first variable
                                     // will not be removed again
  void* tls[3];                      // storage for TLS data
  void* mtx[2] = {(void*)0x123ull, (void*)0x1234ull};

  ft->init(3, argv_mock, rc_clb, nullptr);
  ft->fork(0, 1, &tls[0]);  // t0
  ft->fork(0, 2, &tls[1]);  // t1
  ft->fork(0, 3, &tls[2]);  // t2

  ft->write(tls[1], (void*)0x78Eull, (void*)addr0, 16);

  ft->acquire(tls[0], mtx[0], 1, true);
  ft->read(tls[0], (void*)0x5F3ull, (void*)addr1, 16);
  ft->release(tls[0], mtx[0], true);
  ft->acquire(tls[0], mtx[0], 1, true);
  ft->read(tls[0], (void*)0x5F3ull, (void*)addr1, 16);
  ft->release(tls[0], mtx[0], true);

  ft->acquire(tls[2], mtx[0], 1, true);
  ft->read(tls[2], (void*)0x5F3ull, (void*)addr2, 16);
  ft->release(tls[2], mtx[0], true);
  ft->acquire(tls[2], mtx[0], 1, true);
  ft->read(tls[2], (void*)0x5F3ull, (void*)addr2, 16);
  ft->release(tls[2], mtx[0], true);

  ft->acquire(tls[1], mtx[1], 1, true);
  ft->read(tls[1], (void*)0x5F3ull, (void*)addr3, 16);
  ft->release(tls[1], mtx[1], true);
  ft->acquire(tls[1], mtx[1], 1, true);
  ft->read(tls[1], (void*)0x5F3ull, (void*)addr3, 16);
  ft->release(tls[1], mtx[1], true);

  ft->acquire(tls[0], mtx[0], 1, true);
  ft->read(tls[0], (void*)0x5F3ull, (void*)addr4, 16);
  ft->release(tls[0], mtx[0], true);
  ft->acquire(tls[0], mtx[0], 1, true);
  ft->read(tls[0], (void*)0x5F3ull, (void*)addr4, 16);
  ft->release(tls[0], mtx[0], true);

  ft->acquire(tls[2], mtx[0], 1, true);
  ft->read(tls[2], (void*)0x5F3ull, (void*)addr5, 16);
  ft->release(tls[2], mtx[0], true);
  ft->acquire(tls[2], mtx[0], 1, true);
  ft->read(tls[2], (void*)0x5F3ull, (void*)addr5, 16);
  ft->release(tls[2], mtx[0], true);

  ft->write(tls[2], (void*)0x78Eull, (void*)addr0, 16);  // this is the race

  EXPECT_EQ(ft->getData().wr_race, 0);
  EXPECT_EQ(ft->getData().rw_ex_race, 0);
  EXPECT_EQ(ft->getData().ww_race, 1);
  EXPECT_EQ(ft->getData().rw_sh_race, 0);
  ft->finalize();
}

TEST(FasttrackTest, Full_Fasttrack_Simple_Race) {
  using namespace drace::detector;

  auto ft = std::make_unique<FasttrackWrapper<std::mutex>>();
  auto rc_clb = [](const Detector::Race* r, void*) {
    ASSERT_EQ(r->first.stack_size, 1);
    ASSERT_EQ(r->second.stack_size, 2);
    // first stack
    EXPECT_EQ(r->first.stack_trace.at(0), 0x1ull);
    // second stack
    EXPECT_EQ(r->second.stack_trace.at(0), 0x2ull);
    EXPECT_EQ(r->second.stack_trace.at(1), 0x3ull);
  };
  const char* argv_mock[] = {"ft_test"};
  void* tls[2];  // storage for TLS data

  ft->init(1, argv_mock, rc_clb, nullptr);

  ft->fork(0, 1, &tls[0]);
  ft->fork(0, 2, &tls[1]);

  ft->write(tls[0], (void*)0x1ull, (void*)0x42ull, 1);
  ft->func_enter(tls[1], (void*)0x2ull);
  ft->write(tls[1], (void*)0x3ull, (void*)0x42ull, 1);
  // here, we expect the race. Handled in callback
  ft->finalize();
}

TEST(FasttrackTest, Fasttrack_Race_And_StackTrace) {
  using namespace drace::detector;

  auto ft = std::make_unique<FasttrackWrapper<std::mutex>>();
  auto rc_clb = [](const Detector::Race* r, void*) {
    ASSERT_EQ(r->first.stack_size, 7);
    ASSERT_EQ(r->second.stack_size, 6);
    // first stack
    EXPECT_EQ(r->first.stack_trace[0], 0x1ull);
    EXPECT_EQ(r->first.stack_trace[1], 0x2ull);
    EXPECT_EQ(r->first.stack_trace[2], 0x6ull);
    EXPECT_EQ(r->first.stack_trace[3], 0x3ull);
    EXPECT_EQ(r->first.stack_trace[4], 0x2ull);
    EXPECT_EQ(r->first.stack_trace[5], 0x8ull);
    EXPECT_EQ(r->first.stack_trace[6], 0x1006ull);
    // second stack
    EXPECT_EQ(r->second.stack_trace[0], 0x1ull);
    EXPECT_EQ(r->second.stack_trace[1], 0x20ull);
    EXPECT_EQ(r->second.stack_trace[2], 0x60ull);
    EXPECT_EQ(r->second.stack_trace[3], 0x30ull);
    EXPECT_EQ(r->second.stack_trace[4], 0x60ull);
    EXPECT_EQ(r->second.stack_trace[5], 0x1007ull);
  };
  const char* argv_mock[] = {"ft_test"};
  void* tls[2];  // storage for TLS data

  ft->init(1, argv_mock, rc_clb, nullptr);

  ft->fork(0, 1, &tls[0]);
  ft->fork(0, 2, &tls[1]);

  ft->func_enter(tls[0], (void*)0x1ull);
  ft->func_enter(tls[0], (void*)0x2ull);
  ft->func_enter(tls[0], (void*)0x6ull);
  ft->func_enter(tls[0], (void*)0x3ull);
  ft->func_enter(tls[0], (void*)0x6ull);
  ft->func_exit(tls[0]);
  ft->func_enter(tls[0], (void*)0x2ull);
  ft->func_enter(tls[0], (void*)0x8ull);
  ft->write(tls[0], (void*)0x1006ull, (void*)0x100ull, 8);
  ft->func_exit(tls[0]);
  ft->func_exit(tls[0]);
  ft->func_exit(tls[0]);
  ft->func_enter(tls[0], (void*)0x4ull);
  ft->func_enter(tls[0], (void*)0x2ull);
  ft->func_enter(tls[0], (void*)0x7ull);

  ft->func_enter(tls[1], (void*)0x1ull);
  ft->func_enter(tls[1], (void*)0x20ull);
  ft->func_enter(tls[1], (void*)0x60ull);
  ft->func_enter(tls[1], (void*)0x30ull);
  ft->func_enter(tls[1], (void*)0x60ull);
  ft->write(tls[1], (void*)0x1007ull, (void*)0x100ull, 8);
  ft->func_exit(tls[1]);
  ft->func_enter(tls[1], (void*)0x20ull);
  ft->func_enter(tls[1], (void*)0x80ull);
  ft->func_exit(tls[1]);
  ft->func_exit(tls[1]);
  ft->func_exit(tls[1]);
  ft->func_exit(tls[1]);
  ft->func_enter(tls[1], (void*)0x40ull);
  ft->func_enter(tls[1], (void*)0x20ull);
  ft->func_enter(tls[1], (void*)0x70ull);
  // here, we expect the race. Handled in callback
  ft->finalize();
}
